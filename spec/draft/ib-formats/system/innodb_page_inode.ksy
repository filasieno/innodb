# Index Node (INODE) page containing file segment inode entries
# Reference: MySQL 8.0 Source (storage/innobase/include/fsp0fsp.h)
meta:
  id: innodb_page_inode
  title: InnoDB Index Node Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common
    - innodb_page_fsp_hdr

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
  - id: list_node
    type: innodb_page_fsp_hdr::flst_node_t
  - id: inodes
    type: fseg_inode_t
    repeat: expr
    repeat-expr: 85
  - id: empty_space
    size-eos: true
  - id: fil_trailer
    type: innodb_common::fil_trailer_t

types:
  fseg_inode_t:
    seq:
      - id: fseg_id
        type: u8
      - id: not_full_n_used
        type: u4
      - id: free_list
        type: innodb_page_fsp_hdr::flst_base_node_t
      - id: not_full_list
        type: innodb_page_fsp_hdr::flst_base_node_t
      - id: full_list
        type: innodb_page_fsp_hdr::flst_base_node_t
      - id: magic_n
        type: u4
      - id: frag_arr
        type: u4
        repeat: expr
        repeat-expr: 32
    instances:
      is_used:
        value: fseg_id != 0
      magic_valid:
        value: magic_n == 97937874