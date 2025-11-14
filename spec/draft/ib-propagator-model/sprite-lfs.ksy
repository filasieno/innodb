# $schema=../../schema/kaitai.schema.json
meta:
  id: sprite_lfs
  title: Sprite Log-Structured File System
  application: Sprite Operating System
  file-extension:
    - lfs
    - img
  license: MIT 
  endian: le
  doc: |
    Sprite LFS (Log-Structured File System) from UC Berkeley.
    
    The file system treats storage as a circular log where all modifications
    (inodes, data blocks, directory blocks) are written sequentially to new
    locations. This enables high write performance and simplified crash recovery.
    
    Reference: "The Design and Implementation of a Log-Structured File System"
    by Mendel Rosenblum and John K. Ousterhout (1992)
  doc-ref: https://people.eecs.berkeley.edu/~brewer/cs262/LFS.pdf

seq:
  - id: superblock
    type: superblock
    doc: Superblock containing file system metadata (1024 bytes total)
  
  - id: checkpoint_region_0
    type: checkpoint_region_t
    doc: First checkpoint region (alternates with checkpoint_region_1)
  
  - id: checkpoint_region_1
    type: checkpoint_region_t
    doc: Second checkpoint region (alternates with checkpoint_region_0)
  
  - id: segments
    type: segment_t
    repeat: expr
    repeat-expr: superblock.num_segments
    doc: Array of segments containing all file system data

types:
  # ============================================================================
  # SUPERBLOCK
  # ============================================================================
  superblock:
    doc: |
      The superblock contains global file system parameters and is located
      at the beginning of the file system. Total size is exactly 1024 bytes.
    seq:
      - id: magic
        contents: [0x4c, 0x46, 0x53, 0x31]  # "LFS1"
        doc: Magic number identifying Sprite LFS
      
      - id: version
        type: u4
        doc: File system version
      
      - id: block_size
        type: u4
        doc: Size of one block in bytes (typically 4096)
      
      - id: segment_size
        type: u4
        doc: Size of one segment in bytes (typically 512KB-1MB)
      
      - id: num_segments
        type: u4
        doc: Total number of segments in file system
      
      - id: checkpoint_interval
        type: u4
        doc: Seconds between checkpoints
      
      - id: clean_threshold
        type: u4
        doc: Minimum free segments before cleaning starts
      
      - id: creation_time
        type: u8
        doc: Unix timestamp of file system creation
      
      - id: last_mount_time
        type: u8
        doc: Unix timestamp of last mount
      
      - id: checksum
        type: u4
        doc: CRC32 checksum of superblock metadata
      
      - id: reserved
        size: 1024 - _io.pos
        doc: padding
    
    instances:
      # Calculate starting position of superblock
      superblock_start:
        value: 0
        doc: Superblock always starts at byte 0
      
      # Read superblock data for checksumming (everything before checksum field)
      data_to_checksum:
        io: _root._io
        pos: superblock_start
        size: 56  # All fields before checksum (4 + 4 + 4 + 4 + 4 + 4 + 4 + 8 + 8 + 4 = 48 bytes)
        doc: Superblock data for CRC32 calculation (excludes checksum and reserved)
      
      # Calculate CRC32
      computed_checksum:
        value: data_to_checksum
        process: crc32(0)
        doc: Computed CRC32 of superblock fields
      
      # Validation
      checksum_valid:
        value: computed_checksum == checksum
        doc: True if stored checksum matches computed checksum

  # ============================================================================
  # CHECKPOINT REGION
  # ============================================================================
  checkpoint_region_t:
    doc: |
      Checkpoint region contains pointers to key metadata structures.
      Two checkpoint regions alternate to ensure one valid checkpoint
      survives crashes. Written periodically to enable crash recovery.
      Total size is exactly 1024 bytes.
    seq:
      - id: magic
        contents: [0x43, 0x48, 0x4b, 0x50]  # "CHKP"
        doc: Checkpoint magic number
      
      - id: checkpoint_number
        type: u8
        doc: Monotonically increasing checkpoint sequence number
      
      - id: timestamp
        type: u8
        doc: Unix timestamp when checkpoint was written
      
      - id: is_valid
        type: u4
        enum: checkpoint_state
        doc: Whether this checkpoint is valid
      
      - id: log_head_segment
        type: u4
        doc: Segment number of current log head
      
      - id: log_head_offset
        type: u4
        doc: Offset within log head segment
      
      - id: log_tail_segment
        type: u4
        doc: Segment number of oldest segment still in use
      
      - id: imap_root_segment
        type: u4
        doc: Segment containing root of inode map
      
      - id: imap_root_offset
        type: u4
        doc: Offset of inode map root within segment
      
      - id: seg_usage_table_segment
        type: u4
        doc: Segment containing segment usage table
      
      - id: seg_usage_table_offset
        type: u4
        doc: Offset of segment usage table within segment
      
      - id: num_live_inodes
        type: u8
        doc: Number of allocated inodes
      
      - id: num_free_segments
        type: u4
        doc: Number of completely free segments
      
      - id: checksum
        type: u4
        doc: CRC32 checksum of checkpoint data
      
      - id: reserved
        size: 1024 - _io.pos
        doc: Reserved bytes (padding to reach 1024 bytes total)
    
    instances:
      # Calculate starting position of checkpoint
      checkpoint_start:
        value: _io.pos - (1024 - reserved.size)
        doc: Starting byte position of this checkpoint region
      
      # Read checkpoint data for checksumming (everything before checksum field)
      data_to_checksum:
        io: _root._io
        pos: checkpoint_start
        size: 64  # All fields before checksum (magic=4 + checkpoint_number=8 + timestamp=8 + is_valid=4 + log_head_segment=4 + log_head_offset=4 + log_tail_segment=4 + imap_root_segment=4 + imap_root_offset=4 + seg_usage_table_segment=4 + seg_usage_table_offset=4 + num_live_inodes=8 + num_free_segments=4)
        doc: Checkpoint data for CRC32 calculation
      
      # Calculate CRC32
      computed_checksum:
        value: data_to_checksum
        process: crc32(0)
        doc: Computed CRC32 of checkpoint fields
      
      # Validation
      checksum_valid:
        value: computed_checksum == checksum
        doc: True if stored checksum matches computed checksum
      
      # Combined validation (state flag AND checksum)
      is_fully_valid:
        value: is_valid == checkpoint_state::valid and checksum_valid
        doc: True if checkpoint state is valid AND checksum is correct
      
      # Parsed structures (only if fully valid)
      imap_root:
        io: _root._io
        pos: imap_root_segment * _root.superblock.segment_size + imap_root_offset
        type: imap_block_t
        if: is_fully_valid
        doc: Parsed inode map root block (only if checkpoint is fully valid)
      
      seg_usage_table:
        io: _root._io
        pos: seg_usage_table_segment * _root.superblock.segment_size + seg_usage_table_offset
        type: segment_usage_table_t
        if: is_fully_valid
        doc: Parsed segment usage table (only if checkpoint is fully valid)
    
    enums:
      checkpoint_state:
        0: invalid
        1: valid

  # ============================================================================
  # SEGMENT
  # ============================================================================
  segment_t:
    doc: |
      A segment is a large contiguous region of disk (typically 512KB-1MB)
      containing multiple blocks written sequentially. Each segment is written
      from beginning to end, then becomes read-only until cleaned.
    seq:
      - id: blocks
        type: block_wrapper_t
        repeat: until
        repeat-until: _.block_type == block_type::segment_summary or _io.pos >= _root.superblock.segment_size
        doc: Blocks within this segment
      
      - id: summary
        type: segment_summary_t
        doc: Segment summary block at end of segment
    
    instances:
      segment_number:
        value: _index
        doc: Logical segment number (derived from position)

  # ============================================================================
  # BLOCK WRAPPER
  # ============================================================================
  block_wrapper_t:
    doc: |
      Wrapper around individual blocks in a segment to handle different
      block types (inode, data, indirect, imap).
    seq:
      - id: block_header
        type: block_header_t
        doc: Common block header
      
      - id: block_data
        size: block_header.block_size
        type:
          switch-on: block_header.block_type
          cases:
            block_type::inode: inode_t
            block_type::data: data_block_t
            block_type::indirect: indirect_block_t
            block_type::imap: imap_block_t
        doc: Block data (type depends on block_type)
    
    instances:
      block_type:
        value: block_header.block_type
        doc: Type of this block
      
      # Calculate starting position of block
      block_start:
        value: _io.pos - block_header._sizeof - block_header.block_size
        doc: Starting byte position of this block
      
      # Read block data for checksum validation
      data_for_checksum:
        io: _root._io
        pos: block_start + block_header._sizeof
        size: block_header.block_size
        doc: Block data for CRC32 calculation (excludes header)
      
      # Calculate CRC32 of block data
      computed_checksum:
        value: data_for_checksum
        process: crc32(0)
        doc: Computed CRC32 of block data
      
      # Validate checksum
      checksum_valid:
        value: computed_checksum == block_header.checksum
        doc: True if block data checksum is valid

  # ============================================================================
  # BLOCK HEADER
  # ============================================================================
  block_header_t:
    doc: Common header for all blocks in segments
    seq:
      - id: block_type
        type: u1
        enum: block_type
        doc: Type of block (inode, data, indirect, imap, segment_summary)
      
      - id: block_size
        type: u4
        doc: Size of block data in bytes
      
      - id: inode_number
        type: u4
        doc: Owner inode number (0 for metadata blocks)
      
      - id: block_number
        type: u4
        doc: Logical block number within file (for data/indirect blocks)
      
      - id: version
        type: u8
        doc: Version counter (incremented on each write)
      
      - id: checksum
        type: u4
        doc: CRC32 checksum of block data

  # ============================================================================
  # INODE
  # ============================================================================
  inode_t:
    doc: |
      Inode stores file metadata. Unlike traditional file systems,
      LFS inodes are scattered throughout the log and can move.
      The inode map tracks current locations.
    seq:
      - id: inode_number
        type: u4
        doc: Stable inode number
      
      - id: file_type
        type: u2
        enum: file_type
        doc: Type of file (regular, directory, symlink, device)
      
      - id: mode
        type: u2
        doc: Unix permissions (rwxrwxrwx)
      
      - id: uid
        type: u4
        doc: Owner user ID
      
      - id: gid
        type: u4
        doc: Owner group ID
      
      - id: file_size
        type: u8
        doc: File size in bytes
      
      - id: atime
        type: u8
        doc: Last access time (Unix timestamp)
      
      - id: mtime
        type: u8
        doc: Last modification time (Unix timestamp)
      
      - id: ctime
        type: u8
        doc: Creation time (Unix timestamp)
      
      - id: link_count
        type: u2
        doc: Number of hard links
      
      - id: num_direct_blocks
        type: u2
        doc: Number of direct block pointers (typically 10)
      
      - id: direct_blocks
        type: disk_address_t
        repeat: expr
        repeat-expr: num_direct_blocks
        doc: Direct data block pointers
      
      - id: indirect_block
        type: disk_address_t
        doc: Indirect block pointer (points to array of data blocks)
      
      - id: double_indirect_block
        type: disk_address_t
        doc: Double indirect block pointer
      
      - id: triple_indirect_block
        type: disk_address_t
        doc: Triple indirect block pointer

  # ============================================================================
  # DISK ADDRESS
  # ============================================================================
  disk_address_t:
    doc: |
      Disk address specifying segment number and offset within segment.
      Used for all block pointers.
    seq:
      - id: segment_number
        type: u4
        doc: Segment number (0xFFFFFFFF = null pointer)
      
      - id: offset
        type: u4
        doc: Offset within segment in bytes
    
    instances:
      is_null:
        value: segment_number == 0xFFFFFFFF
        doc: Whether this is a null pointer
      
      absolute_offset:
        value: segment_number * _root.superblock.segment_size + offset
        if: not is_null
        doc: Absolute byte offset in file system

  # ============================================================================
  # DATA BLOCK
  # ============================================================================
  data_block_t:
    doc: Actual file data block
    seq:
      - id: data
        size-eos: true
        doc: Raw file data (size determined by block_header)

  # ============================================================================
  # INDIRECT BLOCK
  # ============================================================================
  indirect_block_t:
    doc: |
      Indirect block contains array of pointers to data blocks
      (or other indirect blocks for double/triple indirect).
    seq:
      - id: level
        type: u1
        doc: Indirection level (1=indirect, 2=double, 3=triple)
      
      - id: num_pointers
        type: u4
        doc: Number of pointers in this block
      
      - id: pointers
        type: disk_address_t
        repeat: expr
        repeat-expr: num_pointers
        doc: Array of block pointers

  # ============================================================================
  # INODE MAP BLOCK
  # ============================================================================
  imap_block_t:
    doc: |
      Inode map block translates inode numbers to their current disk locations.
      The imap itself is written to the log and tracked by checkpoint.
    seq:
      - id: start_inode_num
        type: u4
        doc: First inode number in this block
      
      - id: end_inode_num
        type: u4
        doc: Last inode number in this block
      
      - id: num_entries
        type: u4
        doc: Number of entries in this block
      
      - id: entries
        type: imap_entry_t
        repeat: expr
        repeat-expr: num_entries
        doc: Array of inode number to disk address mappings

  # ============================================================================
  # INODE MAP ENTRY
  # ============================================================================
  imap_entry_t:
    doc: Single entry in inode map
    seq:
      - id: inode_number
        type: u4
        doc: Inode number
      
      - id: location
        type: disk_address_t
        doc: Current disk location of this inode

  # ============================================================================
  # SEGMENT SUMMARY BLOCK
  # ============================================================================
  segment_summary_t:
    doc: |
      Segment summary block written at end of each segment.
      Describes segment contents for efficient cleaning (liveness checking).
    seq:
      - id: magic
        contents: [0x53, 0x55, 0x4d, 0x4d]  # "SUMM"
        doc: Segment summary magic number
      
      - id: segment_number
        type: u4
        doc: Segment number
      
      - id: write_time
        type: u8
        doc: Unix timestamp when segment was written
      
      - id: num_blocks
        type: u4
        doc: Number of blocks in this segment
      
      - id: live_bytes
        type: u4
        doc: Bytes of live data (updated by cleaner)
      
      - id: checksum
        type: u4
        doc: CRC32 checksum of segment summary header
      
      - id: block_descriptors
        type: block_descriptor_t
        repeat: expr
        repeat-expr: num_blocks
        doc: Descriptors for each block in segment
    
    instances:
      # Calculate starting position of segment summary
      summary_start:
        value: _io.pos - block_descriptors._sizeof - 28  # 28 = size of fields before block_descriptors
        doc: Starting byte position of segment summary
      
      # Read segment summary header for checksumming
      header_to_checksum:
        io: _root._io
        pos: summary_start
        size: 24  # magic(4) + segment_number(4) + write_time(8) + num_blocks(4) + live_bytes(4)
        doc: Segment summary header for CRC32 calculation
      
      # Calculate CRC32
      computed_checksum:
        value: header_to_checksum
        process: crc32(0)
        doc: Computed CRC32 of segment summary header
      
      # Validation
      checksum_valid:
        value: computed_checksum == checksum
        doc: True if segment summary checksum is valid

  # ============================================================================
  # BLOCK DESCRIPTOR
  # ============================================================================
  block_descriptor_t:
    doc: |
      Descriptor for a single block in segment summary.
      Enables cleaner to identify block ownership and check liveness.
    seq:
      - id: offset
        type: u4
        doc: Offset of block within segment
      
      - id: block_type
        type: u1
        enum: block_type
        doc: Type of block
      
      - id: inode_number
        type: u4
        doc: Owner inode number (for data/indirect blocks)
      
      - id: block_number
        type: u4
        doc: Logical block number within file (for data blocks)
      
      - id: length
        type: u4
        doc: Block size in bytes
      
      - id: version
        type: u8
        doc: Version at time of write (for liveness checking)

  # ============================================================================
  # SEGMENT USAGE TABLE
  # ============================================================================
  segment_usage_table_t:
    doc: |
      Segment usage table tracks utilization of each segment to guide
      cleaning policy (greedy, cost-benefit, CAT).
    seq:
      - id: magic
        contents: [0x53, 0x45, 0x47, 0x55]  # "SEGU"
        doc: Segment usage table magic number
      
      - id: version
        type: u8
        doc: Version counter (incremented on each update)
      
      - id: num_entries
        type: u4
        doc: Number of segment usage entries
      
      - id: entries
        type: segment_usage_entry_t
        repeat: expr
        repeat-expr: num_entries
        doc: Array of segment usage entries

  # ============================================================================
  # SEGMENT USAGE ENTRY
  # ============================================================================
  segment_usage_entry_t:
    doc: |
      Usage statistics for a single segment.
      Used by cleaner to calculate cost-benefit and select victims.
    seq:
      - id: segment_number
        type: u4
        doc: Segment number
      
      - id: live_bytes
        type: u4
        doc: Bytes of live data in segment
      
      - id: total_bytes
        type: u4
        doc: Total bytes in segment
      
      - id: last_modified
        type: u8
        doc: Unix timestamp of last modification
      
      - id: state
        type: u1
        enum: segment_state
        doc: Current state of segment
    
    instances:
      utilization:
        value: 'live_bytes == 0 ? 0.0 : (live_bytes.as<f8> / total_bytes.as<f8>)'
        doc: Utilization ratio (0.0 to 1.0)
      
      age:
        value: _root.superblock.last_mount_time - last_modified
        doc: Age in seconds since last modification
      
      cost_benefit_score:
        value: 'utilization >= 1.0 ? 0.0 : ((1.0 - utilization) * age) / (2.0 * utilization)'
        doc: Cost-benefit cleaning score (higher = better victim)

# ==============================================================================
# ENUMERATIONS
# ==============================================================================
enums:
  file_type:
    0x1000: fifo
    0x2000: char_device
    0x4000: directory
    0x6000: block_device
    0x8000: regular_file
    0xa000: symbolic_link
    0xc000: socket
  
  block_type:
    0: inode
    1: data
    2: indirect
    3: imap
    4: segment_summary
  
  segment_state:
    0: free
    1: active
    2: full
    3: cleaning
    4: clean

# ==============================================================================
# INSTANCES (File System Level)
# ==============================================================================
instances:
  active_checkpoint:
    value: |
      checkpoint_region_0.is_fully_valid and checkpoint_region_1.is_fully_valid
        ? (checkpoint_region_0.checkpoint_number > checkpoint_region_1.checkpoint_number
           ? checkpoint_region_0
           : checkpoint_region_1)
        : (checkpoint_region_0.is_fully_valid
           ? checkpoint_region_0
           : checkpoint_region_1)
    doc: |
      The most recent fully valid checkpoint (with valid state AND checksum).
      Prefers checkpoint with higher checkpoint_number when both are valid.
      Used for recovery and accessing current file system state.
  
  is_superblock_valid:
    value: superblock.checksum_valid
    doc: True if superblock checksum is valid
  
  has_valid_checkpoint:
    value: checkpoint_region_0.is_fully_valid or checkpoint_region_1.is_fully_valid
    doc: True if at least one checkpoint region is fully valid

