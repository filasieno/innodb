# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_lob_data
  title: InnoDB LOB Data Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: lob_data_header
    type: lob_data_header_t
    doc: LOB data header
  
  - id: lob_data
    size-eos: true
    doc: Actual LOB data
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  lob_data_header_t:
    doc: |
      Header for LOB data pages.
    seq:
      - id: data_len
        type: u4
        doc: Length of data in this page
      
      - id: trx_id
        type: u8
        doc: Transaction ID that created this version

