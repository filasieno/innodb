#ifndef INNODB_COMMON_H_
#define INNODB_COMMON_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_common_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include <set>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_common_t : public kaitai::kstruct {

public:
    class fil_header_t_t;
    class fil_trailer_t_t;
    class mach_compressed_uint_t_t;
    class space_flags_t_t;

    enum checksum_algorithm_enum_t {
        CHECKSUM_ALGORITHM_ENUM_CRC32 = 0,
        CHECKSUM_ALGORITHM_ENUM_INNODB = 1,
        CHECKSUM_ALGORITHM_ENUM_NONE = 2,
        CHECKSUM_ALGORITHM_ENUM_STRICT_CRC32 = 3,
        CHECKSUM_ALGORITHM_ENUM_STRICT_INNODB = 4,
        CHECKSUM_ALGORITHM_ENUM_STRICT_NONE = 5
    };
    static bool _is_defined_checksum_algorithm_enum_t(checksum_algorithm_enum_t v);

private:
    static const std::set<checksum_algorithm_enum_t> _values_checksum_algorithm_enum_t;
    static std::set<checksum_algorithm_enum_t> _build_values_checksum_algorithm_enum_t();

public:

    enum page_type_enum_t {
        PAGE_TYPE_ENUM_ALLOCATED = 0,
        PAGE_TYPE_ENUM_UNDO_LOG = 2,
        PAGE_TYPE_ENUM_INODE = 3,
        PAGE_TYPE_ENUM_IBUF_FREE_LIST = 4,
        PAGE_TYPE_ENUM_IBUF_BITMAP = 5,
        PAGE_TYPE_ENUM_SYS = 6,
        PAGE_TYPE_ENUM_TRX_SYS = 7,
        PAGE_TYPE_ENUM_FSP_HDR = 8,
        PAGE_TYPE_ENUM_XDES = 9,
        PAGE_TYPE_ENUM_BLOB = 10,
        PAGE_TYPE_ENUM_ZLOB_FIRST = 11,
        PAGE_TYPE_ENUM_ZLOB_DATA = 12,
        PAGE_TYPE_ENUM_ZLOB_INDEX = 13,
        PAGE_TYPE_ENUM_ZBLOB = 14,
        PAGE_TYPE_ENUM_ZBLOB2 = 15,
        PAGE_TYPE_ENUM_UNKNOWN = 16,
        PAGE_TYPE_ENUM_INDEX = 17,
        PAGE_TYPE_ENUM_SDI_BLOB = 18,
        PAGE_TYPE_ENUM_SDI_ZBLOB = 19,
        PAGE_TYPE_ENUM_LOB_INDEX = 20,
        PAGE_TYPE_ENUM_LOB_DATA = 21,
        PAGE_TYPE_ENUM_LOB_FIRST = 22,
        PAGE_TYPE_ENUM_ZLOB_FIRST_V2 = 23,
        PAGE_TYPE_ENUM_ZLOB_DATA_V2 = 24,
        PAGE_TYPE_ENUM_ZLOB_INDEX_V2 = 25,
        PAGE_TYPE_ENUM_ZLOB_FRAG = 26,
        PAGE_TYPE_ENUM_ZLOB_FRAG_ENTRY = 27,
        PAGE_TYPE_ENUM_RTREE = 28
    };
    static bool _is_defined_page_type_enum_t(page_type_enum_t v);

private:
    static const std::set<page_type_enum_t> _values_page_type_enum_t;
    static std::set<page_type_enum_t> _build_values_page_type_enum_t();

public:

    enum row_format_enum_t {
        ROW_FORMAT_ENUM_REDUNDANT = 0,
        ROW_FORMAT_ENUM_COMPACT = 1,
        ROW_FORMAT_ENUM_DYNAMIC = 2,
        ROW_FORMAT_ENUM_COMPRESSED = 3
    };
    static bool _is_defined_row_format_enum_t(row_format_enum_t v);

private:
    static const std::set<row_format_enum_t> _values_row_format_enum_t;
    static std::set<row_format_enum_t> _build_values_row_format_enum_t();

public:

    innodb_common_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_common_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_common_t();

    class fil_header_t_t : public kaitai::kstruct {

    public:

        fil_header_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_common_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~fil_header_t_t();

    private:
        uint32_t m_checksum;
        uint32_t m_page_no;
        uint64_t m_prev_page_lsn;
        page_type_enum_t m_page_type;
        uint64_t m_flush_lsn;
        bool n_flush_lsn;

    public:
        bool _is_null_flush_lsn() { flush_lsn(); return n_flush_lsn; };

    private:
        uint32_t m_space_id;
        innodb_common_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint32_t checksum() const { return m_checksum; }
        uint32_t page_no() const { return m_page_no; }
        uint64_t prev_page_lsn() const { return m_prev_page_lsn; }
        page_type_enum_t page_type() const { return m_page_type; }
        uint64_t flush_lsn() const { return m_flush_lsn; }
        uint32_t space_id() const { return m_space_id; }
        innodb_common_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    class fil_trailer_t_t : public kaitai::kstruct {

    public:

        fil_trailer_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_common_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~fil_trailer_t_t();

    private:
        uint32_t m_old_checksum;
        uint32_t m_lsn_low32;
        innodb_common_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint32_t old_checksum() const { return m_old_checksum; }
        uint32_t lsn_low32() const { return m_lsn_low32; }
        innodb_common_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    class mach_compressed_uint_t_t : public kaitai::kstruct {

    public:

        mach_compressed_uint_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_common_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~mach_compressed_uint_t_t();

    private:
        bool f_value;
        int32_t m_value;

    public:
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
        innodb_common_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint8_t first_byte() const { return m_first_byte; }
        uint8_t second_byte() const { return m_second_byte; }
        uint8_t third_byte() const { return m_third_byte; }
        uint8_t fourth_byte() const { return m_fourth_byte; }
        uint8_t fifth_byte() const { return m_fifth_byte; }
        innodb_common_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    class space_flags_t_t : public kaitai::kstruct {

    public:

        space_flags_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_common_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~space_flags_t_t();

    private:
        bool f_atomic_blobs;
        int32_t m_atomic_blobs;

    public:
        int32_t atomic_blobs();

    private:
        bool f_data_dir;
        int32_t m_data_dir;

    public:
        int32_t data_dir();

    private:
        bool f_encryption;
        int32_t m_encryption;

    public:
        int32_t encryption();

    private:
        bool f_page_ssize;
        int32_t m_page_ssize;

    public:
        int32_t page_ssize();

    private:
        bool f_post_antelope;
        bool m_post_antelope;

    public:
        bool post_antelope();

    private:
        bool f_sdi;
        int32_t m_sdi;

    public:
        int32_t sdi();

    private:
        bool f_shared;
        int32_t m_shared;

    public:
        int32_t shared();

    private:
        bool f_temporary;
        int32_t m_temporary;

    public:
        int32_t temporary();

    private:
        bool f_zip_ssize;
        int32_t m_zip_ssize;

    public:
        int32_t zip_ssize();

    private:
        uint32_t m_flags_value;
        innodb_common_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint32_t flags_value() const { return m_flags_value; }
        innodb_common_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

private:
    innodb_common_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_common_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_COMMON_H_
