# InnoDB Record Formats
# InnoDB row formats: REDUNDANT, COMPACT, DYNAMIC, COMPRESSED
# Reference: MySQL 8.0 Source Code (storage/innobase/include/rem0rec.h)
meta:
  id: innodb_record_format
  title: InnoDB Record Formats
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  imports:
    - innodb_common

types:
  compact_record_header_t:
    seq:
      - id: info_flags
        type: u1
      - id: n_owned
        type: u1
      - id: heap_no
        type: u2
      - id: record_type
        type: u1
        enum: record_type_enum
      - id: next_record_offset
        type: s2
    instances:
      is_deleted:
        value: (info_flags & 0x08) != 0
      is_min_rec:
        value: (info_flags & 0x10) != 0
  
  blob_reference_t:
    seq:
      - id: space_id
        type: u4
      - id: page_no
        type: u4
      - id: offset
        type: u4
      - id: blob_length
        type: u8

enums:
  record_type_enum:
    0: conventional
    1: node_pointer
    2: infimum
    3: supremum
  
  field_type_enum:
    1: varchar
    2: char
    3: binary
    4: varbinary
    5: blob
    252: blob_type
