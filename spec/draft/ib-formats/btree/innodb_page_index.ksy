# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_index
  title: InnoDB B-tree Index Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common
    - ../innodb_record_format

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: index_header
    type: index_header_t
    doc: Index page header (36 bytes)
  
  - id: fseg_header
    type: fseg_header_t
    doc: File segment header (20 bytes, only on root page)
  
  - id: system_records
    type: system_records_t
    doc: Infimum and supremum records
  
  - id: user_records_and_free_space
    size-eos: true
    doc: |
      User records and free space.
      Actual parsing requires index metadata to interpret record format.
  
  - id: page_directory
    type: u2
    repeat: expr
    repeat-expr: index_header.n_dir_slots
    doc: |
      Page directory - array of record offsets.
      Each slot points to a record that "owns" a group of records.
      Used for binary search within page.
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  # ============================================================================
  # INDEX PAGE HEADER (36 bytes)
  # ============================================================================
  index_header_t:
    doc: |
      Index page header containing page-specific metadata.
      Located at offset 38 (after FIL header).
    seq:
      - id: n_dir_slots
        type: u2
        doc: Number of slots in page directory
      
      - id: heap_top
        type: u2
        doc: |
          Offset of record heap top. Records are allocated from heap.
          Heap grows from top of page downward.
      
      - id: n_heap
        type: u2
        doc: |
          Number of records in heap (including infimum, supremum, deleted).
          Bit 15 (0x8000) indicates if page uses COMPACT format.
      
      - id: free_offset
        type: u2
        doc: |
          Offset to start of free record list.
          0xFFFF if no free records.
      
      - id: garbage_bytes
        type: u2
        doc: Number of bytes in deleted records (garbage)
      
      - id: last_insert_offset
        type: u2
        doc: Offset of last inserted record (for insert direction optimization)
      
      - id: direction
        type: u2
        enum: insert_direction_enum
        doc: Last insert direction (left, right, or unknown)
      
      - id: n_direction
        type: u2
        doc: |
          Number of consecutive inserts in same direction.
          Used to detect sequential insert patterns.
      
      - id: n_recs
        type: u2
        doc: Number of user records on page (excludes infimum/supremum/deleted)
      
      - id: max_trx_id
        type: u8
        doc: |
          Maximum transaction ID that modified this page.
          Used for MVCC and purge.
      
      - id: level
        type: u2
        doc: |
          Level of this page in B-tree (0 = leaf, >0 = internal node).
          Leaf pages contain actual data, internal nodes contain child pointers.
      
      - id: index_id
        type: u8
        doc: Index ID that this page belongs to
    
    instances:
      is_compact:
        value: (n_heap & 0x8000) != 0
        doc: True if page uses COMPACT record format
      
      actual_n_heap:
        value: n_heap & 0x7FFF
        doc: Actual number of heap records (without format flag)
      
      is_leaf:
        value: level == 0
        doc: True if this is a leaf page

  # ============================================================================
  # FILE SEGMENT HEADER (20 bytes, only on root page)
  # ============================================================================
  fseg_header_t:
    doc: |
      File segment header found on root pages of B-tree indexes.
      Contains pointers to leaf and internal node segments.
    seq:
      - id: leaf_inode_space
        type: u4
        doc: Space ID of leaf segment inode
      
      - id: leaf_inode_page_no
        type: u4
        doc: Page number of leaf segment inode
      
      - id: leaf_inode_offset
        type: u2
        doc: Offset within page of leaf segment inode
      
      - id: internal_inode_space
        type: u4
        doc: Space ID of internal node segment inode
      
      - id: internal_inode_page_no
        type: u4
        doc: Page number of internal node segment inode
      
      - id: internal_inode_offset
        type: u2
        doc: Offset within page of internal node segment inode

  # ============================================================================
  # SYSTEM RECORDS (26 bytes total)
  # ============================================================================
  system_records_t:
    doc: |
      Infimum and supremum records - boundary records on every index page.
      
      Infimum: Minimum possible record (all searches start here)
      Supremum: Maximum possible record (marks end of page)
    seq:
      - id: infimum
        type: infimum_supremum_record_t
        doc: Infimum record (13 bytes)
      
      - id: supremum
        type: infimum_supremum_record_t
        doc: Supremum record (13 bytes)

  infimum_supremum_record_t:
    doc: |
      Structure of infimum/supremum records.
      These are special system records with fixed format.
    seq:
      - id: record_header
        size: 5
        doc: Record header (COMPACT format)
      
      - id: data
        size: 8
        doc: |
          Data portion:
          Infimum: "infimum" (8 bytes)
          Supremum: "supremum" (8 bytes)

# ==============================================================================
# ENUMERATIONS
# ==============================================================================
enums:
  insert_direction_enum:
    1: left
    2: right
    3: same_rec
    4: same_page
    5: no_direction

