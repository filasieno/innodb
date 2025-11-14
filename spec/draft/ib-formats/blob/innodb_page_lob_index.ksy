# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_lob_index
  title: InnoDB LOB Index Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: lob_index_header
    type: lob_index_header_t
    doc: LOB index header
  
  - id: index_entries
    size-eos: true
    doc: LOB index entries pointing to data pages
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  lob_index_header_t:
    doc: |
      Header for LOB index pages.
    seq:
      - id: lob_version
        type: u1
        doc: LOB format version
      
      - id: flags
        type: u1
        doc: LOB flags
      
      - id: reserved
        size: 2
        doc: Reserved
      
      - id: lob_total_len
        type: u8
        doc: Total LOB length
      
      - id: last_trx_id
        type: u8
        doc: Last transaction modifying this LOB
      
      - id: last_undo_no
        type: u8
        doc: Last undo number

