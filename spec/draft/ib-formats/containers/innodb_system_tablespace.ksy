# InnoDB System Tablespace (ibdata*) container
# Reference: MySQL 8.0 Source (storage/innobase/include/srv0srv.h, trx0sys.h)
meta:
  id: innodb_system_tablespace
  title: InnoDB System Tablespace Container
  application: MySQL InnoDB Storage Engine
  file-extension:
    - ibdata1
  license: MIT
  endian: le
  imports:
    - innodb_tablespace

params:
  - id: page_size
    type: u4
    doc: Page size (default 16KB)

seq:
  - id: pages
    type: innodb_tablespace::page_wrapper_t
    repeat: eos
    doc: Pages in system tablespace

instances:
  fsp_header:
    value: pages[0]
    doc: FSP header (page 0)
  ibuf_header:
    value: pages[1]
    doc: Insert buffer header (page 1)
  ibuf_bitmap:
    value: pages[2]
    doc: Insert buffer bitmap (page 2)
  trx_sys:
    value: pages[5]
    doc: Transaction system header (page 5)
  first_rseg:
    value: pages[6]
    doc: First rollback segment page (page 6)