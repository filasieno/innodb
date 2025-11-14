#ifndef INNODB_PAGE_ZLOB_INDEX_H_
#define INNODB_PAGE_ZLOB_INDEX_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_zlob_index_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_zlob_index_t : public kaitai::kstruct {

public:
    class zlob_index_header_t_t;

    innodb_page_zlob_index_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_zlob_index_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_zlob_index_t();

    /**
     * Header for compressed LOB index pages.
     */

    class zlob_index_header_t_t : public kaitai::kstruct {

    public:

        zlob_index_header_t_t(kaitai::kstream* p__io, innodb_page_zlob_index_t* p__parent = 0, innodb_page_zlob_index_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~zlob_index_header_t_t();

    private:
        uint8_t m_lob_version;
        uint8_t m_flags;
        std::string m_reserved;
        uint64_t m_total_compressed_len;
        uint64_t m_total_uncompressed_len;
        innodb_page_zlob_index_t* m__root;
        innodb_page_zlob_index_t* m__parent;

    public:

        /**
         * LOB version
         */
        uint8_t lob_version() const { return m_lob_version; }

        /**
         * Flags
         */
        uint8_t flags() const { return m_flags; }

        /**
         * Reserved
         */
        std::string reserved() const { return m_reserved; }

        /**
         * Total compressed length
         */
        uint64_t total_compressed_len() const { return m_total_compressed_len; }

        /**
         * Total uncompressed length
         */
        uint64_t total_uncompressed_len() const { return m_total_uncompressed_len; }
        innodb_page_zlob_index_t* _root() const { return m__root; }
        innodb_page_zlob_index_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    zlob_index_header_t_t* m_zlob_index_header;
    std::string m_index_entries;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_zlob_index_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Standard FIL header (38 bytes)
     */
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

    /**
     * Compressed LOB index header
     */
    zlob_index_header_t_t* zlob_index_header() const { return m_zlob_index_header; }

    /**
     * Index entries for compressed LOB pages
     */
    std::string index_entries() const { return m_index_entries; }

    /**
     * Standard FIL trailer (8 bytes)
     */
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_zlob_index_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_ZLOB_INDEX_H_
