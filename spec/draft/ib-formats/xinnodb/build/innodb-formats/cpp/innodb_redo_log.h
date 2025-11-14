#ifndef INNODB_REDO_LOG_H_
#define INNODB_REDO_LOG_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_redo_log_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include <set>
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_redo_log_t : public kaitai::kstruct {

public:
    class checkpoint_block_t_t;
    class checkpoint_record_t_t;
    class compressed_uint_t_t;
    class file_header_t_t;
    class generic_record_data_t_t;
    class log_block_header_t_t;
    class log_block_t_t;
    class log_block_trailer_t_t;
    class log_record_t_t;
    class log_records_t_t;
    class page_create_t_t;
    class rec_delete_mark_t_t;
    class rec_insert_t_t;
    class rec_update_t_t;
    class undo_erase_t_t;
    class undo_insert_t_t;
    class update_field_t_t;
    class write_1byte_t_t;
    class write_2bytes_t_t;
    class write_4bytes_t_t;
    class write_8bytes_t_t;
    class write_string_t_t;

    enum mlog_type_t {
        MLOG_TYPE_MLOG_1BYTE = 1,
        MLOG_TYPE_MLOG_2BYTES = 2,
        MLOG_TYPE_MLOG_4BYTES = 4,
        MLOG_TYPE_MLOG_8BYTES = 8,
        MLOG_TYPE_MLOG_REC_INSERT = 9,
        MLOG_TYPE_MLOG_REC_CLUST_DELETE_MARK = 10,
        MLOG_TYPE_MLOG_REC_SEC_DELETE_MARK = 11,
        MLOG_TYPE_MLOG_REC_UPDATE_IN_PLACE = 13,
        MLOG_TYPE_MLOG_REC_DELETE = 14,
        MLOG_TYPE_MLOG_LIST_END_DELETE = 15,
        MLOG_TYPE_MLOG_LIST_START_DELETE = 16,
        MLOG_TYPE_MLOG_LIST_END_COPY_CREATED = 17,
        MLOG_TYPE_MLOG_PAGE_REORGANIZE = 18,
        MLOG_TYPE_MLOG_PAGE_CREATE = 19,
        MLOG_TYPE_MLOG_UNDO_INSERT = 20,
        MLOG_TYPE_MLOG_UNDO_ERASE_END = 21,
        MLOG_TYPE_MLOG_UNDO_INIT = 22,
        MLOG_TYPE_MLOG_UNDO_HDR_REUSE = 23,
        MLOG_TYPE_MLOG_UNDO_HDR_CREATE = 24,
        MLOG_TYPE_MLOG_REC_MIN_MARK = 25,
        MLOG_TYPE_MLOG_IBUF_BITMAP_INIT = 26,
        MLOG_TYPE_MLOG_INIT_FILE_PAGE = 27,
        MLOG_TYPE_MLOG_WRITE_STRING = 30,
        MLOG_TYPE_MLOG_MULTI_REC_END = 31,
        MLOG_TYPE_MLOG_CHECKPOINT = 32,
        MLOG_TYPE_MLOG_PAGE_CREATE_COMPRESSED = 34,
        MLOG_TYPE_MLOG_PAGE_CREATE_RTREE = 36,
        MLOG_TYPE_MLOG_COMP_REC_MIN_MARK = 37,
        MLOG_TYPE_MLOG_COMP_PAGE_CREATE = 38,
        MLOG_TYPE_MLOG_COMP_REC_INSERT = 39,
        MLOG_TYPE_MLOG_COMP_REC_CLUST_DELETE_MARK = 40,
        MLOG_TYPE_MLOG_COMP_REC_SEC_DELETE_MARK = 41,
        MLOG_TYPE_MLOG_COMP_REC_UPDATE_IN_PLACE = 42,
        MLOG_TYPE_MLOG_COMP_REC_DELETE = 43,
        MLOG_TYPE_MLOG_COMP_LIST_END_DELETE = 44,
        MLOG_TYPE_MLOG_COMP_LIST_START_DELETE = 45,
        MLOG_TYPE_MLOG_COMP_LIST_END_COPY_CREATED = 46,
        MLOG_TYPE_MLOG_COMP_PAGE_REORGANIZE = 47,
        MLOG_TYPE_MLOG_FILE_CREATE = 48,
        MLOG_TYPE_MLOG_FILE_RENAME = 49,
        MLOG_TYPE_MLOG_FILE_DELETE = 50,
        MLOG_TYPE_MLOG_FILE_CREATE2 = 51,
        MLOG_TYPE_MLOG_FILE_RENAME2 = 52,
        MLOG_TYPE_MLOG_TRUNCATE = 55,
        MLOG_TYPE_MLOG_INDEX_LOAD = 56,
        MLOG_TYPE_MLOG_TABLE_DYNAMIC_META = 57,
        MLOG_TYPE_MLOG_PAGE_INIT = 58,
        MLOG_TYPE_MLOG_ZIP_PAGE_COMPRESS = 59,
        MLOG_TYPE_MLOG_TEST = 60
    };
    static bool _is_defined_mlog_type_t(mlog_type_t v);

private:
    static const std::set<mlog_type_t> _values_mlog_type_t;
    static std::set<mlog_type_t> _build_values_mlog_type_t();

public:

    innodb_redo_log_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_redo_log_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_redo_log_t();

    /**
     * Checkpoint blocks store information about the consistent state of the
     * database at a point in time. Two checkpoint regions alternate - when
     * one is being written, the other contains the last valid checkpoint.
     * Each checkpoint region consists of two consecutive 512-byte blocks.
     */

    class checkpoint_block_t_t : public kaitai::kstruct {

    public:

        checkpoint_block_t_t(kaitai::kstream* p__io, innodb_redo_log_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~checkpoint_block_t_t();

    private:
        bool f_checkpoint_lsn;
        uint64_t m_checkpoint_lsn;

    public:

        /**
         * LSN up to which all changes have been flushed to disk
         */
        uint64_t checkpoint_lsn();

    private:
        bool f_checkpoint_no;
        uint64_t m_checkpoint_no;

    public:

        /**
         * Checkpoint sequence number (monotonically increasing)
         */
        uint64_t checkpoint_no();

    private:
        bool f_checkpoint_offset;
        uint64_t m_checkpoint_offset;

    public:

        /**
         * Byte offset within redo log files where checkpoint_lsn is located
         */
        uint64_t checkpoint_offset();

    private:
        log_block_t_t* m_block_1;
        log_block_t_t* m_block_2;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t* m__parent;

    public:

        /**
         * First block of checkpoint region
         */
        log_block_t_t* block_1() const { return m_block_1; }

        /**
         * Second block of checkpoint region
         */
        log_block_t_t* block_2() const { return m_block_2; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t* _parent() const { return m__parent; }
    };

    /**
     * Checkpoint record marking a consistent database state
     */

    class checkpoint_record_t_t : public kaitai::kstruct {

    public:

        checkpoint_record_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~checkpoint_record_t_t();

    private:
        uint64_t m_checkpoint_lsn;
        uint64_t m_checkpoint_no;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * LSN of this checkpoint
         */
        uint64_t checkpoint_lsn() const { return m_checkpoint_lsn; }

        /**
         * Checkpoint sequence number
         */
        uint64_t checkpoint_no() const { return m_checkpoint_no; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Variable-length compressed unsigned integer encoding used throughout
     * redo log to save space. Small values use fewer bytes:
     * - Values < 0x80: 1 byte
     * - Values < 0x4000: 2 bytes
     * - Values < 0x200000: 3 bytes
     * - Values < 0x10000000: 4 bytes
     * - Larger values: 5 bytes
     */

    class compressed_uint_t_t : public kaitai::kstruct {

    public:

        compressed_uint_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~compressed_uint_t_t();

    private:
        bool f_value;
        int32_t m_value;

    public:

        /**
         * Decompressed unsigned integer value
         */
        int32_t value();

    private:
        uint8_t m_first_byte;
        uint8_t m_second_byte;
        bool n_second_byte;

    public:
        bool _is_null_second_byte() { second_byte(); return n_second_byte; };

    private:
        uint8_t m_third_byte;
        bool n_third_byte;

    public:
        bool _is_null_third_byte() { third_byte(); return n_third_byte; };

    private:
        uint8_t m_fourth_byte;
        bool n_fourth_byte;

    public:
        bool _is_null_fourth_byte() { fourth_byte(); return n_fourth_byte; };

    private:
        uint8_t m_fifth_byte;
        bool n_fifth_byte;

    public:
        bool _is_null_fifth_byte() { fifth_byte(); return n_fifth_byte; };

    private:
        innodb_redo_log_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint8_t first_byte() const { return m_first_byte; }
        uint8_t second_byte() const { return m_second_byte; }
        uint8_t third_byte() const { return m_third_byte; }
        uint8_t fourth_byte() const { return m_fourth_byte; }
        uint8_t fifth_byte() const { return m_fifth_byte; }
        innodb_redo_log_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    /**
     * The file header occupies the first 512-byte block (block 0) of each
     * redo log file. It contains metadata about the log file including
     * format version, starting LSN, and file identification.
     */

    class file_header_t_t : public kaitai::kstruct {

    public:

        file_header_t_t(kaitai::kstream* p__io, innodb_redo_log_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~file_header_t_t();

    private:
        std::string m_magic;
        uint32_t m_format_version;
        uint64_t m_start_lsn;
        std::string m_creator_name;
        uint32_t m_log_flags;
        std::string m_log_uuid;
        uint32_t m_header_checksum;
        std::string m_reserved;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t* m__parent;

    public:

        /**
         * Magic number identifying InnoDB redo log
         */
        std::string magic() const { return m_magic; }

        /**
         * Redo log format version number. MySQL 8.0 uses format version 2.
         * Earlier versions used format 1 or 0.
         */
        uint32_t format_version() const { return m_format_version; }

        /**
         * Log Sequence Number (LSN) of the first log record in this file.
         * The LSN is a monotonically increasing value that uniquely identifies
         * a position in the redo log.
         */
        uint64_t start_lsn() const { return m_start_lsn; }

        /**
         * Name of MySQL version that created this log file
         */
        std::string creator_name() const { return m_creator_name; }

        /**
         * Flags indicating log file state. Bit 0x1 indicates log is being
         * created, 0x2 indicates crash recovery needed.
         */
        uint32_t log_flags() const { return m_log_flags; }

        /**
         * Unique identifier (UUID) for this redo log. Used to match redo logs
         * with data files and detect log file mismatches.
         */
        std::string log_uuid() const { return m_log_uuid; }

        /**
         * CRC32 checksum of file header fields
         */
        uint32_t header_checksum() const { return m_header_checksum; }

        /**
         * Reserved space padding to 512 bytes
         */
        std::string reserved() const { return m_reserved; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t* _parent() const { return m__parent; }
    };

    /**
     * Generic record data for unspecified or custom record types
     */

    class generic_record_data_t_t : public kaitai::kstruct {

    public:

        generic_record_data_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~generic_record_data_t_t();

    private:
        std::string m_data;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Raw record data (format depends on record type)
         */
        std::string data() const { return m_data; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * 12-byte header at the start of each log block. Contains metadata about
     * the block including its sequence number, data length, and pointers to
     * help locate log records within the block.
     */

    class log_block_header_t_t : public kaitai::kstruct {

    public:

        log_block_header_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_block_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~log_block_header_t_t();

    private:
        bool f_block_no_without_flush_bit;
        int32_t m_block_no_without_flush_bit;

    public:

        /**
         * Block number with flush bit masked out
         */
        int32_t block_no_without_flush_bit();

    private:
        bool f_is_flush_bit_set;
        bool m_is_flush_bit_set;

    public:

        /**
         * True if this block follows a flush operation
         */
        bool is_flush_bit_set();

    private:
        uint32_t m_hdr_no;
        uint16_t m_data_len;
        uint16_t m_first_rec_group;
        uint32_t m_checkpoint_no;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_block_t_t* m__parent;

    public:

        /**
         * Log block number. This is a sequential counter that wraps around.
         * The highest bit (0x80000000) indicates this is the first block
         * after a flush operation.
         */
        uint32_t hdr_no() const { return m_hdr_no; }

        /**
         * Number of bytes of log data written to this block (0-496).
         * When data_len < 496, remaining bytes in data section are undefined.
         */
        uint16_t data_len() const { return m_data_len; }

        /**
         * Offset (from start of data section) to the first log record group
         * that starts in this block. If 0, no new record group starts here.
         */
        uint16_t first_rec_group() const { return m_first_rec_group; }

        /**
         * Checkpoint number when this block was written. Used during recovery
         * to determine which checkpoint is more recent.
         */
        uint32_t checkpoint_no() const { return m_checkpoint_no; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_block_t_t* _parent() const { return m__parent; }
    };

    /**
     * Standard 512-byte redo log block. Each block contains a 12-byte header,
     * 496 bytes of log record data, and a 4-byte trailer with checksum.
     * Log blocks are numbered sequentially with a 32-bit block number that
     * wraps around after reaching the maximum value.
     */

    class log_block_t_t : public kaitai::kstruct {

    public:

        log_block_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~log_block_t_t();

    private:
        bool f_block_number;
        uint32_t m_block_number;

    public:

        /**
         * Sequential block number (from header)
         */
        uint32_t block_number();

    private:
        bool f_has_valid_data;
        bool m_has_valid_data;

    public:

        /**
         * Whether this block contains valid log data
         */
        bool has_valid_data();

    private:
        bool f_log_records;
        log_records_t_t* m_log_records;
        bool n_log_records;

    public:
        bool _is_null_log_records() { log_records(); return n_log_records; };

    private:

    public:

        /**
         * Parsed log records from data section
         */
        log_records_t_t* log_records();

    private:
        log_block_header_t_t* m_header;
        std::string m_data;
        log_block_trailer_t_t* m_trailer;
        innodb_redo_log_t* m__root;
        kaitai::kstruct* m__parent;
        std::string m__raw_log_records;
        bool n__raw_log_records;

    public:
        bool _is_null__raw_log_records() { _raw_log_records(); return n__raw_log_records; };

    private:
        kaitai::kstream* m__io__raw_log_records;

    public:

        /**
         * 12-byte block header
         */
        log_block_header_t_t* header() const { return m_header; }

        /**
         * Log record data section. Contains one or more redo log records.
         * The actual used length is specified in header.data_len.
         */
        std::string data() const { return m_data; }

        /**
         * 4-byte block trailer with checksum
         */
        log_block_trailer_t_t* trailer() const { return m_trailer; }
        innodb_redo_log_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
        std::string _raw_log_records() const { return m__raw_log_records; }
        kaitai::kstream* _io__raw_log_records() const { return m__io__raw_log_records; }
    };

    /**
     * 4-byte trailer containing checksum for block integrity verification
     */

    class log_block_trailer_t_t : public kaitai::kstruct {

    public:

        log_block_trailer_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_block_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~log_block_trailer_t_t();

    private:
        uint32_t m_checksum;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_block_t_t* m__parent;

    public:

        /**
         * CRC32 checksum of the entire log block (header + data).
         * Used to detect corruption in redo log blocks.
         */
        uint32_t checksum() const { return m_checksum; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_block_t_t* _parent() const { return m__parent; }
    };

    /**
     * Individual redo log record describing a single change to a page.
     * Each record has a type, space ID, page number, and type-specific data.
     * Records are variable length and tightly packed.
     */

    class log_record_t_t : public kaitai::kstruct {

    public:

        log_record_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_records_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~log_record_t_t();

    private:
        mlog_type_t m_type;
        compressed_uint_t_t* m_space_id;
        bool n_space_id;

    public:
        bool _is_null_space_id() { space_id(); return n_space_id; };

    private:
        compressed_uint_t_t* m_page_no;
        bool n_page_no;

    public:
        bool _is_null_page_no() { page_no(); return n_page_no; };

    private:
        kaitai::kstruct* m_record_data;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_records_t_t* m__parent;

    public:

        /**
         * Record type (MLOG_* constant). Determines the format of the record
         * data and what operation it represents.
         */
        mlog_type_t type() const { return m_type; }

        /**
         * Tablespace ID where the modification occurred. Compressed format
         * saves space for common small space IDs.
         */
        compressed_uint_t_t* space_id() const { return m_space_id; }

        /**
         * Page number within the tablespace being modified
         */
        compressed_uint_t_t* page_no() const { return m_page_no; }

        /**
         * Type-specific record data
         */
        kaitai::kstruct* record_data() const { return m_record_data; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_records_t_t* _parent() const { return m__parent; }
    };

    /**
     * Container for one or more redo log records within a block's data section.
     * Each record describes a single modification to a tablespace page.
     */

    class log_records_t_t : public kaitai::kstruct {

    public:

        log_records_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_block_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~log_records_t_t();

    private:
        std::vector<log_record_t_t*>* m_records;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_block_t_t* m__parent;

    public:

        /**
         * Sequence of redo log records
         */
        std::vector<log_record_t_t*>* records() const { return m_records; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_block_t_t* _parent() const { return m__parent; }
    };

    /**
     * Create a new page in the buffer pool
     */

    class page_create_t_t : public kaitai::kstruct {

    public:

        page_create_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~page_create_t_t();

    private:
        uint16_t m_page_type;
        uint64_t m_index_id;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Type of page being created:
         * 0 = uncompressed, 1 = compressed
         */
        uint16_t page_type() const { return m_page_type; }

        /**
         * Index ID if this is an index page
         */
        uint64_t index_id() const { return m_index_id; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Mark a clustered index record as deleted
     */

    class rec_delete_mark_t_t : public kaitai::kstruct {

    public:

        rec_delete_mark_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~rec_delete_mark_t_t();

    private:
        uint16_t m_offset;
        uint8_t m_flags;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset of record to mark
         */
        uint16_t offset() const { return m_offset; }

        /**
         * Delete mark flags (1 = mark deleted, 0 = unmark)
         */
        uint8_t flags() const { return m_flags; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Insert a record into a B-tree page
     */

    class rec_insert_t_t : public kaitai::kstruct {

    public:

        rec_insert_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~rec_insert_t_t();

    private:
        uint16_t m_offset;
        compressed_uint_t_t* m_rec_len;
        std::string m_record_data;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset where record is inserted
         */
        uint16_t offset() const { return m_offset; }

        /**
         * Length of record
         */
        compressed_uint_t_t* rec_len() const { return m_rec_len; }

        /**
         * Complete record data including header
         */
        std::string record_data() const { return m_record_data; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Update a record in place (without reorganizing page)
     */

    class rec_update_t_t : public kaitai::kstruct {

    public:

        rec_update_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~rec_update_t_t();

    private:
        uint16_t m_offset;
        compressed_uint_t_t* m_update_vector_len;
        std::vector<update_field_t_t*>* m_update_fields;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset of record being updated
         */
        uint16_t offset() const { return m_offset; }

        /**
         * Number of fields being updated
         */
        compressed_uint_t_t* update_vector_len() const { return m_update_vector_len; }

        /**
         * Array of field updates
         */
        std::vector<update_field_t_t*>* update_fields() const { return m_update_fields; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Erase end portion of an undo log page
     */

    class undo_erase_t_t : public kaitai::kstruct {

    public:

        undo_erase_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_erase_t_t();

    private:
        uint16_t m_offset;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset from which to erase to page end
         */
        uint16_t offset() const { return m_offset; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Insert a record into an undo log page
     */

    class undo_insert_t_t : public kaitai::kstruct {

    public:

        undo_insert_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_insert_t_t();

    private:
        uint16_t m_offset;
        compressed_uint_t_t* m_len;
        std::string m_data;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset where undo record is inserted
         */
        uint16_t offset() const { return m_offset; }

        /**
         * Length of undo record
         */
        compressed_uint_t_t* len() const { return m_len; }

        /**
         * Undo record data
         */
        std::string data() const { return m_data; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Single field update within a record update operation
     */

    class update_field_t_t : public kaitai::kstruct {

    public:

        update_field_t_t(kaitai::kstream* p__io, innodb_redo_log_t::rec_update_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~update_field_t_t();

    private:
        compressed_uint_t_t* m_field_no;
        compressed_uint_t_t* m_field_len;
        std::string m_field_data;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::rec_update_t_t* m__parent;

    public:

        /**
         * Field number being updated
         */
        compressed_uint_t_t* field_no() const { return m_field_no; }

        /**
         * New field length
         */
        compressed_uint_t_t* field_len() const { return m_field_len; }

        /**
         * New field data
         */
        std::string field_data() const { return m_field_data; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::rec_update_t_t* _parent() const { return m__parent; }
    };

    /**
     * Write 1 byte to a page at specified offset
     */

    class write_1byte_t_t : public kaitai::kstruct {

    public:

        write_1byte_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~write_1byte_t_t();

    private:
        uint16_t m_offset;
        uint8_t m_value;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset within page
         */
        uint16_t offset() const { return m_offset; }

        /**
         * Byte value to write
         */
        uint8_t value() const { return m_value; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Write 2 bytes to a page at specified offset
     */

    class write_2bytes_t_t : public kaitai::kstruct {

    public:

        write_2bytes_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~write_2bytes_t_t();

    private:
        uint16_t m_offset;
        uint16_t m_value;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset within page
         */
        uint16_t offset() const { return m_offset; }

        /**
         * 2-byte value to write
         */
        uint16_t value() const { return m_value; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Write 4 bytes to a page at specified offset
     */

    class write_4bytes_t_t : public kaitai::kstruct {

    public:

        write_4bytes_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~write_4bytes_t_t();

    private:
        uint16_t m_offset;
        uint32_t m_value;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset within page
         */
        uint16_t offset() const { return m_offset; }

        /**
         * 4-byte value to write
         */
        uint32_t value() const { return m_value; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Write 8 bytes to a page at specified offset
     */

    class write_8bytes_t_t : public kaitai::kstruct {

    public:

        write_8bytes_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~write_8bytes_t_t();

    private:
        uint16_t m_offset;
        uint64_t m_value;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset within page
         */
        uint16_t offset() const { return m_offset; }

        /**
         * 8-byte value to write
         */
        uint64_t value() const { return m_value; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

    /**
     * Write a string of bytes to a page at specified offset
     */

    class write_string_t_t : public kaitai::kstruct {

    public:

        write_string_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent = 0, innodb_redo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~write_string_t_t();

    private:
        uint16_t m_offset;
        compressed_uint_t_t* m_length;
        std::string m_data;
        innodb_redo_log_t* m__root;
        innodb_redo_log_t::log_record_t_t* m__parent;

    public:

        /**
         * Offset within page
         */
        uint16_t offset() const { return m_offset; }

        /**
         * Length of string in bytes
         */
        compressed_uint_t_t* length() const { return m_length; }

        /**
         * String data to write
         */
        std::string data() const { return m_data; }
        innodb_redo_log_t* _root() const { return m__root; }
        innodb_redo_log_t::log_record_t_t* _parent() const { return m__parent; }
    };

private:
    bool f_active_checkpoint;
    checkpoint_block_t_t* m_active_checkpoint;

public:

    /**
     * The most recent valid checkpoint (with higher checkpoint_no).
     * Used during crash recovery to determine the starting point for
     * log replay.
     */
    checkpoint_block_t_t* active_checkpoint();

private:
    bool f_log_format_version;
    uint32_t m_log_format_version;

public:

    /**
     * Redo log format version from file header
     */
    uint32_t log_format_version();

private:
    file_header_t_t* m_file_header;
    checkpoint_block_t_t* m_checkpoint_1;
    checkpoint_block_t_t* m_checkpoint_2;
    std::vector<log_block_t_t*>* m_log_blocks;
    innodb_redo_log_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Redo log file header (first 512 bytes)
     */
    file_header_t_t* file_header() const { return m_file_header; }

    /**
     * First checkpoint block (blocks 1-2)
     */
    checkpoint_block_t_t* checkpoint_1() const { return m_checkpoint_1; }

    /**
     * Second checkpoint block (blocks 3-4, alternate with checkpoint_1)
     */
    checkpoint_block_t_t* checkpoint_2() const { return m_checkpoint_2; }

    /**
     * Sequence of 512-byte log blocks containing redo records
     */
    std::vector<log_block_t_t*>* log_blocks() const { return m_log_blocks; }
    innodb_redo_log_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_REDO_LOG_H_
