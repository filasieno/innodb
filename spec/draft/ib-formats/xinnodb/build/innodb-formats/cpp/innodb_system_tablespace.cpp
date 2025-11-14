// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_system_tablespace.h"

innodb_system_tablespace_t::innodb_system_tablespace_t(uint32_t p_page_size, kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_system_tablespace_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_page_size = p_page_size;
    m_pages = 0;
    f_first_rseg = false;
    f_fsp_header = false;
    f_ibuf_bitmap = false;
    f_ibuf_header = false;
    f_trx_sys = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_system_tablespace_t::_read() {
    m_pages = new std::vector<innodb_tablespace_t::page_wrapper_t_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_pages->push_back(new innodb_tablespace_t::page_wrapper_t_t(m__io));
            i++;
        }
    }
}

innodb_system_tablespace_t::~innodb_system_tablespace_t() {
    _clean_up();
}

void innodb_system_tablespace_t::_clean_up() {
    if (m_pages) {
        for (std::vector<innodb_tablespace_t::page_wrapper_t_t*>::iterator it = m_pages->begin(); it != m_pages->end(); ++it) {
            delete *it;
        }
        delete m_pages; m_pages = 0;
    }
}

innodb_tablespace_t::page_wrapper_t_t* innodb_system_tablespace_t::first_rseg() {
    if (f_first_rseg)
        return m_first_rseg;
    f_first_rseg = true;
    m_first_rseg = pages()->at(6);
    return m_first_rseg;
}

innodb_tablespace_t::page_wrapper_t_t* innodb_system_tablespace_t::fsp_header() {
    if (f_fsp_header)
        return m_fsp_header;
    f_fsp_header = true;
    m_fsp_header = pages()->at(0);
    return m_fsp_header;
}

innodb_tablespace_t::page_wrapper_t_t* innodb_system_tablespace_t::ibuf_bitmap() {
    if (f_ibuf_bitmap)
        return m_ibuf_bitmap;
    f_ibuf_bitmap = true;
    m_ibuf_bitmap = pages()->at(2);
    return m_ibuf_bitmap;
}

innodb_tablespace_t::page_wrapper_t_t* innodb_system_tablespace_t::ibuf_header() {
    if (f_ibuf_header)
        return m_ibuf_header;
    f_ibuf_header = true;
    m_ibuf_header = pages()->at(1);
    return m_ibuf_header;
}

innodb_tablespace_t::page_wrapper_t_t* innodb_system_tablespace_t::trx_sys() {
    if (f_trx_sys)
        return m_trx_sys;
    f_trx_sys = true;
    m_trx_sys = pages()->at(5);
    return m_trx_sys;
}
