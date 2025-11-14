# Insert Buffer Bitmap page
# Reference: MySQL 8.0 Source (storage/innobase/include/ibuf0ibuf.h)
meta:
  id: innodb_page_ibuf_bitmap
  title: InnoDB Insert Buffer Bitmap Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: bitmap_data
    size-eos: true
  - id: fil_trailer
    type: innodb_common::fil_trailer_t

