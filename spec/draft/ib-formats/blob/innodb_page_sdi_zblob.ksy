# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_sdi_zblob
  title: InnoDB SDI Compressed BLOB Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: sdi_zblob_header
    type: sdi_zblob_header_t
    doc: Compressed SDI header
  
  - id: compressed_sdi_data
    size-eos: true
    doc: Compressed SDI JSON data (zlib)
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  sdi_zblob_header_t:
    doc: |
      Header for compressed SDI BLOB pages.
    seq:
      - id: sdi_version
        type: u4
        doc: SDI version
      
      - id: sdi_type
        type: u4
        doc: SDI object type
      
      - id: sdi_id
        type: u8
        doc: Object ID
      
      - id: compressed_len
        type: u4
        doc: Compressed length
      
      - id: uncompressed_len
        type: u4
        doc: Uncompressed length
      
      - id: next_page_no
        type: u4
        doc: Next page (0xFFFFFFFF = last)

