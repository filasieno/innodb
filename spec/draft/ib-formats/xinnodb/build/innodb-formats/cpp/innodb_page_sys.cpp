// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_sys.h"

innodb_page_sys_t::innodb_page_sys_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_sys_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_sys_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_page_data = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_sys_t::~innodb_page_sys_t() {
    _clean_up();
}

void innodb_page_sys_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}
