// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_trx_sys.h"

innodb_page_trx_sys_t::innodb_page_trx_sys_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_trx_sys_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_trx_sys_header = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_trx_sys_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_trx_sys_header = new trx_sys_header_t_t(m__io, this, m__root);
    m_empty_space = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_trx_sys_t::~innodb_page_trx_sys_t() {
    _clean_up();
}

void innodb_page_trx_sys_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_trx_sys_header) {
        delete m_trx_sys_header; m_trx_sys_header = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_trx_sys_t::binlog_info_t_t::binlog_info_t_t(kaitai::kstream* p__io, innodb_page_trx_sys_t::trx_sys_header_t_t* p__parent, innodb_page_trx_sys_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_trx_sys_t::binlog_info_t_t::_read() {
    m_binlog_file_name_len = m__io->read_u4le();
    m_binlog_file_name = kaitai::kstream::bytes_to_str(m__io->read_bytes(512), "ASCII");
    m_binlog_offset = m__io->read_u8le();
}

innodb_page_trx_sys_t::binlog_info_t_t::~binlog_info_t_t() {
    _clean_up();
}

void innodb_page_trx_sys_t::binlog_info_t_t::_clean_up() {
}

innodb_page_trx_sys_t::trx_sys_header_t_t::trx_sys_header_t_t(kaitai::kstream* p__io, innodb_page_trx_sys_t* p__parent, innodb_page_trx_sys_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_doublewrite_block1 = 0;
    m_doublewrite_block2 = 0;
    m_binlog_info = 0;
    m_rseg_array = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_trx_sys_t::trx_sys_header_t_t::_read() {
    m_trx_sys_magic = m__io->read_u4le();
    m_trx_id_high = m__io->read_u8le();
    m_doublewrite_magic = m__io->read_u4le();
    m_doublewrite_block1 = new innodb_page_fsp_hdr_t::fil_addr_t_t(m__io);
    m_doublewrite_block2 = new innodb_page_fsp_hdr_t::fil_addr_t_t(m__io);
    m_doublewrite_fseg_header = m__io->read_bytes(10);
    m_binlog_info = new binlog_info_t_t(m__io, this, m__root);
    m_rseg_array = new std::vector<uint32_t>();
    const int l_rseg_array = 128;
    for (int i = 0; i < l_rseg_array; i++) {
        m_rseg_array->push_back(m__io->read_u4le());
    }
}

innodb_page_trx_sys_t::trx_sys_header_t_t::~trx_sys_header_t_t() {
    _clean_up();
}

void innodb_page_trx_sys_t::trx_sys_header_t_t::_clean_up() {
    if (m_doublewrite_block1) {
        delete m_doublewrite_block1; m_doublewrite_block1 = 0;
    }
    if (m_doublewrite_block2) {
        delete m_doublewrite_block2; m_doublewrite_block2 = 0;
    }
    if (m_binlog_info) {
        delete m_binlog_info; m_binlog_info = 0;
    }
    if (m_rseg_array) {
        delete m_rseg_array; m_rseg_array = 0;
    }
}
