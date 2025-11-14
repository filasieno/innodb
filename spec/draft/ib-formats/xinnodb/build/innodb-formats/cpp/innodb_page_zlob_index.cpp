// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_zlob_index.h"

innodb_page_zlob_index_t::innodb_page_zlob_index_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_zlob_index_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_zlob_index_header = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_zlob_index_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_zlob_index_header = new zlob_index_header_t_t(m__io, this, m__root);
    m_index_entries = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_zlob_index_t::~innodb_page_zlob_index_t() {
    _clean_up();
}

void innodb_page_zlob_index_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_zlob_index_header) {
        delete m_zlob_index_header; m_zlob_index_header = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_zlob_index_t::zlob_index_header_t_t::zlob_index_header_t_t(kaitai::kstream* p__io, innodb_page_zlob_index_t* p__parent, innodb_page_zlob_index_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_zlob_index_t::zlob_index_header_t_t::_read() {
    m_lob_version = m__io->read_u1();
    m_flags = m__io->read_u1();
    m_reserved = m__io->read_bytes(2);
    m_total_compressed_len = m__io->read_u8le();
    m_total_uncompressed_len = m__io->read_u8le();
}

innodb_page_zlob_index_t::zlob_index_header_t_t::~zlob_index_header_t_t() {
    _clean_up();
}

void innodb_page_zlob_index_t::zlob_index_header_t_t::_clean_up() {
}
