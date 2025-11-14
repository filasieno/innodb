# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_zlob_index
  title: InnoDB Compressed LOB Index Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: zlob_index_header
    type: zlob_index_header_t
    doc: Compressed LOB index header
  
  - id: index_entries
    size-eos: true
    doc: Index entries for compressed LOB pages
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  zlob_index_header_t:
    doc: |
      Header for compressed LOB index pages.
    seq:
      - id: lob_version
        type: u1
        doc: LOB version
      
      - id: flags
        type: u1
        doc: Flags
      
      - id: reserved
        size: 2
        doc: Reserved
      
      - id: total_compressed_len
        type: u8
        doc: Total compressed length
      
      - id: total_uncompressed_len
        type: u8
        doc: Total uncompressed length

