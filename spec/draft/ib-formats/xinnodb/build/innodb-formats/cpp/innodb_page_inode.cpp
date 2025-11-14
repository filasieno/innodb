// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_inode.h"

innodb_page_inode_t::innodb_page_inode_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_inode_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_list_node = 0;
    m_inodes = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_inode_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_list_node = new innodb_page_fsp_hdr_t::flst_node_t_t(m__io);
    m_inodes = new std::vector<fseg_inode_t_t*>();
    const int l_inodes = 85;
    for (int i = 0; i < l_inodes; i++) {
        m_inodes->push_back(new fseg_inode_t_t(m__io, this, m__root));
    }
    m_empty_space = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_inode_t::~innodb_page_inode_t() {
    _clean_up();
}

void innodb_page_inode_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_list_node) {
        delete m_list_node; m_list_node = 0;
    }
    if (m_inodes) {
        for (std::vector<fseg_inode_t_t*>::iterator it = m_inodes->begin(); it != m_inodes->end(); ++it) {
            delete *it;
        }
        delete m_inodes; m_inodes = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_inode_t::fseg_inode_t_t::fseg_inode_t_t(kaitai::kstream* p__io, innodb_page_inode_t* p__parent, innodb_page_inode_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_free_list = 0;
    m_not_full_list = 0;
    m_full_list = 0;
    m_frag_arr = 0;
    f_is_used = false;
    f_magic_valid = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_inode_t::fseg_inode_t_t::_read() {
    m_fseg_id = m__io->read_u8le();
    m_not_full_n_used = m__io->read_u4le();
    m_free_list = new innodb_page_fsp_hdr_t::flst_base_node_t_t(m__io);
    m_not_full_list = new innodb_page_fsp_hdr_t::flst_base_node_t_t(m__io);
    m_full_list = new innodb_page_fsp_hdr_t::flst_base_node_t_t(m__io);
    m_magic_n = m__io->read_u4le();
    m_frag_arr = new std::vector<uint32_t>();
    const int l_frag_arr = 32;
    for (int i = 0; i < l_frag_arr; i++) {
        m_frag_arr->push_back(m__io->read_u4le());
    }
}

innodb_page_inode_t::fseg_inode_t_t::~fseg_inode_t_t() {
    _clean_up();
}

void innodb_page_inode_t::fseg_inode_t_t::_clean_up() {
    if (m_free_list) {
        delete m_free_list; m_free_list = 0;
    }
    if (m_not_full_list) {
        delete m_not_full_list; m_not_full_list = 0;
    }
    if (m_full_list) {
        delete m_full_list; m_full_list = 0;
    }
    if (m_frag_arr) {
        delete m_frag_arr; m_frag_arr = 0;
    }
}

bool innodb_page_inode_t::fseg_inode_t_t::is_used() {
    if (f_is_used)
        return m_is_used;
    f_is_used = true;
    m_is_used = fseg_id() != 0;
    return m_is_used;
}

bool innodb_page_inode_t::fseg_inode_t_t::magic_valid() {
    if (f_magic_valid)
        return m_magic_valid;
    f_magic_valid = true;
    m_magic_valid = magic_n() == 97937874;
    return m_magic_valid;
}
