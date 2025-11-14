# InnoDB Undo Tablespace (undo_*) container (MySQL 8.0+)
# Reference: MySQL 8.0 Source (storage/innobase/include/trx0undo.h, trx0rseg.h)
meta:
  id: innodb_undo_tablespace
  title: InnoDB Undo Tablespace Container
  application: MySQL InnoDB Storage Engine
  file-extension:
    - undo_001
    - undo_002
  license: MIT
  endian: le
  imports:
    - ../innodb_common
    - ../system/innodb_page_fsp_hdr
    - ../system/innodb_page_xdes
    - ../undo/innodb_page_undo_log
    - ../undo/innodb_page_rseg_array

params:
  - id: page_size
    type: u4
    doc: Page size (default 16KB)

seq:
  - id: pages
    type: page_wrapper_t
    repeat: eos
    doc: Pages in undo tablespace

types:
  page_wrapper_t:
    doc: Page wrapper for undo tablespace pages
    seq:
      - id: page_data
        size: _root.page_size
        type: page_dispatcher_t

  page_dispatcher_t:
    doc: Dispatcher for undo tablespace page types
    seq:
      - id: fil_header
        type: innodb_common::fil_header_t
      - id: page_body
        size-eos: true
        type:
          switch-on: fil_header.page_type
          cases:
            'innodb_common::page_type_enum::fsp_hdr': innodb_page_fsp_hdr
            'innodb_common::page_type_enum::xdes': innodb_page_xdes
            'innodb_common::page_type_enum::undo_log': innodb_page_undo_log

instances:
  fsp_header:
    value: pages[0]
    doc: FSP header (page 0)
  rseg_array:
    value: pages[3]
    doc: Rollback segment array (typically page 3)