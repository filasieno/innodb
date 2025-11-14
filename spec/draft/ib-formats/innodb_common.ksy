# InnoDB Common Structures
# Common structures shared across all InnoDB file formats.
# Reference: MySQL 8.0.30+ Source Code (storage/innobase/include/)
#
# This file defines fundamental structures used throughout InnoDB's storage system.
# These structures are shared across all InnoDB file formats including tablespaces,
# system files, undo logs, and blob storage. The structures handle page-level metadata,
# compression algorithms, and type definitions that form the backbone of InnoDB's
# physical storage layer.
meta:
  id: innodb_common
  title: InnoDB Common Structures
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le

types:
  fil_header_t:
    doc: |
      File page header (FIL header) - appears at the beginning of every InnoDB page.
      This 38-byte header contains essential metadata for page identification, integrity,
      and recovery. It's crucial for InnoDB's crash recovery mechanism and page validation.
      Reference: fil0fil.h (struct fil_page_header_t)
    seq:
      - id: checksum
        type: u4
        doc: |
          Page checksum for data integrity validation. Uses CRC32 algorithm by default
          in MySQL 8.0+. Protects against silent data corruption by verifying page
          contents match expected values. Calculated over entire page except this field.
      - id: page_no
        type: u4
        doc: |
          Page number within the tablespace. Zero-based offset from start of file.
          Combined with space_id, uniquely identifies any page in the database.
          Critical for page addressing and extent management.
      - id: prev_page_lsn
        type: u8
        doc: |
          Log Sequence Number (LSN) of the previous page modification. Used for
          crash recovery to determine which pages need redo application. Part of
          InnoDB's Write-Ahead Logging (WAL) mechanism ensuring ACID properties.
      - id: page_type
        type: u2
        enum: page_type_enum
        doc: |
          Type of page content. Determines how the page data should be interpreted.
          Essential for proper parsing of page contents and understanding page purpose
          within InnoDB's storage hierarchy.
      - id: flush_lsn
        type: u8
        if: page_no == 0
        doc: |
          LSN of the last page flush operation. Only present on page 0 (first page)
          of each tablespace. Used by recovery process to determine which pages
          have been durably written to disk.
      - id: space_id
        type: u4
        doc: |
          Tablespace ID. Identifies which tablespace this page belongs to.
          Combined with page_no, provides complete page addressing across
          the entire database instance.
  
  fil_trailer_t:
    doc: |
      File page trailer (FIL trailer) - appears at the end of every InnoDB page.
      This 8-byte trailer provides additional integrity checking and recovery information.
      Works in conjunction with the FIL header to ensure page consistency.
      Reference: fil0fil.h (struct fil_page_trailer_t)
    seq:
      - id: old_checksum
        type: u4
        doc: |
          Legacy checksum field. In older MySQL versions, contained InnoDB checksum.
          In MySQL 8.0+, this field is typically unused but maintained for compatibility.
          Part of the historical checksum validation system.
      - id: lsn_low32
        type: u4
        doc: |
          Lower 32 bits of the page's Log Sequence Number. Provides additional
          verification of page state during recovery. Must match the LSN information
          in the page header and redo logs for page to be considered consistent.
  
  mach_compressed_uint_t:
    doc: |
      Machine-compressed unsigned integer - variable-length encoding used throughout InnoDB.
      Optimizes storage by using fewer bytes for smaller values. First byte indicates
      length and provides most significant bits. Critical for efficient storage of
      variable-sized data like record counts, offsets, and identifiers.
      Reference: mach0data.h (mach_read_compressed_uint)
    seq:
      - id: first_byte
        type: u1
        doc: |
          First byte containing length encoding and high bits. Values < 0x80 indicate
          single-byte encoding. Higher values indicate multi-byte sequences with
          decreasing bit significance.
      - id: second_byte
        type: u1
        if: first_byte >= 0x80
        doc: Second byte for 2-5 byte encodings, containing next 8 bits of the value.
      - id: third_byte
        type: u1
        if: first_byte >= 0xC0
        doc: Third byte for 3-5 byte encodings, containing next 8 bits of the value.
      - id: fourth_byte
        type: u1
        if: first_byte >= 0xE0
        doc: Fourth byte for 4-5 byte encodings, containing next 8 bits of the value.
      - id: fifth_byte
        type: u1
        if: first_byte >= 0xF0
        doc: Fifth byte for 5-byte encoding, containing least significant 8 bits.
    instances:
      value:
        doc: |
          Decoded unsigned integer value. Automatically reconstructs the full value
          from the compressed byte sequence using bit manipulation based on the
          length encoding in the first byte.
        value: |
          first_byte < 0x80 ? first_byte :
          first_byte < 0xC0 ? ((first_byte & 0x3F) << 8) | second_byte :
          first_byte < 0xE0 ? ((first_byte & 0x1F) << 16) | (second_byte << 8) | third_byte :
          first_byte < 0xF0 ? ((first_byte & 0x0F) << 24) | (second_byte << 16) | (third_byte << 8) | fourth_byte :
          (second_byte.as<u8> << 24) | (third_byte.as<u8> << 16) | (fourth_byte.as<u8> << 8) | fifth_byte
  
  space_flags_t:
    doc: |
      Tablespace flags - bitfield describing tablespace characteristics and capabilities.
      These flags control how InnoDB interprets and manages the tablespace, including
      compression, encryption, and special purpose designations. Stored in the
      tablespace header and used throughout the storage engine.
      Reference: fsp0types.h (fsp_flags_t)
    seq:
      - id: flags_value
        type: u4
        doc: |
          Raw 32-bit flags value containing all tablespace configuration options
          as bit flags. Individual flags are extracted via bit manipulation.
    instances:
      post_antelope:
        doc: |
          Post-Antelope file format flag. Indicates tablespace uses the newer
          file format (MySQL 5.1+) with improved BLOB handling and row formats.
          Essential for determining available features and compatibility.
        value: (flags_value & 0x1) != 0
      zip_ssize:
        doc: |
          Compressed page size shift value. For compressed tablespaces, indicates
          the compression factor as a power of 2. 0 = uncompressed, 1 = 2KB pages,
          2 = 4KB pages, etc. Affects storage efficiency and I/O patterns.
        value: (flags_value >> 1) & 0xF
      atomic_blobs:
        doc: |
          Atomic BLOB operations flag. When set, BLOB operations are guaranteed
          to be atomic - either fully committed or fully rolled back. Improves
          data consistency for large object storage.
        value: (flags_value >> 5) & 0x1
      page_ssize:
        doc: |
          Page size shift value for uncompressed tablespaces. Similar to zip_ssize
          but for regular (uncompressed) pages. Determines base page size used
          in the tablespace.
        value: (flags_value >> 6) & 0xF
      data_dir:
        doc: |
          Data directory flag. Indicates tablespace is stored in a data directory
          rather than the default location. Affects file path resolution during
          startup and recovery.
        value: (flags_value >> 10) & 0x1
      shared:
        doc: |
          Shared tablespace flag. Indicates this is a shared system tablespace
          (like ibdata1) rather than a file-per-table tablespace. Affects space
          management and extent allocation strategies.
        value: (flags_value >> 11) & 0x1
      temporary:
        doc: |
          Temporary tablespace flag. Indicates tablespace contains temporary
          data that can be discarded on restart. Used for internal temporary
          tables, sort operations, and other transient data storage.
        value: (flags_value >> 12) & 0x1
      encryption:
        doc: |
          Encryption flag. Indicates tablespace data is encrypted at rest.
          When set, all page data is encrypted using the configured encryption
          key, protecting sensitive data from unauthorized access.
        value: (flags_value >> 13) & 0x1
      sdi:
        doc: |
          Serialized Dictionary Information flag. Indicates tablespace contains
          SDI (Serialized Dictionary Information) pages with metadata about
          database objects. Used for atomic DDL operations and data dictionary
          management in MySQL 8.0+.
        value: (flags_value >> 14) & 0x1

enums:
  page_type_enum:
    doc: |
      Enumeration of all possible InnoDB page types. Each page type serves a specific
      purpose in the storage hierarchy and determines how the page data is interpreted.
      Page types are stored in the FIL header and are critical for proper page parsing
      and understanding the page's role in the database.

      Page Type Values:
      - 0 (allocated): Freshly allocated page that has not yet been initialized. Contains no meaningful
        data and is available for assignment to any page type. Used during extent allocation when new space is needed.
      - 2 (undo_log): Undo log page containing transaction undo records for MVCC and rollback.
        Stores before-images of modified data to support transaction isolation levels and crash recovery.
        Critical for ACID compliance and multi-version concurrency.
      - 3 (inode): Index node page containing file space inode information. Tracks FSEG (file segment)
        allocation and deallocation within the tablespace. Essential for space management and maintaining free space lists.
      - 4 (ibuf_free_list): Insert buffer free list page. Manages free space tracking for the insert buffer,
        which optimizes secondary index insertions by buffering them in memory before batch writing to disk.
      - 5 (ibuf_bitmap): Insert buffer bitmap page. Contains bitmaps tracking which pages have buffered
        operations pending. Used by the insert buffer to determine when pages need merging or flushing.
      - 6 (sys): Generic system page. Used for various system-level data that doesn't fit other specific
        page type categories. Contains system metadata and configuration.
      - 7 (trx_sys): Transaction system header page. Always located at page 5 of the system tablespace.
        Contains global transaction system state, rollback segment information, and doublewrite buffer metadata.
        Critical for transaction management.
      - 8 (fsp_hdr): File space header page. First page (page 0) of every tablespace containing space
        management metadata, extent allocation information, and space flags. Foundation of tablespace organization.
      - 9 (xdes): Extent descriptor page. Contains XDES (extent descriptor) entries tracking which pages
        within extents are allocated or free. Essential for space allocation and deallocation at the extent level.
      - 10 (blob): Uncompressed BLOB (Binary Large Object) data page. Stores large binary data that doesn't
        fit in regular record pages. Used for TEXT, BLOB, and other large object columns.
      - 11 (zlob_first): Compressed BLOB first page (legacy format). Start of a compressed large object
        storage chain. Contains compression metadata and the beginning of compressed data.
      - 12 (zlob_data): Compressed BLOB data continuation page (legacy format). Continuation of compressed
        large object data. Part of the compressed BLOB storage chain.
      - 13 (zlob_index): Compressed BLOB index page (legacy format). Contains indexing information for
        compressed large object fragments. Used to locate specific portions of compressed data.
      - 14 (zblob): Compressed BLOB first page (current format). Modern compressed large object storage
        format with improved compression and fragmentation handling.
      - 15 (zblob2): Compressed BLOB continuation page (current format). Continuation pages for modern
        compressed BLOB storage chains.
      - 16 (unknown): Unknown or unrecognized page type. Used for forward compatibility when new page
        types are introduced that older code doesn't understand.
      - 17 (index): B-tree index page. Contains the actual index data structure with keys and pointers.
        The core of InnoDB's indexing system supporting fast data access.
      - 18 (sdi_blob): Serialized Dictionary Information BLOB page. Contains metadata about database objects
        stored as compressed JSON. Used for atomic DDL operations in MySQL 8.0+.
      - 19 (sdi_zblob): Compressed Serialized Dictionary Information BLOB page. Compressed version of SDI
        data for efficient storage of large metadata structures.
      - 20 (lob_index): Large Object index page. Contains indexing information for large object storage,
        allowing efficient access to specific portions of large data.
      - 21 (lob_data): Large Object data page. Stores actual large object data with optimized access
        patterns for large binary content.
      - 22 (lob_first): Large Object first page. Beginning of a large object storage chain, containing
        metadata and the start of the object data.
      - 23 (zlob_first_v2): Compressed Large Object first page (version 2). Enhanced compressed LOB format
        with improved storage efficiency and access patterns.
      - 24 (zlob_data_v2): Compressed Large Object data page (version 2). Continuation pages for enhanced
        compressed large object storage.
      - 25 (zlob_index_v2): Compressed Large Object index page (version 2). Advanced indexing for compressed
        large objects with better fragmentation handling.
      - 26 (zlob_frag): Compressed Large Object fragment page. Stores fragmented portions of compressed
        large objects, optimizing storage for variable-sized content.
      - 27 (zlob_frag_entry): Compressed Large Object fragment entry page. Contains metadata about compressed
        object fragments, enabling efficient reconstruction.
      - 28 (rtree): R-tree spatial index page. Specialized index structure for spatial data types, enabling
        efficient geometric queries and spatial operations.

      Reference: fil0types.h (enum fil_page_type)
    0: allocated
    2: undo_log
    3: inode
    4: ibuf_free_list
    5: ibuf_bitmap
    6: sys
    7: trx_sys
    8: fsp_hdr
    9: xdes
    10: blob
    11: zlob_first
    12: zlob_data
    13: zlob_index
    14: zblob
    15: zblob2
    16: unknown
    17: index
    18: sdi_blob
    19: sdi_zblob
    20: lob_index
    21: lob_data
    22: lob_first
    23: zlob_first_v2
    24: zlob_data_v2
    25: zlob_index_v2
    26: zlob_frag
    27: zlob_frag_entry
    28: rtree
  
  checksum_algorithm_enum:
    doc: |
      Checksum algorithms used for page integrity verification. Different algorithms
      provide varying levels of protection against data corruption. The choice affects
      performance and compatibility across MySQL versions.

      Checksum Algorithm Values:
      - 0 (crc32): CRC32 checksum algorithm. Fast and reliable, provides good protection against
        accidental data corruption. Default in MySQL 8.0+ for new installations.
      - 1 (innodb): Legacy InnoDB checksum algorithm. Simple polynomial-based checksum used
        in older MySQL versions. Less robust than CRC32 but maintains compatibility.
      - 2 (none): No checksum validation. Disables integrity checking entirely. Only used
        in development or when checksums are handled externally. Not recommended for production use.
      - 3 (strict_crc32): Strict CRC32 validation. Like CRC32 but treats any checksum mismatch as
        a critical error rather than attempting recovery. Used when data integrity is paramount.
      - 4 (strict_innodb): Strict legacy InnoDB validation. Like the legacy algorithm but with strict
        error handling. Maintains backward compatibility while enforcing integrity.
      - 5 (strict_none): Strict no-checksum mode. Like 'none' but explicitly disables all validation.
        Used for testing or when external integrity mechanisms are in place.

      Reference: buf0checksum.h (enum buf_checksum_algorithm)
    0: crc32
    1: innodb
    2: none
    3: strict_crc32
    4: strict_innodb
    5: strict_none

  row_format_enum:
    doc: |
      InnoDB row formats determining how table rows are physically stored.
      Each format provides different trade-offs between storage efficiency,
      compatibility, and feature availability. The format affects how variable-length
      columns and overflow data are handled.

      Row Format Values:
      - 0 (redundant): Redundant row format (legacy). Original InnoDB format with fixed row structure.
        Stores first 768 bytes of variable-length columns in the record, remainder as overflow.
        Limited to 8KB page size and older feature set.
      - 1 (compact): Compact row format (MySQL 5.0+). Improved storage efficiency by storing
        variable-length columns more compactly. Better use of page space and reduced I/O
        for variable-length data. Default format for many years.
      - 2 (dynamic): Dynamic row format (MySQL 5.5+). Further optimization for variable-length
        data storage. Only stores 20-byte pointer in main record for off-page data,
        improving space utilization. Better for tables with large variable columns.
      - 3 (compressed): Compressed row format. Combines dynamic format with page-level compression.
        Provides both storage space savings and reduced I/O through compression.
        Requires compressed tablespace and adds CPU overhead for compression/decompression.

      Reference: row0row.h (enum row_format_t)
    0: redundant
    1: compact
    2: dynamic
    3: compressed
