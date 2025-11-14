# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_zlob_frag_entry
  title: InnoDB Compressed LOB Fragment Entry Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: frag_entry_header
    type: frag_entry_header_t
    doc: Fragment entry header
  
  - id: frag_entries
    size-eos: true
    doc: Fragment entry directory
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  frag_entry_header_t:
    doc: |
      Header for fragment entry pages.
    seq:
      - id: n_entries
        type: u4
        doc: Number of fragment entries
      
      - id: used_len
        type: u4
        doc: Used length
      
      - id: trx_id
        type: u8
        doc: Transaction ID

