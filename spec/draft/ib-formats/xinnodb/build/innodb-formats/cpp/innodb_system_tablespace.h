#ifndef INNODB_SYSTEM_TABLESPACE_H_
#define INNODB_SYSTEM_TABLESPACE_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_system_tablespace_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_tablespace.h"
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_system_tablespace_t : public kaitai::kstruct {

public:

    innodb_system_tablespace_t(uint32_t p_page_size, kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_system_tablespace_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_system_tablespace_t();

private:
    bool f_first_rseg;
    innodb_tablespace_t::page_wrapper_t_t* m_first_rseg;

public:

    /**
     * First rollback segment page (page 6)
     */
    innodb_tablespace_t::page_wrapper_t_t* first_rseg();

private:
    bool f_fsp_header;
    innodb_tablespace_t::page_wrapper_t_t* m_fsp_header;

public:

    /**
     * FSP header (page 0)
     */
    innodb_tablespace_t::page_wrapper_t_t* fsp_header();

private:
    bool f_ibuf_bitmap;
    innodb_tablespace_t::page_wrapper_t_t* m_ibuf_bitmap;

public:

    /**
     * Insert buffer bitmap (page 2)
     */
    innodb_tablespace_t::page_wrapper_t_t* ibuf_bitmap();

private:
    bool f_ibuf_header;
    innodb_tablespace_t::page_wrapper_t_t* m_ibuf_header;

public:

    /**
     * Insert buffer header (page 1)
     */
    innodb_tablespace_t::page_wrapper_t_t* ibuf_header();

private:
    bool f_trx_sys;
    innodb_tablespace_t::page_wrapper_t_t* m_trx_sys;

public:

    /**
     * Transaction system header (page 5)
     */
    innodb_tablespace_t::page_wrapper_t_t* trx_sys();

private:
    std::vector<innodb_tablespace_t::page_wrapper_t_t*>* m_pages;
    uint32_t m_page_size;
    innodb_system_tablespace_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Pages in system tablespace
     */
    std::vector<innodb_tablespace_t::page_wrapper_t_t*>* pages() const { return m_pages; }

    /**
     * Page size (default 16KB)
     */
    uint32_t page_size() const { return m_page_size; }
    innodb_system_tablespace_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_SYSTEM_TABLESPACE_H_
