# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_fsp_hdr
  title: InnoDB File Space Header Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: fsp_header
    type: fsp_header_t
    doc: File space header (112 bytes)
  
  - id: xdes_array
    type: xdes_entry_t
    repeat: expr
    repeat-expr: 256
    doc: |
      Array of 256 extent descriptors.
      Each extent is 64 pages (1MB for 16KB pages).
      This array describes the first 16384 pages of the tablespace.
  
  - id: empty_space
    size-eos: true
    doc: Remaining page space (unused in FSP_HDR page)
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  # ============================================================================
  # FSP HEADER (112 bytes)
  # ============================================================================
  fsp_header_t:
    doc: |
      File space header containing global tablespace information.
      Located at offset 38 (after FIL header) on page 0.
      Total size: 112 bytes
    seq:
      - id: space_id
        type: u4
        doc: Tablespace identifier
      
      - id: unused
        type: u4
        doc: Reserved, unused
      
      - id: size
        type: u4
        doc: Current size of tablespace in pages
      
      - id: free_limit
        type: u4
        doc: |
          Free space limit - pages beyond this are uninitialized.
          Used for extending tablespace.
      
      - id: space_flags
        type: innodb_common::space_flags_t
        doc: Tablespace flags (page size, format, compression, etc.)
      
      - id: frag_n_used
        type: u4
        doc: Number of used pages in fragment list
      
      - id: free_list
        type: flst_base_node_t
        doc: Base node of free extent list
      
      - id: free_frag_list
        type: flst_base_node_t
        doc: Base node of free fragment extent list
      
      - id: full_frag_list
        type: flst_base_node_t
        doc: Base node of full fragment extent list
      
      - id: next_unused_seg_id
        type: u8
        doc: Next unused segment ID
      
      - id: full_inodes_list
        type: flst_base_node_t
        doc: Base node of full inode page list
      
      - id: free_inodes_list
        type: flst_base_node_t
        doc: Base node of free inode page list

  # ============================================================================
  # FILE LIST BASE NODE (16 bytes)
  # ============================================================================
  flst_base_node_t:
    doc: |
      Base node of a file-based list.
      InnoDB uses doubly-linked lists stored across pages.
      This structure tracks the list head and tail.
    seq:
      - id: length
        type: u4
        doc: Number of nodes in list
      
      - id: first_node
        type: fil_addr_t
        doc: File address of first node
      
      - id: last_node
        type: fil_addr_t
        doc: File address of last node

  # ============================================================================
  # FILE ADDRESS (6 bytes)
  # ============================================================================
  fil_addr_t:
    doc: |
      File address - points to a location within the tablespace.
      Consists of page number and offset within page.
    seq:
      - id: page_no
        type: u4
        doc: Page number
      
      - id: byte_offset
        type: u2
        doc: Byte offset within page
    
    instances:
      is_null:
        value: page_no == 0xFFFFFFFF
        doc: True if this is a null pointer

  # ============================================================================
  # EXTENT DESCRIPTOR (40 bytes)
  # ============================================================================
  xdes_entry_t:
    doc: |
      Extent descriptor (XDES) entry describing one extent.
      
      An extent is a group of 64 consecutive pages (1MB for 16KB pages).
      The descriptor tracks which pages in the extent are free/used.
    seq:
      - id: file_segment_id
        type: u8
        doc: |
          ID of file segment owning this extent.
          0 = extent is free
      
      - id: list_node
        type: flst_node_t
        doc: List node for linking in free/full/fragment lists
      
      - id: state
        type: u4
        enum: xdes_state_enum
        doc: State of extent (free, free_frag, full_frag, fseg)
      
      - id: page_state_bitmap
        size: 16
        doc: |
          Bitmap of page states within extent.
          2 bits per page × 64 pages = 128 bits = 16 bytes
          
          Bit values:
          00 = free
          01 = allocated but not used
          10 = allocated and used (contains data)
          11 = reserved
    
    instances:
      is_free:
        value: file_segment_id == 0
        doc: True if extent is free (not owned by any segment)

  # ============================================================================
  # FILE LIST NODE (12 bytes)
  # ============================================================================
  flst_node_t:
    doc: |
      File list node - part of doubly-linked list structure.
      Contains pointers to previous and next nodes.
    seq:
      - id: prev
        type: fil_addr_t
        doc: File address of previous node
      
      - id: next
        type: fil_addr_t
        doc: File address of next node

# ==============================================================================
# ENUMERATIONS
# ==============================================================================
enums:
  xdes_state_enum:
    1: free
    2: free_frag
    3: full_frag
    4: fseg
    5: fseg_frag

