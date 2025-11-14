# Rollback Segment Array page (MySQL 8.0+)
# Reference: MySQL 8.0 Source (storage/innobase/include/trx0rseg.h)
meta:
  id: innodb_page_rseg_array
  title: InnoDB Rollback Segment Array Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: rseg_array_header
    type: rseg_array_header_t
  - id: rseg_slots
    type: u4
    repeat: expr
    repeat-expr: rseg_array_header.max_rollback_segments
  - id: empty_space
    size-eos: true
  - id: fil_trailer
    type: innodb_common::fil_trailer_t

types:
  rseg_array_header_t:
    seq:
      - id: max_rollback_segments
        type: u4
      - id: rseg_array_size
        type: u4
      - id: rseg_array_version
        type: u4