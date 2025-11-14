# Compressed BLOB continuation page
# Reference: MySQL 8.0 Source (storage/innobase/include/lob0lob.h)
meta:
  id: innodb_page_zblob2
  title: InnoDB Compressed BLOB Continuation Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: next_page_no
    type: u4
  - id: compressed_data
    size-eos: true
  - id: fil_trailer
    type: innodb_common::fil_trailer_t

