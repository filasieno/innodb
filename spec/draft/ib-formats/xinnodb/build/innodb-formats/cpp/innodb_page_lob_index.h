#ifndef INNODB_PAGE_LOB_INDEX_H_
#define INNODB_PAGE_LOB_INDEX_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_lob_index_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_lob_index_t : public kaitai::kstruct {

public:
    class lob_index_header_t_t;

    innodb_page_lob_index_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_lob_index_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_lob_index_t();

    /**
     * Header for LOB index pages.
     */

    class lob_index_header_t_t : public kaitai::kstruct {

    public:

        lob_index_header_t_t(kaitai::kstream* p__io, innodb_page_lob_index_t* p__parent = 0, innodb_page_lob_index_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~lob_index_header_t_t();

    private:
        uint8_t m_lob_version;
        uint8_t m_flags;
        std::string m_reserved;
        uint64_t m_lob_total_len;
        uint64_t m_last_trx_id;
        uint64_t m_last_undo_no;
        innodb_page_lob_index_t* m__root;
        innodb_page_lob_index_t* m__parent;

    public:

        /**
         * LOB format version
         */
        uint8_t lob_version() const { return m_lob_version; }

        /**
         * LOB flags
         */
        uint8_t flags() const { return m_flags; }

        /**
         * Reserved
         */
        std::string reserved() const { return m_reserved; }

        /**
         * Total LOB length
         */
        uint64_t lob_total_len() const { return m_lob_total_len; }

        /**
         * Last transaction modifying this LOB
         */
        uint64_t last_trx_id() const { return m_last_trx_id; }

        /**
         * Last undo number
         */
        uint64_t last_undo_no() const { return m_last_undo_no; }
        innodb_page_lob_index_t* _root() const { return m__root; }
        innodb_page_lob_index_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    lob_index_header_t_t* m_lob_index_header;
    std::string m_index_entries;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_lob_index_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Standard FIL header (38 bytes)
     */
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

    /**
     * LOB index header
     */
    lob_index_header_t_t* lob_index_header() const { return m_lob_index_header; }

    /**
     * LOB index entries pointing to data pages
     */
    std::string index_entries() const { return m_index_entries; }

    /**
     * Standard FIL trailer (8 bytes)
     */
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_lob_index_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_LOB_INDEX_H_
