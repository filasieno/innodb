# InnoDB Tablespace file (.ibd) container
# Reference: MySQL 8.0 Source (storage/innobase/include/fil0fil.h)
meta:
  id: innodb_tablespace
  title: InnoDB Tablespace File Container
  application: MySQL InnoDB Storage Engine
  file-extension:
    - ibd
  license: MIT
  endian: le
  imports:
    - ../innodb_common
    - ../system/innodb_page_allocated
    - ../system/innodb_page_fsp_hdr
    - ../system/innodb_page_xdes
    - ../system/innodb_page_inode
    - ../system/innodb_page_ibuf_bitmap
    - ../system/innodb_page_ibuf_free_list
    - ../system/innodb_page_sys
    - ../system/innodb_page_trx_sys
    - ../btree/innodb_page_index
    - ../btree/innodb_page_rtree
    - ../undo/innodb_page_undo_log
    - ../blob/innodb_page_blob
    - ../blob/innodb_page_zblob
    - ../blob/innodb_page_zblob2
    - ../blob/innodb_page_sdi_blob
    - ../blob/innodb_page_sdi_zblob
    - ../blob/innodb_page_lob_index
    - ../blob/innodb_page_lob_data
    - ../blob/innodb_page_lob_first
    - ../blob/innodb_page_zlob_first
    - ../blob/innodb_page_zlob_data
    - ../blob/innodb_page_zlob_index
    - ../blob/innodb_page_zlob_frag
    - ../blob/innodb_page_zlob_frag_entry

params:
  - id: page_size
    type: u4
    doc: Page size in bytes (default 16KB, can be 4KB, 8KB, 32KB, or 64KB)

seq:
  - id: pages
    type: page_wrapper_t
    repeat: eos
    doc: Sequence of pages in tablespace

types:
  page_wrapper_t:
    doc: Wrapper that reads FIL header and dispatches to appropriate page type
    seq:
      - id: page_data
        size: _root.page_size
        type: page_dispatcher_t
        doc: Page data (size determined by page_size parameter)

  page_dispatcher_t:
    doc: Dispatcher that parses page based on FIL header page type
    seq:
      - id: fil_header
        type: innodb_common::fil_header_t
        doc: Read FIL header to determine page type
      - id: page_body
        size-eos: true
        type:
          switch-on: fil_header.page_type
          cases:
            'innodb_common::page_type_enum::allocated': innodb_page_allocated
            'innodb_common::page_type_enum::fsp_hdr': innodb_page_fsp_hdr
            'innodb_common::page_type_enum::xdes': innodb_page_xdes
            'innodb_common::page_type_enum::inode': innodb_page_inode
            'innodb_common::page_type_enum::ibuf_bitmap': innodb_page_ibuf_bitmap
            'innodb_common::page_type_enum::ibuf_free_list': innodb_page_ibuf_free_list
            'innodb_common::page_type_enum::sys': innodb_page_sys
            'innodb_common::page_type_enum::trx_sys': innodb_page_trx_sys
            'innodb_common::page_type_enum::index': innodb_page_index
            'innodb_common::page_type_enum::rtree': innodb_page_rtree
            'innodb_common::page_type_enum::undo_log': innodb_page_undo_log
            'innodb_common::page_type_enum::blob': innodb_page_blob
            'innodb_common::page_type_enum::zblob': innodb_page_zblob
            'innodb_common::page_type_enum::zblob2': innodb_page_zblob2
            'innodb_common::page_type_enum::sdi_blob': innodb_page_sdi_blob
            'innodb_common::page_type_enum::sdi_zblob': innodb_page_sdi_zblob
            'innodb_common::page_type_enum::lob_index': innodb_page_lob_index
            'innodb_common::page_type_enum::lob_data': innodb_page_lob_data
            'innodb_common::page_type_enum::lob_first': innodb_page_lob_first
            'innodb_common::page_type_enum::zlob_first_v2': innodb_page_zlob_first
            'innodb_common::page_type_enum::zlob_data_v2': innodb_page_zlob_data
            'innodb_common::page_type_enum::zlob_index_v2': innodb_page_zlob_index
            'innodb_common::page_type_enum::zlob_frag': innodb_page_zlob_frag
            'innodb_common::page_type_enum::zlob_frag_entry': innodb_page_zlob_frag_entry
        doc: Page body parsed according to type

instances:
  actual_page_size:
    value: _root.page_size
    doc: Actual page size being used