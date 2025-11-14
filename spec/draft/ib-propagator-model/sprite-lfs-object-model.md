# Sprite LFS - Object Model

## Overview

The Sprite Log-Structured File System (LFS) treats storage as a circular log where all modifications (inodes, data blocks, directory blocks) are written sequentially to new locations. This object model captures the core data structures with special attention to **segment reclamation** (cleaning).

## Source

- **Original Paper**: "The Design and Implementation of a Log-Structured File System" by Mendel Rosenblum and John K. Ousterhout (1992)
- **Source Code**: Available at ftp.cs.berkeley.edu/sprite/

---

## Core Entities

### [checkpoint] Checkpoint Region

The checkpoint region contains pointers to key metadata structures, written periodically to enable crash recovery.

```text
[checkpoint] checkpoint_region
- checkpoint_pid    : UUID <<PK>>
- checkpoint_id     : UUID <<UNIQUE>>
- timestamp         : TIMESTAMP
- log_head_addr     : DiskAddress    # Current head of log
- log_tail_addr     : DiskAddress    # Oldest segment still in use
- imap_root_pid     : UUID <<FK imap_block>>  # Root of inode map
- seg_usage_tbl_pid : UUID <<FK segment_usage_table>>
- checkpoint_number : INT64          # Monotonically increasing
- is_active         : BOOL           # Two checkpoints alternate
```

**Notes:**

- Two checkpoint regions exist on disk (positions 0 and 1)
- Alternating writes ensure one valid checkpoint survives crashes
- Contains enough information to reconstruct entire file system state

---

### [imap_block] Inode Map Block

The inode map (imap) translates inode numbers to their current disk locations. The imap itself is written to the log.

```text
[imap_block] inode_map_block
- imap_block_pid  : UUID <<PK>>
- imap_block_id   : UUID <<UNIQUE>>
- disk_address    : DiskAddress  # Where this imap block resides
- start_inode_num : INT32        # First inode number in this block
- end_inode_num   : INT32        # Last inode number in this block
- version         : INT64        # Incremented on each write
- entries         : JSON         # Array of {inode_num → disk_address}
```

**Properties:**


- Imap is divided into blocks (typically 4KB each)
- Each entry maps an inode number to its current disk location
- Imap blocks are cached in memory for fast lookups
- When inode moves (due to update), imap entry is updated and imap block rewritten

---

### [inode] Inode

Inodes store file metadata. Unlike traditional file systems, LFS inodes are scattered throughout the log.

```text
[inode] inode
- inode_pid           : UUID <<PK>>
- inode_id            : UUID <<UNIQUE>>
- inode_number        : INT32        # File system inode number
- disk_address        : DiskAddress  # Current location in log
- file_type           : ENUM('FILE', 'DIR', 'SYMLINK', 'DEV')
- file_size           : INT64
- uid                 : INT32
- gid                 : INT32
- mode                : INT16        # Permissions
- atime               : TIMESTAMP
- mtime               : TIMESTAMP
- ctime               : TIMESTAMP
- link_count          : INT16
- version             : INT64        # Incremented on each write
- direct_blocks       : ARRAY[10] DiskAddress  # Direct data block pointers
- indirect_block_pid  : UUID <<FK indirect_block>>
- double_indirect_pid : UUID <<FK indirect_block>>
- triple_indirect_pid : UUID <<FK indirect_block>>
```

**Properties:**

- Written to log on every modification (copy-on-write)
- Old versions become garbage, reclaimed by cleaner
- Inode number is stable; disk address changes

---

### [indirect_block] Indirect Block

Indirect blocks contain arrays of pointers for large files.

```text
[indirect_block] indirect_block
- indirect_block_pid : UUID <<PK>>
- indirect_block_id  : UUID <<UNIQUE>>
- disk_address       : DiskAddress
- level              : INT8           # 1=indirect, 2=double, 3=triple
- pointers           : ARRAY DiskAddress  # Pointers to data or other indirect blocks
- version            : INT64
```

---

### [data_block] Data Block

Actual file data blocks.

```text
[data_block] data_block
- data_block_pid : UUID <<PK>>
- data_block_id  : UUID <<UNIQUE>>
- disk_address   : DiskAddress
- inode_number   : INT32        # Owner inode
- block_number   : INT32        # Logical block number within file
- data           : BLOB         # Actual file data (typically 4KB)
- checksum       : BYTES[32]    # SHA-256 for integrity
```

---

### [segment] Segment

A segment is a large contiguous region of disk (typically 512KB - 1MB) containing multiple blocks written sequentially.

```text
[segment] segment
- segment_pid       : UUID <<PK>>
- segment_id        : UUID <<UNIQUE>>
- segment_number    : INT32        # Logical segment number
- disk_address      : DiskAddress  # Starting address
- size              : INT32        # Segment size in bytes
- write_time        : TIMESTAMP    # When segment was written
- summary_block_pid : UUID <<FK segment_summary>>
- state             : ENUM('ACTIVE', 'FULL', 'CLEANING', 'FREE')
```

**Properties:**

- Disk divided into fixed-size segments
- Each segment written sequentially from beginning to end
- Once full, segment becomes read-only (until cleaned)
- Contains mix of inodes, data blocks, indirect blocks, imap blocks

---

### [segment_summary] Segment Summary Block

Written at the end of each segment, describes the segment's contents for efficient cleaning.

```text
[segment_summary] segment_summary_block
- summary_pid   : UUID <<PK>>
- summary_id    : UUID <<UNIQUE>>
- segment_pid   : UUID <<FK segment>>
- disk_address  : DiskAddress
- num_blocks    : INT32  # Number of blocks in segment
- blocks        : JSON   # Array of block descriptors
```

**Block Descriptor Structure:**

```json
{
  "offset": 0,              // Offset within segment
  "type": "INODE",          // INODE, DATA, INDIRECT, IMAP
  "inode_number": 42,       // For DATA/INDIRECT blocks
  "block_number": 5,        // Logical block number (for DATA)
  "length": 4096            // Block size
}
```

**Usage:**

- Enables cleaner to identify block ownership
- Allows quick determination of live vs. dead blocks
- Critical for segment reclamation

---

## Segment Reclamation Subsystem

### [segment_usage_table] Segment Usage Table

Tracks utilization of each segment to guide cleaning policy.

```text
[segment_usage_table] segment_usage_table
- seg_usage_tbl_pid : UUID <<PK>>
- seg_usage_tbl_id  : UUID <<UNIQUE>>
- disk_address      : DiskAddress  # Written to log like other data
- version           : INT64
- entries           : JSON         # Array of segment usage entries
```

**Segment Usage Entry:**

```json
{
  "segment_number": 42,
  "live_bytes": 262144,        // Bytes of live data
  "total_bytes": 524288,       // Total segment size
  "last_modified": "2025-11-11T10:00:00Z",
  "age": 3600                  // Seconds since last modification
}
```

**Properties:**

- Updated whenever blocks in a segment become obsolete
- Used by cleaner to select segments for cleaning
- Cached in memory, periodically written to log

---

### [segment_cleaner] Segment Cleaner

The cleaner process that performs segment reclamation.

```text
[segment_cleaner] segment_cleaner
- cleaner_pid      : UUID <<PK>>
- cleaner_id       : UUID <<UNIQUE>>
- policy           : ENUM('GREEDY', 'COST_BENEFIT', 'CAT')
- threshold        : FLOAT      # Min utilization to trigger cleaning
- is_active        : BOOL
- segments_cleaned : INT64      # Statistics
- bytes_reclaimed  : INT64
- last_run_time    : TIMESTAMP
```

**Cleaning Policies:**

1. **Greedy**: Clean segments with most free space
   - Simple, but can cause high write traffic
   - Formula: `benefit = (1 - utilization)`

2. **Cost-Benefit**: Balance benefit vs. cost of cleaning
   - Accounts for segment age (hot vs. cold data)
   - Formula: `benefit = ((1 - u) * age) / (2 * u)`
   - Where `u` = utilization (live_bytes / total_bytes)
   - Where `age` = time since segment last modified

3. **CAT (Cleaner Adaptive Threshold)**: Adjust threshold based on disk utilization

---

### [cleaning_job] Cleaning Job

Represents a single cleaning operation.

```text
[cleaning_job] cleaning_job
- job_pid             : UUID <<PK>>
- job_id              : UUID <<UNIQUE>>
- cleaner_pid         : UUID <<FK segment_cleaner>>
- target_segment_pid  : UUID <<FK segment>>
- state               : ENUM('PENDING', 'READING', 'WRITING', 'COMPLETE', 'FAILED')
- start_time          : TIMESTAMP
- end_time            : TIMESTAMP
- live_blocks_read    : INT32
- live_blocks_written : INT32
- bytes_reclaimed     : INT64
```

**Cleaning Process:**

1. Select victim segments using policy
2. Read segment summary to identify blocks
3. Check liveness: Read inode map / inode to verify block is still referenced
4. Copy live blocks to new segments
5. Update inode map with new addresses
6. Mark old segment as free

---

### [cleaning_policy_params] Cleaning Policy Parameters

Configuration for cleaning behavior.

```text
[cleaning_policy_params] cleaning_policy_params
- params_pid           : UUID <<PK>>
- params_id            : UUID <<UNIQUE>>
- min_free_segments    : INT32   # Trigger cleaning when below this
- target_free_segments : INT32   # Stop cleaning when reached
- clean_window_start   : TIME    # Time window for background cleaning
- clean_window_end     : TIME
- write_cost           : FLOAT   # Cost of writing one block
- read_cost            : FLOAT   # Cost of reading one block
- hot_threshold        : INT32   # Age (seconds) to classify as hot data
- cold_threshold       : INT32   # Age (seconds) to classify as cold data
```

**Hot/Cold Segregation:**

- Hot data: Frequently modified, should be cleaned together
- Cold data: Rarely modified, stable
- Prevents mixing hot/cold in same segment (reduces future cleaning)

---

### [liveness_info] Liveness Information

Tracks whether a block at a given address is still live (current version).

```text
[liveness_info] liveness_info
- liveness_pid  : UUID <<PK>>
- liveness_id   : UUID <<UNIQUE>>
- disk_address  : DiskAddress
- inode_number  : INT32
- block_number  : INT32        # Logical block in file
- block_type    : ENUM('INODE', 'DATA', 'INDIRECT', 'IMAP')
- is_live       : BOOL         # True if current version
- checked_at    : TIMESTAMP
```

**Liveness Determination:**

- For INODE: Check inode map, if imap[inode_number] == disk_address → live
- For DATA: Read inode, traverse block pointers, if points to disk_address → live
- For INDIRECT: Similar to DATA
- For IMAP: Check checkpoint region, if points to this imap block → live

---

## Write Path Entities

### [log_segment_writer] Log Segment Writer

Manages writing new data to the active segment.

```text
[log_segment_writer] log_segment_writer
- writer_pid         : UUID <<PK>>
- writer_id          : UUID <<UNIQUE>>
- active_segment_pid : UUID <<FK segment>>
- current_offset     : INT64  # Write position within segment
- buffer_pid         : UUID <<FK write_buffer>>
- writes_pending     : INT32
```

---

### [write_buffer] Write Buffer

In-memory buffer for batching writes to segments.

```text
[write_buffer] write_buffer
- buffer_pid      : UUID <<PK>>
- buffer_id       : UUID <<UNIQUE>>
- size            : INT32  # Buffer size (typically 1MB)
- used            : INT32  # Bytes currently used
- blocks          : JSON   # Array of buffered blocks
- flush_threshold : INT32  # Flush when buffer reaches this size
```

---

## Crash Recovery Entities

### [recovery_log] Recovery Log

Tracks recovery state after crash.

```text
[recovery_log] recovery_log
- recovery_pid            : UUID <<PK>>
- recovery_id             : UUID <<UNIQUE>>
- crash_time              : TIMESTAMP
- recovery_start          : TIMESTAMP
- recovery_end            : TIMESTAMP
- checkpoint_used_pid     : UUID <<FK checkpoint>>
- segments_rolled_forward : INT32  # Segments applied after checkpoint
- state                   : ENUM('SCANNING', 'REPLAYING', 'COMPLETE')
```

**Recovery Process:**

1. Read most recent valid checkpoint
2. Restore inode map and segment usage table
3. Roll forward: Scan segments written after checkpoint
4. Reconstruct in-memory structures
5. Mark file system clean

---

## Relationships

### Checkpoint → Inode Map

- Checkpoint contains pointer to root of inode map tree
- `checkpoint.imap_root_pid` → `imap_block.imap_block_pid`

### Checkpoint → Segment Usage Table

- Checkpoint points to current segment usage table
- `checkpoint.seg_usage_tbl_pid` → `segment_usage_table.seg_usage_tbl_pid`

### Inode Map → Inode

- Inode map entries point to current inode locations
- `imap_block.entries[i].disk_address` → `inode.disk_address`

### Inode → Data/Indirect Blocks

- Inode contains pointers to file data
- `inode.direct_blocks[i]` → `data_block.disk_address`
- `inode.indirect_block_pid` → `indirect_block.indirect_block_pid`

### Segment → Segment Summary

- Each segment has one summary block
- `segment.summary_block_pid` → `segment_summary.summary_pid`

### Segment Cleaner → Cleaning Jobs

- Cleaner spawns jobs to clean segments
- `cleaning_job.cleaner_pid` → `segment_cleaner.cleaner_pid`

### Cleaning Job → Segment

- Each job targets one or more segments
- `cleaning_job.target_segment_pid` → `segment.segment_pid`

---

## Segment Reclamation: Detailed Flow

### Phase 1: Victim Selection

```text
1. Read segment_usage_table
2. For each segment:
   - Calculate utilization: u = live_bytes / total_bytes
   - Calculate age: age = now - last_modified
   - Calculate benefit score:
     * Greedy: score = 1 - u
     * Cost-Benefit: score = ((1 - u) * age) / (2 * u)
3. Sort segments by score (descending)
4. Select top N segments where N determined by free space threshold
```

### Phase 2: Liveness Check

```text
For each block in victim segment (from segment_summary):
  1. Read block descriptor (type, inode_number, block_number)
  2. Determine liveness:
     - INODE: Check imap[inode_number] == block.disk_address
     - DATA: Read inode, traverse to block_number, check pointer
     - INDIRECT: Read parent indirect/inode, check pointer
     - IMAP: Check checkpoint.imap_root points to this block
  3. If live:
     - Add to live_block_list
  4. If dead:
     - Count as reclaimable space
```

### Phase 3: Block Migration

```text
For each live block in victim segment:
  1. Read block data from old location
  2. Write block to new active segment
  3. Update pointers:
     - For INODE: Update imap entry
     - For DATA: Update inode's block pointer (write new inode version)
     - For INDIRECT: Update parent pointer
     - For IMAP: Update checkpoint region
  4. Log old → new address mapping (for rollback)
```

### Phase 4: Segment Reclamation

```text
1. Verify all live blocks migrated successfully
2. Mark old segment as FREE in segment_usage_table
3. Update segment state: FULL → FREE
4. Add segment to free_segment_list
5. Update statistics:
   - segments_cleaned++
   - bytes_reclaimed += segment.size
```

### Phase 5: Metadata Update

```text
1. Write updated inode map blocks to log
2. Write updated segment usage table to log
3. Write new checkpoint region (atomic)
4. Cleaning operation now durable
```

---

## Optimizations & Considerations

### Hot/Cold Segregation

- Classify segments as hot (frequently modified) or cold (stable)
- Clean hot segments less aggressively (data will likely be rewritten soon)
- Clean cold segments more thoroughly (data will remain stable)

### Cleaning Threshold

- Too aggressive: High write amplification (cleaning overhead)
- Too conservative: Run out of free space
- Typical: Start cleaning at 75% disk utilization, stop at 50%

### Crash During Cleaning

- Cleaning is idempotent: Old data remains valid until checkpoint updated
- If crash occurs:
  - Old blocks still referenced by inode map
  - New blocks written but not yet referenced (garbage)
  - Checkpoint points to old inode map → consistent state
  - Resume cleaning after recovery

### Write Amplification

- Key metric: `write_amp = (user_writes + cleaning_writes) / user_writes`
- LFS typically achieves 1.1-1.5x write amplification with good cleaning policy
- Cost-benefit policy minimizes write amp by cleaning cold data less frequently

### Segment Size Trade-offs

- Larger segments: Better sequential write throughput, but coarser cleaning granularity
- Smaller segments: Finer control, but more metadata overhead
- Typical: 512KB - 1MB segments

---

## References

1. Rosenblum, M., & Ousterhout, J. K. (1992). "The Design and Implementation of a Log-Structured File System". ACM Transactions on Computer Systems, 10(1), 26-52.

2. Seltzer, M., Bostic, K., McKusick, M. K., & Staelin, C. (1993). "An Implementation of a Log-Structured File System for UNIX". USENIX Winter.

3. Matthews, J. N., et al. (1997). "Improving the Performance of Log-Structured File Systems with Adaptive Methods". ACM SOSP.

---

## Implementation Notes for CPSC Integration

Given the CPSC context (from propagator-cpsc.md), here's how LFS concepts map:

### Coroutine Integration

- **Segment Cleaning**: Each cleaning job = coroutine
  - Suspend on I/O: Reading victim segments
  - Suspend on I/O: Writing live blocks to new segments
  - Suspend on latch: Acquiring segment locks
  
- **Write Path**: Log writer = coroutine
  - Suspend when buffer full (async flush)
  - Batch multiple writes for efficiency

### Database Backing

- Store segment usage table in CPSC database
- Store inode map in database (with caching)
- Cleaning policy parameters as database configuration
- Historical cleaning statistics for adaptive policies

### Naming Convention Compliance

All entities follow the `<entity>_pid` / `<entity>_id` pattern:

- Physical ID (`_pid`): Internal database FK references
- Logical ID (`_id`): Stable identifier for external references

This design enables building LFS-style storage using CPSC's coroutine model with database-backed metadata.

