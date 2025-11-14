# CRC32 Checksum Additions to Sprite LFS

## Overview

Added comprehensive CRC32 checksum validation to all critical data structures in the Sprite LFS format specification. This provides data integrity verification for crash recovery and corruption detection.

## Structures with Checksum Validation

### 1. **Superblock** (`superblock`)

**Added Fields:**
- `checksum` (u4): CRC32 checksum of superblock metadata

**Added Instances:**
- `superblock_start`: Always 0 (superblock starts at beginning)
- `data_to_checksum`: Reads first 56 bytes (all fields before checksum)
- `computed_checksum`: Calculates CRC32 of superblock data
- `checksum_valid`: Boolean validation (computed == stored)

**What is Protected:**
- Magic number
- Version
- Block size
- Segment size
- Number of segments
- Checkpoint interval
- Clean threshold
- Creation time
- Last mount time

---

### 2. **Checkpoint Region** (`checkpoint_region_t`)

**Existing Field Enhanced:**
- `checksum` (u4): Already existed, now validated with instances

**Added Instances:**
- `checkpoint_start`: Calculates starting position of checkpoint
- `data_to_checksum`: Reads 64 bytes of checkpoint fields
- `computed_checksum`: Calculates CRC32
- `checksum_valid`: Boolean validation
- `is_fully_valid`: Combined validation (state == valid AND checksum valid)

**Enhanced Behavior:**
- `imap_root` and `seg_usage_table` now only parse if `is_fully_valid` is true
- Protects against using corrupted checkpoint data

**What is Protected:**
- Magic number
- Checkpoint number
- Timestamp
- Valid state flag
- Log head/tail pointers
- Inode map root location
- Segment usage table location
- Live inode count
- Free segment count

---

### 3. **Block Wrapper** (`block_wrapper_t`)

**Existing Field Enhanced:**
- Block header already had `checksum` field, now validated

**Added Instances:**
- `block_start`: Calculates block starting position
- `data_for_checksum`: Reads block data (excludes header)
- `computed_checksum`: Calculates CRC32 of block data
- `checksum_valid`: Boolean validation

**What is Protected:**
- All block data content (inodes, data blocks, indirect blocks, imap blocks)
- Does NOT include block header itself (header is metadata)

---

### 4. **Segment Summary** (`segment_summary_t`)

**Added Fields:**
- `checksum` (u4): CRC32 checksum of segment summary header

**Added Instances:**
- `summary_start`: Calculates segment summary starting position
- `header_to_checksum`: Reads 24 bytes of header fields
- `computed_checksum`: Calculates CRC32
- `checksum_valid`: Boolean validation

**What is Protected:**
- Magic number
- Segment number
- Write time
- Number of blocks
- Live bytes count
- Does NOT include block descriptors (too variable, calculated separately if needed)

---

## File-Level Validation Instances

Added top-level instances for overall file system health checks:

```yaml
instances:
  active_checkpoint:
    # Now uses is_fully_valid instead of just is_valid
    # Ensures both state flag and checksum are valid
  
  is_superblock_valid:
    value: superblock.checksum_valid
    doc: True if superblock checksum is valid
  
  has_valid_checkpoint:
    value: checkpoint_region_0.is_fully_valid or checkpoint_region_1.is_fully_valid
    doc: True if at least one checkpoint region is fully valid
```

---

## Usage Examples

### Python (Generated Code)

```python
from kaitaistruct import KaitaiStream
from sprite_lfs import SpriteLfs

# Parse LFS file
with open("filesystem.lfs", "rb") as f:
    lfs = SpriteLfs(KaitaiStream(f))

# Validate superblock
if lfs.is_superblock_valid:
    print("✓ Superblock is valid")
    print(f"  Format version: {lfs.superblock.version}")
else:
    print("✗ Superblock corrupted!")
    print(f"  Expected: {hex(lfs.superblock.checksum)}")
    print(f"  Got:      {hex(lfs.superblock.computed_checksum)}")

# Validate checkpoints
if lfs.has_valid_checkpoint:
    checkpoint = lfs.active_checkpoint
    print(f"✓ Active checkpoint #{checkpoint.checkpoint_number}")
    print(f"  Checkpoint valid: {checkpoint.is_fully_valid}")
    print(f"  State: {checkpoint.is_valid}")
    print(f"  Checksum: {checkpoint.checksum_valid}")
    
    # Safe to access checkpoint data
    if checkpoint.is_fully_valid:
        imap = checkpoint.imap_root
        print(f"  Inode map: {imap.num_entries} entries")
else:
    print("✗ No valid checkpoint found!")

# Validate blocks in segments
for seg in lfs.segments:
    print(f"\nSegment {seg.segment_number}:")
    
    # Check segment summary
    if seg.summary.checksum_valid:
        print(f"  ✓ Summary valid ({seg.summary.num_blocks} blocks)")
    else:
        print(f"  ✗ Summary corrupted!")
    
    # Check individual blocks
    for block in seg.blocks:
        if block.checksum_valid:
            print(f"  ✓ Block {block.block_header.block_number} valid")
        else:
            print(f"  ✗ Block {block.block_header.block_number} corrupted!")
```

---

## Technical Details

### CRC32 Algorithm

- **Algorithm**: Standard CRC32 (polynomial 0xEDB88320)
- **Initial value**: 0
- **Process**: `process: crc32(0)` in Kaitai Struct
- **Byte order**: Little-endian (matches file format)

### Checksum Calculation Approach

All checksums follow this pattern:

1. **Store position**: Calculate starting position of structure
2. **Extract data**: Read specific byte range (fields before checksum)
3. **Calculate**: Apply CRC32 with seed 0
4. **Compare**: Validate computed vs stored checksum
5. **Conditional parsing**: Only parse dependent structures if valid

### Memory Efficiency

Checksums are **lazily evaluated**:
- Computed only when accessed
- Not calculated during initial parsing
- Minimal performance impact if not used

---

## Benefits

### 1. **Crash Recovery**
- Detect corrupted checkpoints after crash
- Choose valid checkpoint automatically
- Prevent using corrupted metadata

### 2. **Corruption Detection**
- Identify bit rot in superblock
- Detect corrupted blocks during cleaning
- Validate segment summaries before use

### 3. **Development & Debugging**
- Verify file system writes
- Test cleaner correctness
- Validate checkpoint atomicity

### 4. **Forensics**
- Identify corruption patterns
- Determine extent of damage
- Recover partial data

---

## Backward Compatibility

**Breaking Change**: Added checksum fields to:
- Superblock (+4 bytes)
- Segment summary (+4 bytes)

**Preserved Fields**:
- Checkpoint checksum field (already existed)
- Block header checksum field (already existed)

**Migration Path**:
1. Calculate and write checksums during next mount
2. Mark old checksums as 0xFFFFFFFF (invalid)
3. Validate during read, recalculate during write

---

## Future Enhancements

Potential additions:

1. **Additional checksums** for:
   - Individual block descriptors
   - Inode map entries
   - Indirect block pointer arrays

2. **Stronger algorithms**:
   - CRC64 for large files
   - SHA-256 for cryptographic verification
   - Reed-Solomon error correction

3. **Metadata checksums**:
   - Inode checksum
   - Directory entry checksum
   - Extended attribute checksum

4. **Hierarchical validation**:
   - Merkle tree of block checksums
   - Incremental verification
   - Partial validation

