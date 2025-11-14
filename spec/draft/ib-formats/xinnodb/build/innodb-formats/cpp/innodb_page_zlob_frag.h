#ifndef INNODB_PAGE_ZLOB_FRAG_H_
#define INNODB_PAGE_ZLOB_FRAG_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_zlob_frag_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_zlob_frag_t : public kaitai::kstruct {

public:
    class zlob_frag_header_t_t;

    innodb_page_zlob_frag_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_zlob_frag_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_zlob_frag_t();

    /**
     * Header for compressed LOB fragment pages.
     */

    class zlob_frag_header_t_t : public kaitai::kstruct {

    public:

        zlob_frag_header_t_t(kaitai::kstream* p__io, innodb_page_zlob_frag_t* p__parent = 0, innodb_page_zlob_frag_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~zlob_frag_header_t_t();

    private:
        uint32_t m_n_frags;
        uint32_t m_used_len;
        uint64_t m_trx_id;
        innodb_page_zlob_frag_t* m__root;
        innodb_page_zlob_frag_t* m__parent;

    public:

        /**
         * Number of fragments in this page
         */
        uint32_t n_frags() const { return m_n_frags; }

        /**
         * Used length in page
         */
        uint32_t used_len() const { return m_used_len; }

        /**
         * Transaction ID
         */
        uint64_t trx_id() const { return m_trx_id; }
        innodb_page_zlob_frag_t* _root() const { return m__root; }
        innodb_page_zlob_frag_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    zlob_frag_header_t_t* m_zlob_frag_header;
    std::string m_fragment_data;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_zlob_frag_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Standard FIL header (38 bytes)
     */
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

    /**
     * Compressed LOB fragment header
     */
    zlob_frag_header_t_t* zlob_frag_header() const { return m_zlob_frag_header; }

    /**
     * Fragment data
     */
    std::string fragment_data() const { return m_fragment_data; }

    /**
     * Standard FIL trailer (8 bytes)
     */
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_zlob_frag_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_ZLOB_FRAG_H_
