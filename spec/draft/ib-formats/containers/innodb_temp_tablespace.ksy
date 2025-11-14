# InnoDB Temporary Tablespace file (ibtmp*) (MySQL 8.0.20+)
# Reference: MySQL 8.0 Source (storage/innobase/include/fil0fil.h)
meta:
  id: innodb_temp_tablespace
  title: InnoDB Temporary Tablespace Container
  application: MySQL InnoDB Storage Engine
  file-extension:
    - ibtmp1
  license: MIT
  endian: le
  imports:
    - innodb_tablespace

params:
  - id: page_size
    type: u4
    doc: Page size

seq:
  - id: pages
    type: innodb_tablespace::page_wrapper_t
    repeat: eos
    doc: |
      Pages in temporary tablespace.
      Uses same page structure as regular tablespace.

instances:
  fsp_header:
    value: pages[0]
    doc: FSP header (page 0)
  
  is_temporary:
    value: true
    doc: Flag indicating this is a temporary tablespace

