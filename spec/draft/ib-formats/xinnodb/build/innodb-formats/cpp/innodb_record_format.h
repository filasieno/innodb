#ifndef INNODB_RECORD_FORMAT_H_
#define INNODB_RECORD_FORMAT_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_record_format_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include <set>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_record_format_t : public kaitai::kstruct {

public:
    class blob_reference_t_t;
    class compact_record_header_t_t;

    enum field_type_enum_t {
        FIELD_TYPE_ENUM_VARCHAR = 1,
        FIELD_TYPE_ENUM_CHAR = 2,
        FIELD_TYPE_ENUM_BINARY = 3,
        FIELD_TYPE_ENUM_VARBINARY = 4,
        FIELD_TYPE_ENUM_BLOB = 5,
        FIELD_TYPE_ENUM_BLOB_TYPE = 252
    };
    static bool _is_defined_field_type_enum_t(field_type_enum_t v);

private:
    static const std::set<field_type_enum_t> _values_field_type_enum_t;
    static std::set<field_type_enum_t> _build_values_field_type_enum_t();

public:

    enum record_type_enum_t {
        RECORD_TYPE_ENUM_CONVENTIONAL = 0,
        RECORD_TYPE_ENUM_NODE_POINTER = 1,
        RECORD_TYPE_ENUM_INFIMUM = 2,
        RECORD_TYPE_ENUM_SUPREMUM = 3
    };
    static bool _is_defined_record_type_enum_t(record_type_enum_t v);

private:
    static const std::set<record_type_enum_t> _values_record_type_enum_t;
    static std::set<record_type_enum_t> _build_values_record_type_enum_t();

public:

    innodb_record_format_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_record_format_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_record_format_t();

    class blob_reference_t_t : public kaitai::kstruct {

    public:

        blob_reference_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_record_format_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~blob_reference_t_t();

    private:
        uint32_t m_space_id;
        uint32_t m_page_no;
        uint32_t m_offset;
        uint64_t m_blob_length;
        innodb_record_format_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint32_t space_id() const { return m_space_id; }
        uint32_t page_no() const { return m_page_no; }
        uint32_t offset() const { return m_offset; }
        uint64_t blob_length() const { return m_blob_length; }
        innodb_record_format_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    class compact_record_header_t_t : public kaitai::kstruct {

    public:

        compact_record_header_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_record_format_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~compact_record_header_t_t();

    private:
        bool f_is_deleted;
        bool m_is_deleted;

    public:
        bool is_deleted();

    private:
        bool f_is_min_rec;
        bool m_is_min_rec;

    public:
        bool is_min_rec();

    private:
        uint8_t m_info_flags;
        uint8_t m_n_owned;
        uint16_t m_heap_no;
        record_type_enum_t m_record_type;
        int16_t m_next_record_offset;
        innodb_record_format_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint8_t info_flags() const { return m_info_flags; }
        uint8_t n_owned() const { return m_n_owned; }
        uint16_t heap_no() const { return m_heap_no; }
        record_type_enum_t record_type() const { return m_record_type; }
        int16_t next_record_offset() const { return m_next_record_offset; }
        innodb_record_format_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

private:
    innodb_record_format_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_record_format_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_RECORD_FORMAT_H_
