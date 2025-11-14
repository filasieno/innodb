#ifndef INNODB_PAGE_INDEX_H_
#define INNODB_PAGE_INDEX_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_index_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include <set>
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_index_t : public kaitai::kstruct {

public:
    class fseg_header_t_t;
    class index_header_t_t;
    class infimum_supremum_record_t_t;
    class system_records_t_t;

    enum insert_direction_enum_t {
        INSERT_DIRECTION_ENUM_LEFT = 1,
        INSERT_DIRECTION_ENUM_RIGHT = 2,
        INSERT_DIRECTION_ENUM_SAME_REC = 3,
        INSERT_DIRECTION_ENUM_SAME_PAGE = 4,
        INSERT_DIRECTION_ENUM_NO_DIRECTION = 5
    };
    static bool _is_defined_insert_direction_enum_t(insert_direction_enum_t v);

private:
    static const std::set<insert_direction_enum_t> _values_insert_direction_enum_t;
    static std::set<insert_direction_enum_t> _build_values_insert_direction_enum_t();

public:

    innodb_page_index_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_index_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_index_t();

    /**
     * File segment header found on root pages of B-tree indexes.
     * Contains pointers to leaf and internal node segments.
     */

    class fseg_header_t_t : public kaitai::kstruct {

    public:

        fseg_header_t_t(kaitai::kstream* p__io, innodb_page_index_t* p__parent = 0, innodb_page_index_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~fseg_header_t_t();

    private:
        uint32_t m_leaf_inode_space;
        uint32_t m_leaf_inode_page_no;
        uint16_t m_leaf_inode_offset;
        uint32_t m_internal_inode_space;
        uint32_t m_internal_inode_page_no;
        uint16_t m_internal_inode_offset;
        innodb_page_index_t* m__root;
        innodb_page_index_t* m__parent;

    public:

        /**
         * Space ID of leaf segment inode
         */
        uint32_t leaf_inode_space() const { return m_leaf_inode_space; }

        /**
         * Page number of leaf segment inode
         */
        uint32_t leaf_inode_page_no() const { return m_leaf_inode_page_no; }

        /**
         * Offset within page of leaf segment inode
         */
        uint16_t leaf_inode_offset() const { return m_leaf_inode_offset; }

        /**
         * Space ID of internal node segment inode
         */
        uint32_t internal_inode_space() const { return m_internal_inode_space; }

        /**
         * Page number of internal node segment inode
         */
        uint32_t internal_inode_page_no() const { return m_internal_inode_page_no; }

        /**
         * Offset within page of internal node segment inode
         */
        uint16_t internal_inode_offset() const { return m_internal_inode_offset; }
        innodb_page_index_t* _root() const { return m__root; }
        innodb_page_index_t* _parent() const { return m__parent; }
    };

    /**
     * Index page header containing page-specific metadata.
     * Located at offset 38 (after FIL header).
     */

    class index_header_t_t : public kaitai::kstruct {

    public:

        index_header_t_t(kaitai::kstream* p__io, innodb_page_index_t* p__parent = 0, innodb_page_index_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~index_header_t_t();

    private:
        bool f_actual_n_heap;
        int32_t m_actual_n_heap;

    public:

        /**
         * Actual number of heap records (without format flag)
         */
        int32_t actual_n_heap();

    private:
        bool f_is_compact;
        bool m_is_compact;

    public:

        /**
         * True if page uses COMPACT record format
         */
        bool is_compact();

    private:
        bool f_is_leaf;
        bool m_is_leaf;

    public:

        /**
         * True if this is a leaf page
         */
        bool is_leaf();

    private:
        uint16_t m_n_dir_slots;
        uint16_t m_heap_top;
        uint16_t m_n_heap;
        uint16_t m_free_offset;
        uint16_t m_garbage_bytes;
        uint16_t m_last_insert_offset;
        insert_direction_enum_t m_direction;
        uint16_t m_n_direction;
        uint16_t m_n_recs;
        uint64_t m_max_trx_id;
        uint16_t m_level;
        uint64_t m_index_id;
        innodb_page_index_t* m__root;
        innodb_page_index_t* m__parent;

    public:

        /**
         * Number of slots in page directory
         */
        uint16_t n_dir_slots() const { return m_n_dir_slots; }

        /**
         * Offset of record heap top. Records are allocated from heap.
         * Heap grows from top of page downward.
         */
        uint16_t heap_top() const { return m_heap_top; }

        /**
         * Number of records in heap (including infimum, supremum, deleted).
         * Bit 15 (0x8000) indicates if page uses COMPACT format.
         */
        uint16_t n_heap() const { return m_n_heap; }

        /**
         * Offset to start of free record list.
         * 0xFFFF if no free records.
         */
        uint16_t free_offset() const { return m_free_offset; }

        /**
         * Number of bytes in deleted records (garbage)
         */
        uint16_t garbage_bytes() const { return m_garbage_bytes; }

        /**
         * Offset of last inserted record (for insert direction optimization)
         */
        uint16_t last_insert_offset() const { return m_last_insert_offset; }

        /**
         * Last insert direction (left, right, or unknown)
         */
        insert_direction_enum_t direction() const { return m_direction; }

        /**
         * Number of consecutive inserts in same direction.
         * Used to detect sequential insert patterns.
         */
        uint16_t n_direction() const { return m_n_direction; }

        /**
         * Number of user records on page (excludes infimum/supremum/deleted)
         */
        uint16_t n_recs() const { return m_n_recs; }

        /**
         * Maximum transaction ID that modified this page.
         * Used for MVCC and purge.
         */
        uint64_t max_trx_id() const { return m_max_trx_id; }

        /**
         * Level of this page in B-tree (0 = leaf, >0 = internal node).
         * Leaf pages contain actual data, internal nodes contain child pointers.
         */
        uint16_t level() const { return m_level; }

        /**
         * Index ID that this page belongs to
         */
        uint64_t index_id() const { return m_index_id; }
        innodb_page_index_t* _root() const { return m__root; }
        innodb_page_index_t* _parent() const { return m__parent; }
    };

    /**
     * Structure of infimum/supremum records.
     * These are special system records with fixed format.
     */

    class infimum_supremum_record_t_t : public kaitai::kstruct {

    public:

        infimum_supremum_record_t_t(kaitai::kstream* p__io, innodb_page_index_t::system_records_t_t* p__parent = 0, innodb_page_index_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~infimum_supremum_record_t_t();

    private:
        std::string m_record_header;
        std::string m_data;
        innodb_page_index_t* m__root;
        innodb_page_index_t::system_records_t_t* m__parent;

    public:

        /**
         * Record header (COMPACT format)
         */
        std::string record_header() const { return m_record_header; }

        /**
         * Data portion:
         * Infimum: "infimum" (8 bytes)
         * Supremum: "supremum" (8 bytes)
         */
        std::string data() const { return m_data; }
        innodb_page_index_t* _root() const { return m__root; }
        innodb_page_index_t::system_records_t_t* _parent() const { return m__parent; }
    };

    /**
     * Infimum and supremum records - boundary records on every index page.
     * 
     * Infimum: Minimum possible record (all searches start here)
     * Supremum: Maximum possible record (marks end of page)
     */

    class system_records_t_t : public kaitai::kstruct {

    public:

        system_records_t_t(kaitai::kstream* p__io, innodb_page_index_t* p__parent = 0, innodb_page_index_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~system_records_t_t();

    private:
        infimum_supremum_record_t_t* m_infimum;
        infimum_supremum_record_t_t* m_supremum;
        innodb_page_index_t* m__root;
        innodb_page_index_t* m__parent;

    public:

        /**
         * Infimum record (13 bytes)
         */
        infimum_supremum_record_t_t* infimum() const { return m_infimum; }

        /**
         * Supremum record (13 bytes)
         */
        infimum_supremum_record_t_t* supremum() const { return m_supremum; }
        innodb_page_index_t* _root() const { return m__root; }
        innodb_page_index_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    index_header_t_t* m_index_header;
    fseg_header_t_t* m_fseg_header;
    system_records_t_t* m_system_records;
    std::string m_user_records_and_free_space;
    std::vector<uint16_t>* m_page_directory;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_index_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Standard FIL header (38 bytes)
     */
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

    /**
     * Index page header (36 bytes)
     */
    index_header_t_t* index_header() const { return m_index_header; }

    /**
     * File segment header (20 bytes, only on root page)
     */
    fseg_header_t_t* fseg_header() const { return m_fseg_header; }

    /**
     * Infimum and supremum records
     */
    system_records_t_t* system_records() const { return m_system_records; }

    /**
     * User records and free space.
     * Actual parsing requires index metadata to interpret record format.
     */
    std::string user_records_and_free_space() const { return m_user_records_and_free_space; }

    /**
     * Page directory - array of record offsets.
     * Each slot points to a record that "owns" a group of records.
     * Used for binary search within page.
     */
    std::vector<uint16_t>* page_directory() const { return m_page_directory; }

    /**
     * Standard FIL trailer (8 bytes)
     */
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_index_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_INDEX_H_
