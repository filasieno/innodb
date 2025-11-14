# SDI BLOB page (Serialized Dictionary Information)
# Reference: MySQL 8.0 Source (storage/innobase/include/dict0sdi.h)
meta:
  id: innodb_page_sdi_blob
  title: InnoDB SDI BLOB Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: sdi_header
    type: sdi_header_t
  - id: sdi_json_data
    size-eos: true
    type: str
    encoding: UTF-8
  - id: fil_trailer
    type: innodb_common::fil_trailer_t

types:
  sdi_header_t:
    doc: |
      Header for SDI BLOB pages.
    seq:
      - id: sdi_version
        type: u4
        doc: SDI version number
      
      - id: sdi_type
        type: u4
        doc: Type of SDI object (table, tablespace, etc.)
      
      - id: sdi_id
        type: u8
        doc: Object ID
      
      - id: data_len
        type: u4
        doc: Length of JSON data
      
      - id: next_page_no
        type: u4
        doc: Next SDI BLOB page (0xFFFFFFFF = last)

