#ifndef INNODB_PAGE_UNDO_LOG_H_
#define INNODB_PAGE_UNDO_LOG_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_undo_log_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include <set>
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_undo_log_t : public kaitai::kstruct {

public:
    class undo_delete_data_t_t;
    class undo_field_data_t_t;
    class undo_insert_data_t_t;
    class undo_page_header_t_t;
    class undo_record_header_t_t;
    class undo_record_list_t_t;
    class undo_record_t_t;
    class undo_record_with_next_t_t;
    class undo_truncate_data_t_t;
    class undo_update_data_t_t;

    enum undo_page_type_enum_t {
        UNDO_PAGE_TYPE_ENUM_INSERT = 1,
        UNDO_PAGE_TYPE_ENUM_UPDATE = 2
    };
    static bool _is_defined_undo_page_type_enum_t(undo_page_type_enum_t v);

private:
    static const std::set<undo_page_type_enum_t> _values_undo_page_type_enum_t;
    static std::set<undo_page_type_enum_t> _build_values_undo_page_type_enum_t();

public:

    enum undo_record_type_enum_t {
        UNDO_RECORD_TYPE_ENUM_INSERT = 11,
        UNDO_RECORD_TYPE_ENUM_UPDATE_EXISTING = 12,
        UNDO_RECORD_TYPE_ENUM_UPDATE_DELETED = 13,
        UNDO_RECORD_TYPE_ENUM_DELETE = 14,
        UNDO_RECORD_TYPE_ENUM_PURGE = 15,
        UNDO_RECORD_TYPE_ENUM_INSERT_TRUNCATE = 16,
        UNDO_RECORD_TYPE_ENUM_UPDATE_TRUNCATE = 17
    };
    static bool _is_defined_undo_record_type_enum_t(undo_record_type_enum_t v);

private:
    static const std::set<undo_record_type_enum_t> _values_undo_record_type_enum_t;
    static std::set<undo_record_type_enum_t> _build_values_undo_record_type_enum_t();

public:

    innodb_page_undo_log_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_undo_log_t();

    class undo_delete_data_t_t : public kaitai::kstruct {

    public:

        undo_delete_data_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_t_t* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_delete_data_t_t();

    private:
        undo_record_header_t_t* m_record_header;
        std::string m_null_bitmap;
        std::vector<undo_field_data_t_t*>* m_field_data;
        innodb_page_undo_log_t* m__root;
        innodb_page_undo_log_t::undo_record_t_t* m__parent;

    public:

        /**
         * InnoDB record header for the deleted row
         */
        undo_record_header_t_t* record_header() const { return m_record_header; }

        /**
         * Null bitmap indicating which fields are NULL
         */
        std::string null_bitmap() const { return m_null_bitmap; }

        /**
         * Field data for each column in the deleted row
         */
        std::vector<undo_field_data_t_t*>* field_data() const { return m_field_data; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        innodb_page_undo_log_t::undo_record_t_t* _parent() const { return m__parent; }
    };

    class undo_field_data_t_t : public kaitai::kstruct {

    public:

        undo_field_data_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_field_data_t_t();

    private:
        uint32_t m_len_field_value;
        std::string m_field_value;
        innodb_page_undo_log_t* m__root;
        kaitai::kstruct* m__parent;

    public:

        /**
         * Length of the field data
         */
        uint32_t len_field_value() const { return m_len_field_value; }

        /**
         * The actual field data (varies by type)
         */
        std::string field_value() const { return m_field_value; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    class undo_insert_data_t_t : public kaitai::kstruct {

    public:

        undo_insert_data_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_t_t* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_insert_data_t_t();

    private:
        std::vector<undo_field_data_t_t*>* m_primary_key_fields;
        innodb_page_undo_log_t* m__root;
        innodb_page_undo_log_t::undo_record_t_t* m__parent;

    public:

        /**
         * Primary key fields of the inserted row (for rollback)
         */
        std::vector<undo_field_data_t_t*>* primary_key_fields() const { return m_primary_key_fields; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        innodb_page_undo_log_t::undo_record_t_t* _parent() const { return m__parent; }
    };

    class undo_page_header_t_t : public kaitai::kstruct {

    public:

        undo_page_header_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_page_header_t_t();

    private:
        undo_page_type_enum_t m_page_type;
        uint16_t m_latest_log_record_offset;
        uint16_t m_free_offset;
        std::string m_page_list_node;
        innodb_page_undo_log_t* m__root;
        innodb_page_undo_log_t* m__parent;

    public:
        undo_page_type_enum_t page_type() const { return m_page_type; }
        uint16_t latest_log_record_offset() const { return m_latest_log_record_offset; }
        uint16_t free_offset() const { return m_free_offset; }
        std::string page_list_node() const { return m_page_list_node; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        innodb_page_undo_log_t* _parent() const { return m__parent; }
    };

    class undo_record_header_t_t : public kaitai::kstruct {

    public:

        undo_record_header_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_delete_data_t_t* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_record_header_t_t();

    private:
        uint8_t m_info_flags;
        uint32_t m_num_fields;
        innodb_page_undo_log_t* m__root;
        innodb_page_undo_log_t::undo_delete_data_t_t* m__parent;

    public:

        /**
         * Record info flags
         */
        uint8_t info_flags() const { return m_info_flags; }

        /**
         * Number of fields in this record
         */
        uint32_t num_fields() const { return m_num_fields; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        innodb_page_undo_log_t::undo_delete_data_t_t* _parent() const { return m__parent; }
    };

    class undo_record_list_t_t : public kaitai::kstruct {

    public:

        undo_record_list_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_record_list_t_t();

    private:
        bool f_first_record;
        undo_record_with_next_t_t* m_first_record;
        bool n_first_record;

    public:
        bool _is_null_first_record() { first_record(); return n_first_record; };

    private:

    public:
        undo_record_with_next_t_t* first_record();

    private:
        innodb_page_undo_log_t* m__root;
        innodb_page_undo_log_t* m__parent;

    public:
        innodb_page_undo_log_t* _root() const { return m__root; }
        innodb_page_undo_log_t* _parent() const { return m__parent; }
    };

    class undo_record_t_t : public kaitai::kstruct {

    public:

        undo_record_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_with_next_t_t* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_record_t_t();

    private:
        undo_record_type_enum_t m_undo_rec_type;
        innodb_common_t::mach_compressed_uint_t_t* m_undo_no;
        innodb_common_t::mach_compressed_uint_t_t* m_table_id;
        uint8_t m_info_bits;
        uint64_t m_trx_id;
        uint64_t m_roll_ptr;
        kaitai::kstruct* m_data;
        bool n_data;

    public:
        bool _is_null_data() { data(); return n_data; };

    private:
        uint16_t m_next_record_offset;
        innodb_page_undo_log_t* m__root;
        innodb_page_undo_log_t::undo_record_with_next_t_t* m__parent;

    public:
        undo_record_type_enum_t undo_rec_type() const { return m_undo_rec_type; }
        innodb_common_t::mach_compressed_uint_t_t* undo_no() const { return m_undo_no; }
        innodb_common_t::mach_compressed_uint_t_t* table_id() const { return m_table_id; }
        uint8_t info_bits() const { return m_info_bits; }
        uint64_t trx_id() const { return m_trx_id; }
        uint64_t roll_ptr() const { return m_roll_ptr; }
        kaitai::kstruct* data() const { return m_data; }
        uint16_t next_record_offset() const { return m_next_record_offset; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        innodb_page_undo_log_t::undo_record_with_next_t_t* _parent() const { return m__parent; }
    };

    class undo_record_with_next_t_t : public kaitai::kstruct {

    public:

        undo_record_with_next_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_record_with_next_t_t();

    private:
        bool f_next_record;
        undo_record_with_next_t_t* m_next_record;
        bool n_next_record;

    public:
        bool _is_null_next_record() { next_record(); return n_next_record; };

    private:

    public:
        undo_record_with_next_t_t* next_record();

    private:
        undo_record_t_t* m_record;
        innodb_page_undo_log_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        undo_record_t_t* record() const { return m_record; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    class undo_truncate_data_t_t : public kaitai::kstruct {

    public:

        undo_truncate_data_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_t_t* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_truncate_data_t_t();

    private:
        innodb_common_t::mach_compressed_uint_t_t* m_truncate_table_id;
        uint32_t m_truncate_flags;
        uint64_t m_truncate_index_id;
        std::string m_truncate_extra_data;
        innodb_page_undo_log_t* m__root;
        innodb_page_undo_log_t::undo_record_t_t* m__parent;

    public:

        /**
         * Table ID being truncated
         */
        innodb_common_t::mach_compressed_uint_t_t* truncate_table_id() const { return m_truncate_table_id; }

        /**
         * Truncate operation flags
         */
        uint32_t truncate_flags() const { return m_truncate_flags; }

        /**
         * Index ID for truncate operations
         */
        uint64_t truncate_index_id() const { return m_truncate_index_id; }

        /**
         * Additional truncate operation metadata
         */
        std::string truncate_extra_data() const { return m_truncate_extra_data; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        innodb_page_undo_log_t::undo_record_t_t* _parent() const { return m__parent; }
    };

    class undo_update_data_t_t : public kaitai::kstruct {

    public:

        undo_update_data_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_t_t* p__parent = 0, innodb_page_undo_log_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~undo_update_data_t_t();

    private:
        uint32_t m_num_field_numbers;
        std::vector<innodb_common_t::mach_compressed_uint_t_t*>* m_field_numbers;
        std::vector<undo_field_data_t_t*>* m_field_old_values;
        innodb_page_undo_log_t* m__root;
        innodb_page_undo_log_t::undo_record_t_t* m__parent;

    public:

        /**
         * Number of fields that were updated
         */
        uint32_t num_field_numbers() const { return m_num_field_numbers; }

        /**
         * Field numbers (column positions) that were updated
         */
        std::vector<innodb_common_t::mach_compressed_uint_t_t*>* field_numbers() const { return m_field_numbers; }

        /**
         * Old values of the updated fields
         */
        std::vector<undo_field_data_t_t*>* field_old_values() const { return m_field_old_values; }
        innodb_page_undo_log_t* _root() const { return m__root; }
        innodb_page_undo_log_t::undo_record_t_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    undo_page_header_t_t* m_undo_page_header;
    undo_record_list_t_t* m_undo_records;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_undo_log_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }
    undo_page_header_t_t* undo_page_header() const { return m_undo_page_header; }
    undo_record_list_t_t* undo_records() const { return m_undo_records; }
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_undo_log_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_UNDO_LOG_H_
