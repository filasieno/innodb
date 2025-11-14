# Compressed BLOB page (first page of compressed BLOB)
# Reference: MySQL 8.0 Source (storage/innobase/include/lob0lob.h)
meta:
  id: innodb_page_zblob
  title: InnoDB Compressed BLOB Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: zblob_header
    type: zblob_header_t
  - id: compressed_data
    size-eos: true
  - id: fil_trailer
    type: innodb_common::fil_trailer_t

types:
  zblob_header_t:
    doc: |
      Header for compressed BLOB first page.
    seq:
      - id: blob_version
        type: u1
        doc: BLOB version (compression format version)
      
      - id: reserved
        size: 3
        doc: Reserved bytes
      
      - id: compressed_len
        type: u4
        doc: Total compressed length
      
      - id: uncompressed_len
        type: u4
        doc: Total uncompressed length
      
      - id: next_page_no
        type: u4
        doc: Next page in compressed BLOB chain (0xFFFFFFFF = last)

