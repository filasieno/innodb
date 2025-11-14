# Freshly Allocated page - not yet initialized with specific type
# Reference: MySQL 8.0 Source (storage/innobase/include/fil0fil.h)
meta:
  id: innodb_page_allocated
  title: InnoDB Freshly Allocated Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: empty_space
    size-eos: true
  - id: fil_trailer
    type: innodb_common::fil_trailer_t