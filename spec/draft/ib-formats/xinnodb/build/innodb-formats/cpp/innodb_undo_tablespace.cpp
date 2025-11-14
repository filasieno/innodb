// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_undo_tablespace.h"

innodb_undo_tablespace_t::innodb_undo_tablespace_t(uint32_t p_page_size, kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_undo_tablespace_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_page_size = p_page_size;
    m_pages = 0;
    f_fsp_header = false;
    f_rseg_array = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_undo_tablespace_t::_read() {
    m_pages = new std::vector<page_wrapper_t_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_pages->push_back(new page_wrapper_t_t(m__io, this, m__root));
            i++;
        }
    }
}

innodb_undo_tablespace_t::~innodb_undo_tablespace_t() {
    _clean_up();
}

void innodb_undo_tablespace_t::_clean_up() {
    if (m_pages) {
        for (std::vector<page_wrapper_t_t*>::iterator it = m_pages->begin(); it != m_pages->end(); ++it) {
            delete *it;
        }
        delete m_pages; m_pages = 0;
    }
}

innodb_undo_tablespace_t::page_dispatcher_t_t::page_dispatcher_t_t(kaitai::kstream* p__io, innodb_undo_tablespace_t::page_wrapper_t_t* p__parent, innodb_undo_tablespace_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_fil_header = 0;
    m__io__raw_page_body = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_undo_tablespace_t::page_dispatcher_t_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    n_page_body = true;
    switch (fil_header()->page_type()) {
    case innodb_common_t::PAGE_TYPE_ENUM_FSP_HDR: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_fsp_hdr_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_UNDO_LOG: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_undo_log_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_XDES: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_xdes_t(m__io__raw_page_body);
        break;
    }
    default: {
        m__raw_page_body = m__io->read_bytes_full();
        break;
    }
    }
}

innodb_undo_tablespace_t::page_dispatcher_t_t::~page_dispatcher_t_t() {
    _clean_up();
}

void innodb_undo_tablespace_t::page_dispatcher_t_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (!n_page_body) {
        if (m__io__raw_page_body) {
            delete m__io__raw_page_body; m__io__raw_page_body = 0;
        }
        if (m_page_body) {
            delete m_page_body; m_page_body = 0;
        }
    }
}

innodb_undo_tablespace_t::page_wrapper_t_t::page_wrapper_t_t(kaitai::kstream* p__io, innodb_undo_tablespace_t* p__parent, innodb_undo_tablespace_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_page_data = 0;
    m__io__raw_page_data = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_undo_tablespace_t::page_wrapper_t_t::_read() {
    m__raw_page_data = m__io->read_bytes(_root()->page_size());
    m__io__raw_page_data = new kaitai::kstream(m__raw_page_data);
    m_page_data = new page_dispatcher_t_t(m__io__raw_page_data, this, m__root);
}

innodb_undo_tablespace_t::page_wrapper_t_t::~page_wrapper_t_t() {
    _clean_up();
}

void innodb_undo_tablespace_t::page_wrapper_t_t::_clean_up() {
    if (m__io__raw_page_data) {
        delete m__io__raw_page_data; m__io__raw_page_data = 0;
    }
    if (m_page_data) {
        delete m_page_data; m_page_data = 0;
    }
}

innodb_undo_tablespace_t::page_wrapper_t_t* innodb_undo_tablespace_t::fsp_header() {
    if (f_fsp_header)
        return m_fsp_header;
    f_fsp_header = true;
    m_fsp_header = pages()->at(0);
    return m_fsp_header;
}

innodb_undo_tablespace_t::page_wrapper_t_t* innodb_undo_tablespace_t::rseg_array() {
    if (f_rseg_array)
        return m_rseg_array;
    f_rseg_array = true;
    m_rseg_array = pages()->at(3);
    return m_rseg_array;
}
