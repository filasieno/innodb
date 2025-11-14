# InnoDB Doublewrite Buffer file (#ib_*_*.dblwr) (MySQL 8.0.20+)
# Reference: MySQL 8.0 Source (storage/innobase/include/buf0dblwr.h)
meta:
  id: innodb_doublewrite
  title: InnoDB Doublewrite Buffer Container
  application: MySQL InnoDB Storage Engine
  file-extension:
    - dblwr
  license: MIT
  endian: le
  imports:
    - ../innodb_common

params:
  - id: page_size
    type: u4
    doc: Page size

seq:
  - id: dblwr_header
    type: dblwr_header_t
    doc: Doublewrite buffer file header
  
  - id: pages
    type: dblwr_page_t
    repeat: eos
    doc: Array of doublewrite pages

types:
  # ============================================================================
  # DOUBLEWRITE HEADER
  # ============================================================================
  dblwr_header_t:
    doc: |
      Doublewrite buffer file header.
    seq:
      - id: magic
        contents: [0x44, 0x42, 0x4C, 0x57]  # "DBLW"
        doc: Magic number identifying doublewrite buffer file
      
      - id: version
        type: u4
        doc: Doublewrite buffer format version
      
      - id: page_size
        type: u4
        doc: Page size for this doublewrite buffer
      
      - id: max_pages
        type: u4
        doc: Maximum number of pages in doublewrite buffer
      
      - id: reserved
        size: _root.page_size - 16
        doc: Reserved space (padding to one page)

  # ============================================================================
  # DOUBLEWRITE PAGE
  # ============================================================================
  dblwr_page_t:
    doc: |
      A page copy in the doublewrite buffer.
      Contains a full page image for recovery purposes.
    seq:
      - id: page_copy
        size: _root.page_size
        doc: |
          Complete page copy. Can be parsed as any InnoDB page type
          by reading the FIL header.
    
    instances:
      fil_header:
        pos: 0
        type: innodb_common::fil_header_t
        doc: FIL header of the copied page
      
      is_valid:
        value: fil_header.page_no != 0xFFFFFFFF
        doc: True if this doublewrite slot contains a valid page

