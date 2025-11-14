// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_fsp_hdr.h"
std::set<innodb_page_fsp_hdr_t::xdes_state_enum_t> innodb_page_fsp_hdr_t::_build_values_xdes_state_enum_t() {
    std::set<innodb_page_fsp_hdr_t::xdes_state_enum_t> _t;
    _t.insert(innodb_page_fsp_hdr_t::XDES_STATE_ENUM_FREE);
    _t.insert(innodb_page_fsp_hdr_t::XDES_STATE_ENUM_FREE_FRAG);
    _t.insert(innodb_page_fsp_hdr_t::XDES_STATE_ENUM_FULL_FRAG);
    _t.insert(innodb_page_fsp_hdr_t::XDES_STATE_ENUM_FSEG);
    _t.insert(innodb_page_fsp_hdr_t::XDES_STATE_ENUM_FSEG_FRAG);
    return _t;
}
const std::set<innodb_page_fsp_hdr_t::xdes_state_enum_t> innodb_page_fsp_hdr_t::_values_xdes_state_enum_t = innodb_page_fsp_hdr_t::_build_values_xdes_state_enum_t();
bool innodb_page_fsp_hdr_t::_is_defined_xdes_state_enum_t(innodb_page_fsp_hdr_t::xdes_state_enum_t v) {
    return innodb_page_fsp_hdr_t::_values_xdes_state_enum_t.find(v) != innodb_page_fsp_hdr_t::_values_xdes_state_enum_t.end();
}

innodb_page_fsp_hdr_t::innodb_page_fsp_hdr_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_fsp_hdr_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_fsp_header = 0;
    m_xdes_array = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_fsp_hdr_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_fsp_header = new fsp_header_t_t(m__io, this, m__root);
    m_xdes_array = new std::vector<xdes_entry_t_t*>();
    const int l_xdes_array = 256;
    for (int i = 0; i < l_xdes_array; i++) {
        m_xdes_array->push_back(new xdes_entry_t_t(m__io, this, m__root));
    }
    m_empty_space = m__io->read_bytes_full();
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_fsp_hdr_t::~innodb_page_fsp_hdr_t() {
    _clean_up();
}

void innodb_page_fsp_hdr_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_fsp_header) {
        delete m_fsp_header; m_fsp_header = 0;
    }
    if (m_xdes_array) {
        for (std::vector<xdes_entry_t_t*>::iterator it = m_xdes_array->begin(); it != m_xdes_array->end(); ++it) {
            delete *it;
        }
        delete m_xdes_array; m_xdes_array = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_fsp_hdr_t::fil_addr_t_t::fil_addr_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_fsp_hdr_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    f_is_null = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_fsp_hdr_t::fil_addr_t_t::_read() {
    m_page_no = m__io->read_u4le();
    m_byte_offset = m__io->read_u2le();
}

innodb_page_fsp_hdr_t::fil_addr_t_t::~fil_addr_t_t() {
    _clean_up();
}

void innodb_page_fsp_hdr_t::fil_addr_t_t::_clean_up() {
}

bool innodb_page_fsp_hdr_t::fil_addr_t_t::is_null() {
    if (f_is_null)
        return m_is_null;
    f_is_null = true;
    m_is_null = page_no() == 4294967295UL;
    return m_is_null;
}

innodb_page_fsp_hdr_t::flst_base_node_t_t::flst_base_node_t_t(kaitai::kstream* p__io, innodb_page_fsp_hdr_t::fsp_header_t_t* p__parent, innodb_page_fsp_hdr_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_first_node = 0;
    m_last_node = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_fsp_hdr_t::flst_base_node_t_t::_read() {
    m_length = m__io->read_u4le();
    m_first_node = new fil_addr_t_t(m__io, this, m__root);
    m_last_node = new fil_addr_t_t(m__io, this, m__root);
}

innodb_page_fsp_hdr_t::flst_base_node_t_t::~flst_base_node_t_t() {
    _clean_up();
}

void innodb_page_fsp_hdr_t::flst_base_node_t_t::_clean_up() {
    if (m_first_node) {
        delete m_first_node; m_first_node = 0;
    }
    if (m_last_node) {
        delete m_last_node; m_last_node = 0;
    }
}

innodb_page_fsp_hdr_t::flst_node_t_t::flst_node_t_t(kaitai::kstream* p__io, innodb_page_fsp_hdr_t::xdes_entry_t_t* p__parent, innodb_page_fsp_hdr_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_prev = 0;
    m_next = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_fsp_hdr_t::flst_node_t_t::_read() {
    m_prev = new fil_addr_t_t(m__io, this, m__root);
    m_next = new fil_addr_t_t(m__io, this, m__root);
}

innodb_page_fsp_hdr_t::flst_node_t_t::~flst_node_t_t() {
    _clean_up();
}

void innodb_page_fsp_hdr_t::flst_node_t_t::_clean_up() {
    if (m_prev) {
        delete m_prev; m_prev = 0;
    }
    if (m_next) {
        delete m_next; m_next = 0;
    }
}

innodb_page_fsp_hdr_t::fsp_header_t_t::fsp_header_t_t(kaitai::kstream* p__io, innodb_page_fsp_hdr_t* p__parent, innodb_page_fsp_hdr_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_space_flags = 0;
    m_free_list = 0;
    m_free_frag_list = 0;
    m_full_frag_list = 0;
    m_full_inodes_list = 0;
    m_free_inodes_list = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_fsp_hdr_t::fsp_header_t_t::_read() {
    m_space_id = m__io->read_u4le();
    m_unused = m__io->read_u4le();
    m_size = m__io->read_u4le();
    m_free_limit = m__io->read_u4le();
    m_space_flags = new innodb_common_t::space_flags_t_t(m__io);
    m_frag_n_used = m__io->read_u4le();
    m_free_list = new flst_base_node_t_t(m__io, this, m__root);
    m_free_frag_list = new flst_base_node_t_t(m__io, this, m__root);
    m_full_frag_list = new flst_base_node_t_t(m__io, this, m__root);
    m_next_unused_seg_id = m__io->read_u8le();
    m_full_inodes_list = new flst_base_node_t_t(m__io, this, m__root);
    m_free_inodes_list = new flst_base_node_t_t(m__io, this, m__root);
}

innodb_page_fsp_hdr_t::fsp_header_t_t::~fsp_header_t_t() {
    _clean_up();
}

void innodb_page_fsp_hdr_t::fsp_header_t_t::_clean_up() {
    if (m_space_flags) {
        delete m_space_flags; m_space_flags = 0;
    }
    if (m_free_list) {
        delete m_free_list; m_free_list = 0;
    }
    if (m_free_frag_list) {
        delete m_free_frag_list; m_free_frag_list = 0;
    }
    if (m_full_frag_list) {
        delete m_full_frag_list; m_full_frag_list = 0;
    }
    if (m_full_inodes_list) {
        delete m_full_inodes_list; m_full_inodes_list = 0;
    }
    if (m_free_inodes_list) {
        delete m_free_inodes_list; m_free_inodes_list = 0;
    }
}

innodb_page_fsp_hdr_t::xdes_entry_t_t::xdes_entry_t_t(kaitai::kstream* p__io, innodb_page_fsp_hdr_t* p__parent, innodb_page_fsp_hdr_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_list_node = 0;
    f_is_free = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_fsp_hdr_t::xdes_entry_t_t::_read() {
    m_file_segment_id = m__io->read_u8le();
    m_list_node = new flst_node_t_t(m__io, this, m__root);
    m_state = static_cast<innodb_page_fsp_hdr_t::xdes_state_enum_t>(m__io->read_u4le());
    m_page_state_bitmap = m__io->read_bytes(16);
}

innodb_page_fsp_hdr_t::xdes_entry_t_t::~xdes_entry_t_t() {
    _clean_up();
}

void innodb_page_fsp_hdr_t::xdes_entry_t_t::_clean_up() {
    if (m_list_node) {
        delete m_list_node; m_list_node = 0;
    }
}

bool innodb_page_fsp_hdr_t::xdes_entry_t_t::is_free() {
    if (f_is_free)
        return m_is_free;
    f_is_free = true;
    m_is_free = file_segment_id() == 0;
    return m_is_free;
}
