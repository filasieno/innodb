#ifndef INNODB_PAGE_FSP_HDR_H_
#define INNODB_PAGE_FSP_HDR_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_fsp_hdr_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include <set>
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_fsp_hdr_t : public kaitai::kstruct {

public:
    class fil_addr_t_t;
    class flst_base_node_t_t;
    class flst_node_t_t;
    class fsp_header_t_t;
    class xdes_entry_t_t;

    enum xdes_state_enum_t {
        XDES_STATE_ENUM_FREE = 1,
        XDES_STATE_ENUM_FREE_FRAG = 2,
        XDES_STATE_ENUM_FULL_FRAG = 3,
        XDES_STATE_ENUM_FSEG = 4,
        XDES_STATE_ENUM_FSEG_FRAG = 5
    };
    static bool _is_defined_xdes_state_enum_t(xdes_state_enum_t v);

private:
    static const std::set<xdes_state_enum_t> _values_xdes_state_enum_t;
    static std::set<xdes_state_enum_t> _build_values_xdes_state_enum_t();

public:

    innodb_page_fsp_hdr_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_fsp_hdr_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_fsp_hdr_t();

    /**
     * File address - points to a location within the tablespace.
     * Consists of page number and offset within page.
     */

    class fil_addr_t_t : public kaitai::kstruct {

    public:

        fil_addr_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_fsp_hdr_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~fil_addr_t_t();

    private:
        bool f_is_null;
        bool m_is_null;

    public:

        /**
         * True if this is a null pointer
         */
        bool is_null();

    private:
        uint32_t m_page_no;
        uint16_t m_byte_offset;
        innodb_page_fsp_hdr_t* m__root;
        kaitai::kstruct* m__parent;

    public:

        /**
         * Page number
         */
        uint32_t page_no() const { return m_page_no; }

        /**
         * Byte offset within page
         */
        uint16_t byte_offset() const { return m_byte_offset; }
        innodb_page_fsp_hdr_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    /**
     * Base node of a file-based list.
     * InnoDB uses doubly-linked lists stored across pages.
     * This structure tracks the list head and tail.
     */

    class flst_base_node_t_t : public kaitai::kstruct {

    public:

        flst_base_node_t_t(kaitai::kstream* p__io, innodb_page_fsp_hdr_t::fsp_header_t_t* p__parent = 0, innodb_page_fsp_hdr_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~flst_base_node_t_t();

    private:
        uint32_t m_length;
        fil_addr_t_t* m_first_node;
        fil_addr_t_t* m_last_node;
        innodb_page_fsp_hdr_t* m__root;
        innodb_page_fsp_hdr_t::fsp_header_t_t* m__parent;

    public:

        /**
         * Number of nodes in list
         */
        uint32_t length() const { return m_length; }

        /**
         * File address of first node
         */
        fil_addr_t_t* first_node() const { return m_first_node; }

        /**
         * File address of last node
         */
        fil_addr_t_t* last_node() const { return m_last_node; }
        innodb_page_fsp_hdr_t* _root() const { return m__root; }
        innodb_page_fsp_hdr_t::fsp_header_t_t* _parent() const { return m__parent; }
    };

    /**
     * File list node - part of doubly-linked list structure.
     * Contains pointers to previous and next nodes.
     */

    class flst_node_t_t : public kaitai::kstruct {

    public:

        flst_node_t_t(kaitai::kstream* p__io, innodb_page_fsp_hdr_t::xdes_entry_t_t* p__parent = 0, innodb_page_fsp_hdr_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~flst_node_t_t();

    private:
        fil_addr_t_t* m_prev;
        fil_addr_t_t* m_next;
        innodb_page_fsp_hdr_t* m__root;
        innodb_page_fsp_hdr_t::xdes_entry_t_t* m__parent;

    public:

        /**
         * File address of previous node
         */
        fil_addr_t_t* prev() const { return m_prev; }

        /**
         * File address of next node
         */
        fil_addr_t_t* next() const { return m_next; }
        innodb_page_fsp_hdr_t* _root() const { return m__root; }
        innodb_page_fsp_hdr_t::xdes_entry_t_t* _parent() const { return m__parent; }
    };

    /**
     * File space header containing global tablespace information.
     * Located at offset 38 (after FIL header) on page 0.
     * Total size: 112 bytes
     */

    class fsp_header_t_t : public kaitai::kstruct {

    public:

        fsp_header_t_t(kaitai::kstream* p__io, innodb_page_fsp_hdr_t* p__parent = 0, innodb_page_fsp_hdr_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~fsp_header_t_t();

    private:
        uint32_t m_space_id;
        uint32_t m_unused;
        uint32_t m_size;
        uint32_t m_free_limit;
        innodb_common_t::space_flags_t_t* m_space_flags;
        uint32_t m_frag_n_used;
        flst_base_node_t_t* m_free_list;
        flst_base_node_t_t* m_free_frag_list;
        flst_base_node_t_t* m_full_frag_list;
        uint64_t m_next_unused_seg_id;
        flst_base_node_t_t* m_full_inodes_list;
        flst_base_node_t_t* m_free_inodes_list;
        innodb_page_fsp_hdr_t* m__root;
        innodb_page_fsp_hdr_t* m__parent;

    public:

        /**
         * Tablespace identifier
         */
        uint32_t space_id() const { return m_space_id; }

        /**
         * Reserved, unused
         */
        uint32_t unused() const { return m_unused; }

        /**
         * Current size of tablespace in pages
         */
        uint32_t size() const { return m_size; }

        /**
         * Free space limit - pages beyond this are uninitialized.
         * Used for extending tablespace.
         */
        uint32_t free_limit() const { return m_free_limit; }

        /**
         * Tablespace flags (page size, format, compression, etc.)
         */
        innodb_common_t::space_flags_t_t* space_flags() const { return m_space_flags; }

        /**
         * Number of used pages in fragment list
         */
        uint32_t frag_n_used() const { return m_frag_n_used; }

        /**
         * Base node of free extent list
         */
        flst_base_node_t_t* free_list() const { return m_free_list; }

        /**
         * Base node of free fragment extent list
         */
        flst_base_node_t_t* free_frag_list() const { return m_free_frag_list; }

        /**
         * Base node of full fragment extent list
         */
        flst_base_node_t_t* full_frag_list() const { return m_full_frag_list; }

        /**
         * Next unused segment ID
         */
        uint64_t next_unused_seg_id() const { return m_next_unused_seg_id; }

        /**
         * Base node of full inode page list
         */
        flst_base_node_t_t* full_inodes_list() const { return m_full_inodes_list; }

        /**
         * Base node of free inode page list
         */
        flst_base_node_t_t* free_inodes_list() const { return m_free_inodes_list; }
        innodb_page_fsp_hdr_t* _root() const { return m__root; }
        innodb_page_fsp_hdr_t* _parent() const { return m__parent; }
    };

    /**
     * Extent descriptor (XDES) entry describing one extent.
     * 
     * An extent is a group of 64 consecutive pages (1MB for 16KB pages).
     * The descriptor tracks which pages in the extent are free/used.
     */

    class xdes_entry_t_t : public kaitai::kstruct {

    public:

        xdes_entry_t_t(kaitai::kstream* p__io, innodb_page_fsp_hdr_t* p__parent = 0, innodb_page_fsp_hdr_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~xdes_entry_t_t();

    private:
        bool f_is_free;
        bool m_is_free;

    public:

        /**
         * True if extent is free (not owned by any segment)
         */
        bool is_free();

    private:
        uint64_t m_file_segment_id;
        flst_node_t_t* m_list_node;
        xdes_state_enum_t m_state;
        std::string m_page_state_bitmap;
        innodb_page_fsp_hdr_t* m__root;
        innodb_page_fsp_hdr_t* m__parent;

    public:

        /**
         * ID of file segment owning this extent.
         * 0 = extent is free
         */
        uint64_t file_segment_id() const { return m_file_segment_id; }

        /**
         * List node for linking in free/full/fragment lists
         */
        flst_node_t_t* list_node() const { return m_list_node; }

        /**
         * State of extent (free, free_frag, full_frag, fseg)
         */
        xdes_state_enum_t state() const { return m_state; }

        /**
         * Bitmap of page states within extent.
         * 2 bits per page × 64 pages = 128 bits = 16 bytes
         * 
         * Bit values:
         * 00 = free
         * 01 = allocated but not used
         * 10 = allocated and used (contains data)
         * 11 = reserved
         */
        std::string page_state_bitmap() const { return m_page_state_bitmap; }
        innodb_page_fsp_hdr_t* _root() const { return m__root; }
        innodb_page_fsp_hdr_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    fsp_header_t_t* m_fsp_header;
    std::vector<xdes_entry_t_t*>* m_xdes_array;
    std::string m_empty_space;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_fsp_hdr_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Standard FIL header (38 bytes)
     */
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

    /**
     * File space header (112 bytes)
     */
    fsp_header_t_t* fsp_header() const { return m_fsp_header; }

    /**
     * Array of 256 extent descriptors.
     * Each extent is 64 pages (1MB for 16KB pages).
     * This array describes the first 16384 pages of the tablespace.
     */
    std::vector<xdes_entry_t_t*>* xdes_array() const { return m_xdes_array; }

    /**
     * Remaining page space (unused in FSP_HDR page)
     */
    std::string empty_space() const { return m_empty_space; }

    /**
     * Standard FIL trailer (8 bytes)
     */
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_fsp_hdr_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_FSP_HDR_H_
