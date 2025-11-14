# Transaction System Header page - always page 5 of system tablespace (ibdata1)
# Reference: MySQL 8.0 Source (storage/innobase/include/trx0sys.h)
meta:
  id: innodb_page_trx_sys
  title: InnoDB Transaction System Header Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common
    - innodb_page_fsp_hdr

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: trx_sys_header
    type: trx_sys_header_t
  - id: empty_space
    size-eos: true
  - id: fil_trailer
    type: innodb_common::fil_trailer_t

types:
  trx_sys_header_t:
    seq:
      - id: trx_sys_magic
        type: u4
      - id: trx_id_high
        type: u8
      - id: doublewrite_magic
        type: u4
      - id: doublewrite_block1
        type: innodb_page_fsp_hdr::fil_addr_t
      - id: doublewrite_block2
        type: innodb_page_fsp_hdr::fil_addr_t
      - id: doublewrite_fseg_header
        size: 10
      - id: binlog_info
        type: binlog_info_t
      - id: rseg_array
        type: u4
        repeat: expr
        repeat-expr: 128

  binlog_info_t:
    seq:
      - id: binlog_file_name_len
        type: u4
      - id: binlog_file_name
        size: 512
        type: str
        encoding: ASCII
      - id: binlog_offset
        type: u8