# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_xdes
  title: InnoDB Extent Descriptor Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common
    - innodb_page_fsp_hdr

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: xdes_array
    type: innodb_page_fsp_hdr::xdes_entry_t
    repeat: expr
    repeat-expr: 256
    doc: |
      Array of 256 extent descriptors.
      Each describes 64 pages, so this covers 16384 pages total.
  
  - id: empty_space
    size-eos: true
    doc: Remaining page space (unused in XDES page)
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

