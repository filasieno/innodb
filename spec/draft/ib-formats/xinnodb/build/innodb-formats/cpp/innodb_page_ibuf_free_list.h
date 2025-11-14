#ifndef INNODB_PAGE_IBUF_FREE_LIST_H_
#define INNODB_PAGE_IBUF_FREE_LIST_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_ibuf_free_list_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_ibuf_free_list_t : public kaitai::kstruct {

public:

    innodb_page_ibuf_free_list_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_ibuf_free_list_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_ibuf_free_list_t();

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    std::string m_free_list_data;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_ibuf_free_list_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }
    std::string free_list_data() const { return m_free_list_data; }
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_ibuf_free_list_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_IBUF_FREE_LIST_H_
