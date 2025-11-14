# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_zlob_data
  title: InnoDB Compressed LOB Data Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: zlob_data_header
    type: zlob_data_header_t
    doc: Compressed LOB data header
  
  - id: compressed_data
    size-eos: true
    doc: Compressed data chunk
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  zlob_data_header_t:
    doc: |
      Header for compressed LOB data pages.
    seq:
      - id: data_len
        type: u4
        doc: Length of compressed data in this page
      
      - id: trx_id
        type: u8
        doc: Transaction ID

