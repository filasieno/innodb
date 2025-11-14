// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_rseg_array.h"

innodb_page_rseg_array_t::innodb_page_rseg_array_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_rseg_array_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_rseg_array_header = 0;
    m_rseg_slots = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_rseg_array_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_rseg_array_header = new rseg_array_header_t_t(m__io, this, m__root);
    m_rseg_slots = new std::vector<uint32_t>();
    const int l_rseg_slots = rseg_array_header()->max_rollback_segments();
    for (int i = 0; i < l_rseg_slots; i++) {
        m_rseg_slots->push_back(m__io->read_u4le());
    }
    m_empty_space = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_rseg_array_t::~innodb_page_rseg_array_t() {
    _clean_up();
}

void innodb_page_rseg_array_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_rseg_array_header) {
        delete m_rseg_array_header; m_rseg_array_header = 0;
    }
    if (m_rseg_slots) {
        delete m_rseg_slots; m_rseg_slots = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_rseg_array_t::rseg_array_header_t_t::rseg_array_header_t_t(kaitai::kstream* p__io, innodb_page_rseg_array_t* p__parent, innodb_page_rseg_array_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_rseg_array_t::rseg_array_header_t_t::_read() {
    m_max_rollback_segments = m__io->read_u4le();
    m_rseg_array_size = m__io->read_u4le();
    m_rseg_array_version = m__io->read_u4le();
}

innodb_page_rseg_array_t::rseg_array_header_t_t::~rseg_array_header_t_t() {
    _clean_up();
}

void innodb_page_rseg_array_t::rseg_array_header_t_t::_clean_up() {
}
