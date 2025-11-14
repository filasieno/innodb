# $schema=../../../schema/kaitai.schema.json
meta:
  id: innodb_page_rtree
  title: InnoDB R-tree Spatial Index Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - ../innodb_common
    - innodb_page_index

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: Standard FIL header (38 bytes)
  
  - id: index_header
    type: innodb_page_index::index_header_t
    doc: Index page header (same as B-tree)
  
  - id: rtree_header
    type: rtree_header_t
    doc: R-tree specific header
  
  - id: mbr_data
    size-eos: true
    doc: |
      Minimum Bounding Rectangle (MBR) data and records.
      Each record contains MBR coordinates and child pointer.
  
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: Standard FIL trailer (8 bytes)

types:
  rtree_header_t:
    doc: |
      R-tree specific header information.
    seq:
      - id: mbr_count
        type: u2
        doc: Number of MBRs in this node
      
      - id: level
        type: u2
        doc: Level in R-tree (0 = leaf)
      
      - id: reserved
        size: 4
        doc: Reserved for future use

  mbr_t:
    doc: |
      Minimum Bounding Rectangle coordinates.
      Format: xmin, xmax, ymin, ymax
    seq:
      - id: xmin
        type: f8
        doc: Minimum X coordinate
      
      - id: xmax
        type: f8
        doc: Maximum X coordinate
      
      - id: ymin
        type: f8
        doc: Minimum Y coordinate
      
      - id: ymax
        type: f8
        doc: Maximum Y coordinate

