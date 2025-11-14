// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_blob.h"

innodb_page_blob_t::innodb_page_blob_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_blob_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_blob_header = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_blob_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_blob_header = new blob_header_t_t(m__io, this, m__root);
    m_blob_data = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_blob_t::~innodb_page_blob_t() {
    _clean_up();
}

void innodb_page_blob_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_blob_header) {
        delete m_blob_header; m_blob_header = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_blob_t::blob_header_t_t::blob_header_t_t(kaitai::kstream* p__io, innodb_page_blob_t* p__parent, innodb_page_blob_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    f_has_next = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_blob_t::blob_header_t_t::_read() {
    m_blob_part_len = m__io->read_u4le();
    m_next_page_no = m__io->read_u4le();
}

innodb_page_blob_t::blob_header_t_t::~blob_header_t_t() {
    _clean_up();
}

void innodb_page_blob_t::blob_header_t_t::_clean_up() {
}

bool innodb_page_blob_t::blob_header_t_t::has_next() {
    if (f_has_next)
        return m_has_next;
    f_has_next = true;
    m_has_next = next_page_no() != 4294967295UL;
    return m_has_next;
}
