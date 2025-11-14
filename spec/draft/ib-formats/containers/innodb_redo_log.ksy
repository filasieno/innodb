# $schema=../../schema/kaitai.schema.json
meta:
  id: innodb_redo_log
  title: InnoDB Redo Log
  application: MySQL InnoDB Storage Engine
  file-extension:
    - ib_logfile0
    - ib_logfile1
  license: MIT
  endian: le

seq:
  - id: file_header
    type: file_header_t
    doc: Redo log file header (first 512 bytes)
  
  - id: checkpoint_1
    type: checkpoint_block_t
    doc: First checkpoint block (blocks 1-2)
  
  - id: checkpoint_2
    type: checkpoint_block_t
    doc: Second checkpoint block (blocks 3-4, alternate with checkpoint_1)
  
  - id: log_blocks
    type: log_block_t
    repeat: eos
    doc: Sequence of 512-byte log blocks containing redo records

types:
  # ============================================================================
  # FILE HEADER
  # ============================================================================
  file_header_t:
    doc: |
      The file header occupies the first 512-byte block (block 0) of each
      redo log file. It contains metadata about the log file including
      format version, starting LSN, and file identification.
    seq:
      - id: magic
        contents: [0x49, 0x42, 0x4c, 0x4f, 0x47]  # "IBLOG"
        doc: Magic number identifying InnoDB redo log
      
      - id: format_version
        type: u4
        doc: |
          Redo log format version number. MySQL 8.0 uses format version 2.
          Earlier versions used format 1 or 0.
      
      - id: start_lsn
        type: u8
        doc: |
          Log Sequence Number (LSN) of the first log record in this file.
          The LSN is a monotonically increasing value that uniquely identifies
          a position in the redo log.
      
      - id: creator_name
        type: str
        size: 32
        encoding: ASCII
        doc: Name of MySQL version that created this log file
      
      - id: log_flags
        type: u4
        doc: |
          Flags indicating log file state. Bit 0x1 indicates log is being
          created, 0x2 indicates crash recovery needed.
      
      - id: log_uuid
        size: 16
        doc: |
          Unique identifier (UUID) for this redo log. Used to match redo logs
          with data files and detect log file mismatches.
      
      - id: header_checksum
        type: u4
        doc: CRC32 checksum of file header fields
      
      - id: reserved
        size: 512 - _io.pos
        doc: Reserved space padding to 512 bytes

  # ============================================================================
  # CHECKPOINT BLOCK
  # ============================================================================
  checkpoint_block_t:
    doc: |
      Checkpoint blocks store information about the consistent state of the
      database at a point in time. Two checkpoint regions alternate - when
      one is being written, the other contains the last valid checkpoint.
      Each checkpoint region consists of two consecutive 512-byte blocks.
    seq:
      - id: block_1
        type: log_block_t
        doc: First block of checkpoint region
      
      - id: block_2
        type: log_block_t
        doc: Second block of checkpoint region
    
    instances:
      checkpoint_no:
        pos: 12
        type: u8
        doc: Checkpoint sequence number (monotonically increasing)
      
      checkpoint_lsn:
        pos: 20
        type: u8
        doc: LSN up to which all changes have been flushed to disk
      
      checkpoint_offset:
        pos: 28
        type: u8
        doc: Byte offset within redo log files where checkpoint_lsn is located

  # ============================================================================
  # LOG BLOCK
  # ============================================================================
  log_block_t:
    doc: |
      Standard 512-byte redo log block. Each block contains a 12-byte header,
      496 bytes of log record data, and a 4-byte trailer with checksum.
      Log blocks are numbered sequentially with a 32-bit block number that
      wraps around after reaching the maximum value.
    seq:
      - id: header
        type: log_block_header_t
        doc: 12-byte block header
      
      - id: data
        size: 496
        doc: |
          Log record data section. Contains one or more redo log records.
          The actual used length is specified in header.data_len.
      
      - id: trailer
        type: log_block_trailer_t
        doc: 4-byte block trailer with checksum
    
    instances:
      block_number:
        value: header.hdr_no
        doc: Sequential block number (from header)
      
      has_valid_data:
        value: header.data_len > 0 and header.data_len <= 496
        doc: Whether this block contains valid log data
      
      log_records:
        pos: 12
        size: header.data_len
        type: log_records_t
        if: has_valid_data
        doc: Parsed log records from data section

  # ============================================================================
  # LOG BLOCK HEADER
  # ============================================================================
  log_block_header_t:
    doc: |
      12-byte header at the start of each log block. Contains metadata about
      the block including its sequence number, data length, and pointers to
      help locate log records within the block.
    seq:
      - id: hdr_no
        type: u4
        doc: |
          Log block number. This is a sequential counter that wraps around.
          The highest bit (0x80000000) indicates this is the first block
          after a flush operation.
      
      - id: data_len
        type: u2
        doc: |
          Number of bytes of log data written to this block (0-496).
          When data_len < 496, remaining bytes in data section are undefined.
      
      - id: first_rec_group
        type: u2
        doc: |
          Offset (from start of data section) to the first log record group
          that starts in this block. If 0, no new record group starts here.
      
      - id: checkpoint_no
        type: u4
        doc: |
          Checkpoint number when this block was written. Used during recovery
          to determine which checkpoint is more recent.
    
    instances:
      is_flush_bit_set:
        value: (hdr_no & 0x80000000) != 0
        doc: True if this block follows a flush operation
      
      block_no_without_flush_bit:
        value: hdr_no & 0x7FFFFFFF
        doc: Block number with flush bit masked out

  # ============================================================================
  # LOG BLOCK TRAILER
  # ============================================================================
  log_block_trailer_t:
    doc: 4-byte trailer containing checksum for block integrity verification
    seq:
      - id: checksum
        type: u4
        doc: |
          CRC32 checksum of the entire log block (header + data).
          Used to detect corruption in redo log blocks.

  # ============================================================================
  # LOG RECORDS
  # ============================================================================
  log_records_t:
    doc: |
      Container for one or more redo log records within a block's data section.
      Each record describes a single modification to a tablespace page.
    seq:
      - id: records
        type: log_record_t
        repeat: eos
        doc: Sequence of redo log records

  # ============================================================================
  # LOG RECORD
  # ============================================================================
  log_record_t:
    doc: |
      Individual redo log record describing a single change to a page.
      Each record has a type, space ID, page number, and type-specific data.
      Records are variable length and tightly packed.
    seq:
      - id: type
        type: u1
        enum: mlog_type
        doc: |
          Record type (MLOG_* constant). Determines the format of the record
          data and what operation it represents.
      
      - id: space_id
        type: compressed_uint_t
        if: type != mlog_type::mlog_multi_rec_end
        doc: |
          Tablespace ID where the modification occurred. Compressed format
          saves space for common small space IDs.
      
      - id: page_no
        type: compressed_uint_t
        if: type != mlog_type::mlog_multi_rec_end and type != mlog_type::mlog_checkpoint
        doc: Page number within the tablespace being modified
      
      - id: record_data
        type:
          switch-on: type
          cases:
            mlog_type::mlog_1byte: write_1byte_t
            mlog_type::mlog_2bytes: write_2bytes_t
            mlog_type::mlog_4bytes: write_4bytes_t
            mlog_type::mlog_8bytes: write_8bytes_t
            mlog_type::mlog_write_string: write_string_t
            mlog_type::mlog_rec_insert: rec_insert_t
            mlog_type::mlog_rec_clust_delete_mark: rec_delete_mark_t
            mlog_type::mlog_rec_update_in_place: rec_update_t
            mlog_type::mlog_page_create: page_create_t
            mlog_type::mlog_undo_insert: undo_insert_t
            mlog_type::mlog_undo_erase_end: undo_erase_t
            mlog_type::mlog_checkpoint: checkpoint_record_t
            _: generic_record_data_t
        doc: Type-specific record data

  # ============================================================================
  # COMPRESSED UNSIGNED INTEGER
  # ============================================================================
  compressed_uint_t:
    doc: |
      Variable-length compressed unsigned integer encoding used throughout
      redo log to save space. Small values use fewer bytes:
      - Values < 0x80: 1 byte
      - Values < 0x4000: 2 bytes
      - Values < 0x200000: 3 bytes
      - Values < 0x10000000: 4 bytes
      - Larger values: 5 bytes
    seq:
      - id: first_byte
        type: u1
      
      - id: second_byte
        type: u1
        if: first_byte >= 0x80
      
      - id: third_byte
        type: u1
        if: first_byte >= 0xC0
      
      - id: fourth_byte
        type: u1
        if: first_byte >= 0xE0
      
      - id: fifth_byte
        type: u1
        if: first_byte >= 0xF0
    
    instances:
      value:
        value: |
          first_byte < 0x80 ? first_byte :
          first_byte < 0xC0 ? ((first_byte & 0x3F) << 8) | second_byte :
          first_byte < 0xE0 ? ((first_byte & 0x1F) << 16) | (second_byte << 8) | third_byte :
          first_byte < 0xF0 ? ((first_byte & 0x0F) << 24) | (second_byte << 16) | (third_byte << 8) | fourth_byte :
          (first_byte.as<u8> << 32) | (second_byte.as<u8> << 24) | (third_byte.as<u8> << 16) | (fourth_byte.as<u8> << 8) | fifth_byte
        doc: Decompressed unsigned integer value

  # ============================================================================
  # RECORD DATA TYPES
  # ============================================================================
  
  write_1byte_t:
    doc: Write 1 byte to a page at specified offset
    seq:
      - id: offset
        type: u2
        doc: Offset within page
      - id: value
        type: u1
        doc: Byte value to write

  write_2bytes_t:
    doc: Write 2 bytes to a page at specified offset
    seq:
      - id: offset
        type: u2
        doc: Offset within page
      - id: value
        type: u2
        doc: 2-byte value to write

  write_4bytes_t:
    doc: Write 4 bytes to a page at specified offset
    seq:
      - id: offset
        type: u2
        doc: Offset within page
      - id: value
        type: u4
        doc: 4-byte value to write

  write_8bytes_t:
    doc: Write 8 bytes to a page at specified offset
    seq:
      - id: offset
        type: u2
        doc: Offset within page
      - id: value
        type: u8
        doc: 8-byte value to write

  write_string_t:
    doc: Write a string of bytes to a page at specified offset
    seq:
      - id: offset
        type: u2
        doc: Offset within page
      - id: length
        type: compressed_uint_t
        doc: Length of string in bytes
      - id: data
        size: length.value
        doc: String data to write

  rec_insert_t:
    doc: Insert a record into a B-tree page
    seq:
      - id: offset
        type: u2
        doc: Offset where record is inserted
      - id: rec_len
        type: compressed_uint_t
        doc: Length of record
      - id: record_data
        size: rec_len.value
        doc: Complete record data including header

  rec_delete_mark_t:
    doc: Mark a clustered index record as deleted
    seq:
      - id: offset
        type: u2
        doc: Offset of record to mark
      - id: flags
        type: u1
        doc: Delete mark flags (1 = mark deleted, 0 = unmark)

  rec_update_t:
    doc: Update a record in place (without reorganizing page)
    seq:
      - id: offset
        type: u2
        doc: Offset of record being updated
      - id: update_vector_len
        type: compressed_uint_t
        doc: Number of fields being updated
      - id: update_fields
        type: update_field_t
        repeat: expr
        repeat-expr: update_vector_len.value
        doc: Array of field updates

  update_field_t:
    doc: Single field update within a record update operation
    seq:
      - id: field_no
        type: compressed_uint_t
        doc: Field number being updated
      - id: field_len
        type: compressed_uint_t
        doc: New field length
      - id: field_data
        size: field_len.value
        doc: New field data

  page_create_t:
    doc: Create a new page in the buffer pool
    seq:
      - id: page_type
        type: u2
        doc: |
          Type of page being created:
          0 = uncompressed, 1 = compressed
      - id: index_id
        type: u8
        doc: Index ID if this is an index page

  undo_insert_t:
    doc: Insert a record into an undo log page
    seq:
      - id: offset
        type: u2
        doc: Offset where undo record is inserted
      - id: len
        type: compressed_uint_t
        doc: Length of undo record
      - id: data
        size: len.value
        doc: Undo record data

  undo_erase_t:
    doc: Erase end portion of an undo log page
    seq:
      - id: offset
        type: u2
        doc: Offset from which to erase to page end

  checkpoint_record_t:
    doc: Checkpoint record marking a consistent database state
    seq:
      - id: checkpoint_lsn
        type: u8
        doc: LSN of this checkpoint
      - id: checkpoint_no
        type: u8
        doc: Checkpoint sequence number

  generic_record_data_t:
    doc: Generic record data for unspecified or custom record types
    seq:
      - id: data
        size-eos: true
        doc: Raw record data (format depends on record type)

# ==============================================================================
# ENUMERATIONS
# ==============================================================================
enums:
  mlog_type:
    # Single byte write
    1: mlog_1byte
    # 2 bytes write
    2: mlog_2bytes
    # 4 bytes write
    4: mlog_4bytes
    # 8 bytes write
    8: mlog_8bytes
    # Record insert
    9: mlog_rec_insert
    # Mark clustered index record deleted
    10: mlog_rec_clust_delete_mark
    # Mark secondary index record deleted
    11: mlog_rec_sec_delete_mark
    # Update record in place
    13: mlog_rec_update_in_place
    # Delete a record from a page
    14: mlog_rec_delete
    # Set next record pointer
    15: mlog_list_end_delete
    # List start delete
    16: mlog_list_start_delete
    # Copy record list end to a new created page
    17: mlog_list_end_copy_created
    # Reorganize a page
    18: mlog_page_reorganize
    # Create a page
    19: mlog_page_create
    # Insert entry in an undo log
    20: mlog_undo_insert
    # Erase undo log page end
    21: mlog_undo_erase_end
    # Initialize an undo log header
    22: mlog_undo_init
    # Reuse an insert undo log header
    23: mlog_undo_hdr_reuse
    # Create an undo log header
    24: mlog_undo_hdr_create
    # Mark an index record as the predefined minimum
    25: mlog_rec_min_mark
    # Initialize an ibuf bitmap page
    26: mlog_ibuf_bitmap_init
    # Initialize a file segment inode page
    27: mlog_init_file_page
    # Write a string of bytes
    30: mlog_write_string
    # Mark end of multi-record group
    31: mlog_multi_rec_end
    # Checkpoint record
    32: mlog_checkpoint
    # Create a compressed page
    34: mlog_page_create_compressed
    # Compressed page reorganize
    36: mlog_page_create_rtree
    # Initialize SDI page
    37: mlog_comp_rec_min_mark
    # Compensation log record
    38: mlog_comp_page_create
    # Compressed record insert
    39: mlog_comp_rec_insert
    # Mark compressed clustered index record deleted
    40: mlog_comp_rec_clust_delete_mark
    # Mark compressed secondary index record deleted
    41: mlog_comp_rec_sec_delete_mark
    # Update compressed record in place
    42: mlog_comp_rec_update_in_place
    # Delete compressed record
    43: mlog_comp_rec_delete
    # Copy compressed record list end to new page
    44: mlog_comp_list_end_delete
    # Copy compressed record list start to new page
    45: mlog_comp_list_start_delete
    # Copy compressed record list end to created page
    46: mlog_comp_list_end_copy_created
    # Reorganize compressed page
    47: mlog_comp_page_reorganize
    # Create an R-tree index page
    48: mlog_file_create
    # Rename a tablespace file
    49: mlog_file_rename
    # Delete a tablespace file
    50: mlog_file_delete
    # Mark a page as free in FSP header
    51: mlog_file_create2
    # Rename a tablespace file (new format)
    52: mlog_file_rename2
    # Truncate a tablespace
    55: mlog_truncate
    # Index load
    56: mlog_index_load
    # Table metadata
    57: mlog_table_dynamic_meta
    # Page init
    58: mlog_page_init
    # Zip page alloc
    59: mlog_zip_page_compress
    # Write log record
    60: mlog_test

# ==============================================================================
# INSTANCES (File Level)
# ==============================================================================
instances:
  active_checkpoint:
    value: |
      checkpoint_1.checkpoint_no > checkpoint_2.checkpoint_no
        ? checkpoint_1
        : checkpoint_2
    doc: |
      The most recent valid checkpoint (with higher checkpoint_no).
      Used during crash recovery to determine the starting point for
      log replay.
  
  log_format_version:
    value: file_header.format_version
    doc: Redo log format version from file header

