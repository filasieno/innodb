# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_zlob_frag
  title: InnoDB Compressed LOB Fragment Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: zlob_frag_header
    type: zlob_frag_header_t
    doc: Compressed LOB fragment header
  
  - id: fragment_data
    size-eos: true
    doc: Fragment data
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  zlob_frag_header_t:
    doc: |
      Header for compressed LOB fragment pages.
    seq:
      - id: n_frags
        type: u4
        doc: Number of fragments in this page
      
      - id: used_len
        type: u4
        doc: Used length in page
      
      - id: trx_id
        type: u8
        doc: Transaction ID

