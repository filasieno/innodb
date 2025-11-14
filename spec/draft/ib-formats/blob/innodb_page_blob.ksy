# Uncompressed BLOB page for storing large object data
# Reference: MySQL 8.0 Source (storage/innobase/include/lob0lob.h)
meta:
  id: innodb_page_blob
  title: InnoDB Uncompressed BLOB Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: blob_header
    type: blob_header_t
  - id: blob_data
    size-eos: true
  - id: fil_trailer
    type: innodb_common::fil_trailer_t

types:
  blob_header_t:
    seq:
      - id: blob_part_len
        type: u4
      - id: next_page_no
        type: u4
    instances:
      has_next:
        value: next_page_no != 0xFFFFFFFF