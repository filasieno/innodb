// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_xdes.h"

innodb_page_xdes_t::innodb_page_xdes_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_xdes_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_xdes_array = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_xdes_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_xdes_array = new std::vector<innodb_page_fsp_hdr_t::xdes_entry_t_t*>();
    const int l_xdes_array = 256;
    for (int i = 0; i < l_xdes_array; i++) {
        m_xdes_array->push_back(new innodb_page_fsp_hdr_t::xdes_entry_t_t(m__io));
    }
    m_empty_space = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_xdes_t::~innodb_page_xdes_t() {
    _clean_up();
}

void innodb_page_xdes_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_xdes_array) {
        for (std::vector<innodb_page_fsp_hdr_t::xdes_entry_t_t*>::iterator it = m_xdes_array->begin(); it != m_xdes_array->end(); ++it) {
            delete *it;
        }
        delete m_xdes_array; m_xdes_array = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}
