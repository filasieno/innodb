// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_zlob_frag_entry.h"

innodb_page_zlob_frag_entry_t::innodb_page_zlob_frag_entry_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_zlob_frag_entry_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_frag_entry_header = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_zlob_frag_entry_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_frag_entry_header = new frag_entry_header_t_t(m__io, this, m__root);
    m_frag_entries = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_zlob_frag_entry_t::~innodb_page_zlob_frag_entry_t() {
    _clean_up();
}

void innodb_page_zlob_frag_entry_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_frag_entry_header) {
        delete m_frag_entry_header; m_frag_entry_header = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_zlob_frag_entry_t::frag_entry_header_t_t::frag_entry_header_t_t(kaitai::kstream* p__io, innodb_page_zlob_frag_entry_t* p__parent, innodb_page_zlob_frag_entry_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_zlob_frag_entry_t::frag_entry_header_t_t::_read() {
    m_n_entries = m__io->read_u4le();
    m_used_len = m__io->read_u4le();
    m_trx_id = m__io->read_u8le();
}

innodb_page_zlob_frag_entry_t::frag_entry_header_t_t::~frag_entry_header_t_t() {
    _clean_up();
}

void innodb_page_zlob_frag_entry_t::frag_entry_header_t_t::_clean_up() {
}
