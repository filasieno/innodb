// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_rtree.h"

innodb_page_rtree_t::innodb_page_rtree_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_rtree_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_index_header = 0;
    m_rtree_header = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_rtree_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_index_header = new innodb_page_index_t::index_header_t_t(m__io);
    m_rtree_header = new rtree_header_t_t(m__io, this, m__root);
    m_mbr_data = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_rtree_t::~innodb_page_rtree_t() {
    _clean_up();
}

void innodb_page_rtree_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_index_header) {
        delete m_index_header; m_index_header = 0;
    }
    if (m_rtree_header) {
        delete m_rtree_header; m_rtree_header = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_rtree_t::mbr_t_t::mbr_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_rtree_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_rtree_t::mbr_t_t::_read() {
    m_xmin = m__io->read_f8le();
    m_xmax = m__io->read_f8le();
    m_ymin = m__io->read_f8le();
    m_ymax = m__io->read_f8le();
}

innodb_page_rtree_t::mbr_t_t::~mbr_t_t() {
    _clean_up();
}

void innodb_page_rtree_t::mbr_t_t::_clean_up() {
}

innodb_page_rtree_t::rtree_header_t_t::rtree_header_t_t(kaitai::kstream* p__io, innodb_page_rtree_t* p__parent, innodb_page_rtree_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_rtree_t::rtree_header_t_t::_read() {
    m_mbr_count = m__io->read_u2le();
    m_level = m__io->read_u2le();
    m_reserved = m__io->read_bytes(4);
}

innodb_page_rtree_t::rtree_header_t_t::~rtree_header_t_t() {
    _clean_up();
}

void innodb_page_rtree_t::rtree_header_t_t::_clean_up() {
}
