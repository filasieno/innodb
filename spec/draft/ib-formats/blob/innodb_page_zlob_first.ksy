# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_zlob_first
  title: InnoDB Compressed LOB First Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: zlob_first_header
    type: zlob_first_header_t
    doc: Compressed LOB first page header
  
  - id: compressed_data
    size-eos: true
    doc: Compressed LOB data
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  zlob_first_header_t:
    doc: |
      Header for compressed LOB first pages.
    seq:
      - id: lob_version
        type: u1
        doc: LOB version
      
      - id: flags
        type: u1
        doc: Compression and other flags
      
      - id: reserved
        size: 2
        doc: Reserved
      
      - id: compressed_len
        type: u8
        doc: Compressed length
      
      - id: uncompressed_len
        type: u8
        doc: Uncompressed length
      
      - id: last_trx_id
        type: u8
        doc: Last transaction ID

