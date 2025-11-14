# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_lob_first
  title: InnoDB LOB First Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: lob_first_header
    type: lob_first_header_t
    doc: LOB first page header
  
  - id: lob_data
    size-eos: true
    doc: Beginning of LOB data
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  lob_first_header_t:
    doc: |
      Header for LOB first pages.
    seq:
      - id: lob_version
        type: u1
        doc: LOB version
      
      - id: flags
        type: u1
        doc: LOB flags
      
      - id: reserved
        size: 6
        doc: Reserved
      
      - id: lob_total_len
        type: u8
        doc: Total LOB length
      
      - id: last_trx_id
        type: u8
        doc: Last modifying transaction

