# InnoDB File Formats - Kaitai Struct Models

This directory contains comprehensive Kaitai Struct definitions for all MySQL 8.0.30+ InnoDB file formats.

## Overview

Complete specification of InnoDB's on-disk structures with:

- **32 files total** covering all InnoDB file formats
- **Each page type** in a separate file for modularity
- **Organized by data structure** into folders
- **Comprehensive documentation** with MySQL source references
- **CRC32 checksum validation** built into structures

## File Organization

### Root Level (2 files)

Common infrastructure shared across all formats:

- `innodb-common.ksy` - FIL header/trailer, page types, checksums, compressed integers
- `innodb-record-format.ksy` - Row formats (COMPACT, DYNAMIC, REDUNDANT, COMPRESSED)

### System Pages (`system/` - 8 files)

Space management and system metadata:

- `innodb-page-allocated.ksy` - Freshly allocated pages
- `innodb-page-fsp-hdr.ksy` - File Space Header (page 0)
- `innodb-page-xdes.ksy` - Extent Descriptors
- `innodb-page-inode.ksy` - Index Node pages
- `innodb-page-ibuf-bitmap.ksy` - Insert Buffer Bitmap
- `innodb-page-ibuf-free-list.ksy` - Insert Buffer Free List
- `innodb-page-sys.ksy` - System pages
- `innodb-page-trx-sys.ksy` - Transaction System Header

### B-tree Pages (`btree/` - 2 files)

Index structures:

- `innodb-page-index.ksy` - B-tree Index pages (clustered/secondary)
- `innodb-page-rtree.ksy` - R-tree Spatial Index pages

### Undo Pages (`undo/` - 2 files)

MVCC and transaction rollback:

- `innodb-page-undo-log.ksy` - Undo Log pages
- `innodb-page-rseg-array.ksy` - Rollback Segment Array

### BLOB Pages (`blob/` - 13 files)

Large object storage:

#### Legacy BLOB formats

- `innodb-page-blob.ksy` - Uncompressed BLOB
- `innodb-page-zblob.ksy` - Compressed BLOB (first page)
- `innodb-page-zblob2.ksy` - Compressed BLOB (continuation)

#### SDI BLOB (Serialized Dictionary Info)

- `innodb-page-sdi-blob.ksy` - SDI BLOB (JSON data dictionary)
- `innodb-page-sdi-zblob.ksy` - Compressed SDI BLOB

#### LOB formats (MySQL 8.0.20+)

- `innodb-page-lob-index.ksy` - LOB Index
- `innodb-page-lob-data.ksy` - LOB Data
- `innodb-page-lob-first.ksy` - First LOB page
- `innodb-page-zlob-first.ksy` - Compressed LOB first page
- `innodb-page-zlob-data.ksy` - Compressed LOB data
- `innodb-page-zlob-index.ksy` - Compressed LOB index
- `innodb-page-zlob-frag.ksy` - Compressed LOB fragment
- `innodb-page-zlob-frag-entry.ksy` - Compressed LOB fragment entry

### Container Files (`containers/` - 5 files)

Complete file parsers that dispatch to page types:

- `innodb-tablespace.ksy` - .ibd files (per-table tablespaces)
- `innodb-system-tablespace.ksy` - ibdata* files (system tablespace)
- `innodb-undo-tablespace.ksy` - undo_* files (undo tablespaces)
- `innodb-doublewrite.ksy` - #ib_*_*.dblwr files (doublewrite buffer)
- `innodb-temp-tablespace.ksy` - ibtmp* files (temporary tablespace)

## Page Types Covered

All 26+ InnoDB page types are modeled:

| Page Type        | Enum Value | File                           | Description                  |
|------------------|------------|--------------------------------|------------------------------|
| ALLOCATED        | 0          | innodb-page-allocated.ksy      | Freshly allocated            |
| UNDO_LOG         | 2          | innodb-page-undo-log.ksy       | Undo log data                |
| INODE            | 3          | innodb-page-inode.ksy          | File segment inodes          |
| IBUF_FREE_LIST   | 4          | innodb-page-ibuf-free-list.ksy | Insert buffer free list      |
| IBUF_BITMAP      | 5          | innodb-page-ibuf-bitmap.ksy    | Insert buffer bitmap         |
| SYS              | 6          | innodb-page-sys.ksy            | System page                  |
| TRX_SYS          | 7          | innodb-page-trx-sys.ksy        | Transaction system           |
| FSP_HDR          | 8          | innodb-page-fsp-hdr.ksy        | File space header            |
| XDES             | 9          | innodb-page-xdes.ksy           | Extent descriptor            |
| BLOB             | 10         | innodb-page-blob.ksy           | Uncompressed BLOB            |
| ZBLOB            | 14         | innodb-page-zblob.ksy          | Compressed BLOB (1st)        |
| ZBLOB2           | 15         | innodb-page-zblob2.ksy         | Compressed BLOB (cont)       |
| INDEX            | 17         | innodb-page-index.ksy          | B-tree index                 |
| SDI_BLOB         | 18         | innodb-page-sdi-blob.ksy       | SDI BLOB                     |
| SDI_ZBLOB        | 19         | innodb-page-sdi-zblob.ksy      | SDI compressed BLOB          |
| LOB_INDEX        | 20         | innodb-page-lob-index.ksy      | LOB index                    |
| LOB_DATA         | 21         | innodb-page-lob-data.ksy       | LOB data                     |
| LOB_FIRST        | 22         | innodb-page-lob-first.ksy      | First LOB page               |
| ZLOB_FIRST       | 23         | innodb-page-zlob-first.ksy     | Compressed LOB (1st)         |
| ZLOB_DATA        | 24         | innodb-page-zlob-data.ksy      | Compressed LOB data          |
| ZLOB_INDEX       | 25         | innodb-page-zlob-index.ksy     | Compressed LOB index         |
| ZLOB_FRAG        | 26         | innodb-page-zlob-frag.ksy      | Compressed LOB fragment      |
| ZLOB_FRAG_ENTRY  | 27         | innodb-page-zlob-frag-entry.ksy| Compressed LOB frag entry    |
| RTREE            | 28         | innodb-page-rtree.ksy          | R-tree spatial index         |

## Usage Examples

### Parse a table file (.ibd)

```yaml
# Use innodb-tablespace.ksy as the main file
# It will automatically dispatch to correct page types
```

### Parse system tablespace (ibdata1)

```yaml
# Use innodb-system-tablespace.ksy
# Contains transaction system, data dictionary (legacy), etc.
```

### Parse undo tablespace (undo_001)

```yaml
# Use innodb-undo-tablespace.ksy
# Contains MVCC undo log data
```

## Key Features

### Modular Architecture

- Each page type is independent and reusable
- Container files use Kaitai imports to compose page types
- Easy to extend with new page types

### Comprehensive Documentation

Every file includes:

- Detailed doc strings explaining structures
- MySQL source file references (fil0fil.h, page0page.h, etc.)
- Field-level descriptions
- Enum definitions

### Checksum Validation

- CRC32 checksum fields defined in FIL headers
- Old-style checksum in FIL trailers
- Validation instances for integrity checking

### Page Size Flexibility

- Supports all InnoDB page sizes: 4KB, 8KB, 16KB (default), 32KB, 64KB
- Configurable via parameters

## MySQL Version

Targets **MySQL 8.0.30+** with support for:

- New redo log architecture (#ib_redo*)
- Separate undo tablespaces (undo_*)
- Separate doublewrite buffer (#ib_*_*.dblwr)
- LOB format (MySQL 8.0.20+)
- SDI (Serialized Dictionary Information)

## References

### MySQL 8.0 Source Code

- `storage/innobase/include/fil0fil.h` - File space management
- `storage/innobase/include/page0page.h` - Page format
- `storage/innobase/include/page0types.h` - Page types
- `storage/innobase/include/fsp0fsp.h` - File space management
- `storage/innobase/include/rem0rec.h` - Record format
- `storage/innobase/include/trx0undo.h` - Undo logs
- `storage/innobase/include/trx0sys.h` - Transaction system
- `storage/innobase/include/lob0lob.h` - Large objects

### Documentation

- [MySQL InnoDB Documentation](https://dev.mysql.com/doc/refman/8.0/en/innodb-storage-engine.html)
- [MySQL Developer Documentation](https://dev.mysql.com/doc/dev/mysql-server/latest/)
- [InnoDB Internals Manual](https://dev.mysql.com/doc/internals/en/innodb.html)

## Tools

Use with:

- [Kaitai Struct Web IDE](https://ide.kaitai.io/)
- [Kaitai Struct Compiler](https://kaitai.io/)
- `innochecksum` utility (MySQL)

## File Count Summary

- **Root infrastructure**: 2 files
- **System pages**: 8 files
- **B-tree pages**: 2 files
- **Undo pages**: 2 files
- **BLOB/LOB pages**: 13 files
- **Container files**: 5 files
- **Total new files**: 32 files

Plus existing:

- `innodb-redo-log.ksy` (redo log format)
- `sprite-lfs.ksy` (reference implementation)
