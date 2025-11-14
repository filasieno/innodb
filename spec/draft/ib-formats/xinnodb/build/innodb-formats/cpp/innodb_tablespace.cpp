// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_tablespace.h"

innodb_tablespace_t::innodb_tablespace_t(uint32_t p_page_size, kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_tablespace_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_page_size = p_page_size;
    m_pages = 0;
    f_actual_page_size = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_tablespace_t::_read() {
    m_pages = new std::vector<page_wrapper_t_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_pages->push_back(new page_wrapper_t_t(m__io, this, m__root));
            i++;
        }
    }
}

innodb_tablespace_t::~innodb_tablespace_t() {
    _clean_up();
}

void innodb_tablespace_t::_clean_up() {
    if (m_pages) {
        for (std::vector<page_wrapper_t_t*>::iterator it = m_pages->begin(); it != m_pages->end(); ++it) {
            delete *it;
        }
        delete m_pages; m_pages = 0;
    }
}

innodb_tablespace_t::page_dispatcher_t_t::page_dispatcher_t_t(kaitai::kstream* p__io, innodb_tablespace_t::page_wrapper_t_t* p__parent, innodb_tablespace_t* p__root) : kaitai::kstruct(p__io) {
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

void innodb_tablespace_t::page_dispatcher_t_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    n_page_body = true;
    switch (fil_header()->page_type()) {
    case innodb_common_t::PAGE_TYPE_ENUM_ALLOCATED: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_allocated_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_BLOB: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_blob_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_FSP_HDR: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_fsp_hdr_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_IBUF_BITMAP: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_ibuf_bitmap_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_IBUF_FREE_LIST: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_ibuf_free_list_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_INDEX: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_index_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_INODE: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_inode_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_LOB_DATA: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_lob_data_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_LOB_FIRST: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_lob_first_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_LOB_INDEX: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_lob_index_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_RTREE: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_rtree_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_SDI_BLOB: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_sdi_blob_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_SDI_ZBLOB: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_sdi_zblob_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_SYS: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_sys_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_TRX_SYS: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_trx_sys_t(m__io__raw_page_body);
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
    case innodb_common_t::PAGE_TYPE_ENUM_ZBLOB: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_zblob_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_ZBLOB2: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_zblob2_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_ZLOB_DATA_V2: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_zlob_data_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_ZLOB_FIRST_V2: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_zlob_first_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_ZLOB_FRAG: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_zlob_frag_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_ZLOB_FRAG_ENTRY: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_zlob_frag_entry_t(m__io__raw_page_body);
        break;
    }
    case innodb_common_t::PAGE_TYPE_ENUM_ZLOB_INDEX_V2: {
        n_page_body = false;
        m__raw_page_body = m__io->read_bytes_full();
        m__io__raw_page_body = new kaitai::kstream(m__raw_page_body);
        m_page_body = new innodb_page_zlob_index_t(m__io__raw_page_body);
        break;
    }
    default: {
        m__raw_page_body = m__io->read_bytes_full();
        break;
    }
    }
}

innodb_tablespace_t::page_dispatcher_t_t::~page_dispatcher_t_t() {
    _clean_up();
}

void innodb_tablespace_t::page_dispatcher_t_t::_clean_up() {
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

innodb_tablespace_t::page_wrapper_t_t::page_wrapper_t_t(kaitai::kstream* p__io, innodb_tablespace_t* p__parent, innodb_tablespace_t* p__root) : kaitai::kstruct(p__io) {
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

void innodb_tablespace_t::page_wrapper_t_t::_read() {
    m__raw_page_data = m__io->read_bytes(_root()->page_size());
    m__io__raw_page_data = new kaitai::kstream(m__raw_page_data);
    m_page_data = new page_dispatcher_t_t(m__io__raw_page_data, this, m__root);
}

innodb_tablespace_t::page_wrapper_t_t::~page_wrapper_t_t() {
    _clean_up();
}

void innodb_tablespace_t::page_wrapper_t_t::_clean_up() {
    if (m__io__raw_page_data) {
        delete m__io__raw_page_data; m__io__raw_page_data = 0;
    }
    if (m_page_data) {
        delete m_page_data; m_page_data = 0;
    }
}

uint32_t innodb_tablespace_t::actual_page_size() {
    if (f_actual_page_size)
        return m_actual_page_size;
    f_actual_page_size = true;
    m_actual_page_size = _root()->page_size();
    return m_actual_page_size;
}
