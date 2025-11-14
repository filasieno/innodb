#ifndef INNODB_PAGE_XDES_H_
#define INNODB_PAGE_XDES_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_xdes_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include "innodb_page_fsp_hdr.h"
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_xdes_t : public kaitai::kstruct {

public:

    innodb_page_xdes_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_xdes_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_xdes_t();

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    std::vector<innodb_page_fsp_hdr_t::xdes_entry_t_t*>* m_xdes_array;
    std::string m_empty_space;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_xdes_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Standard FIL header (38 bytes)
     */
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

    /**
     * Array of 256 extent descriptors.
     * Each describes 64 pages, so this covers 16384 pages total.
     */
    std::vector<innodb_page_fsp_hdr_t::xdes_entry_t_t*>* xdes_array() const { return m_xdes_array; }

    /**
     * Remaining page space (unused in XDES page)
     */
    std::string empty_space() const { return m_empty_space; }

    /**
     * Standard FIL trailer (8 bytes)
     */
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_xdes_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_XDES_H_
