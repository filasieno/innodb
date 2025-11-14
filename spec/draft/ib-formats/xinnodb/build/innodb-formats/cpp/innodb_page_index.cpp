// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_index.h"
std::set<innodb_page_index_t::insert_direction_enum_t> innodb_page_index_t::_build_values_insert_direction_enum_t() {
    std::set<innodb_page_index_t::insert_direction_enum_t> _t;
    _t.insert(innodb_page_index_t::INSERT_DIRECTION_ENUM_LEFT);
    _t.insert(innodb_page_index_t::INSERT_DIRECTION_ENUM_RIGHT);
    _t.insert(innodb_page_index_t::INSERT_DIRECTION_ENUM_SAME_REC);
    _t.insert(innodb_page_index_t::INSERT_DIRECTION_ENUM_SAME_PAGE);
    _t.insert(innodb_page_index_t::INSERT_DIRECTION_ENUM_NO_DIRECTION);
    return _t;
}
const std::set<innodb_page_index_t::insert_direction_enum_t> innodb_page_index_t::_values_insert_direction_enum_t = innodb_page_index_t::_build_values_insert_direction_enum_t();
bool innodb_page_index_t::_is_defined_insert_direction_enum_t(innodb_page_index_t::insert_direction_enum_t v) {
    return innodb_page_index_t::_values_insert_direction_enum_t.find(v) != innodb_page_index_t::_values_insert_direction_enum_t.end();
}

innodb_page_index_t::innodb_page_index_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_index_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_index_header = 0;
    m_fseg_header = 0;
    m_system_records = 0;
    m_page_directory = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_index_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_index_header = new index_header_t_t(m__io, this, m__root);
    m_fseg_header = new fseg_header_t_t(m__io, this, m__root);
    m_system_records = new system_records_t_t(m__io, this, m__root);
    m_user_records_and_free_space = m__io->read_bytes_full();
    m_page_directory = new std::vector<uint16_t>();
    const int l_page_directory = index_header()->n_dir_slots();
    for (int i = 0; i < l_page_directory; i++) {
        m_page_directory->push_back(m__io->read_u2le());
    }
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_index_t::~innodb_page_index_t() {
    _clean_up();
}

void innodb_page_index_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_index_header) {
        delete m_index_header; m_index_header = 0;
    }
    if (m_fseg_header) {
        delete m_fseg_header; m_fseg_header = 0;
    }
    if (m_system_records) {
        delete m_system_records; m_system_records = 0;
    }
    if (m_page_directory) {
        delete m_page_directory; m_page_directory = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_index_t::fseg_header_t_t::fseg_header_t_t(kaitai::kstream* p__io, innodb_page_index_t* p__parent, innodb_page_index_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_index_t::fseg_header_t_t::_read() {
    m_leaf_inode_space = m__io->read_u4le();
    m_leaf_inode_page_no = m__io->read_u4le();
    m_leaf_inode_offset = m__io->read_u2le();
    m_internal_inode_space = m__io->read_u4le();
    m_internal_inode_page_no = m__io->read_u4le();
    m_internal_inode_offset = m__io->read_u2le();
}

innodb_page_index_t::fseg_header_t_t::~fseg_header_t_t() {
    _clean_up();
}

void innodb_page_index_t::fseg_header_t_t::_clean_up() {
}

innodb_page_index_t::index_header_t_t::index_header_t_t(kaitai::kstream* p__io, innodb_page_index_t* p__parent, innodb_page_index_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    f_actual_n_heap = false;
    f_is_compact = false;
    f_is_leaf = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_index_t::index_header_t_t::_read() {
    m_n_dir_slots = m__io->read_u2le();
    m_heap_top = m__io->read_u2le();
    m_n_heap = m__io->read_u2le();
    m_free_offset = m__io->read_u2le();
    m_garbage_bytes = m__io->read_u2le();
    m_last_insert_offset = m__io->read_u2le();
    m_direction = static_cast<innodb_page_index_t::insert_direction_enum_t>(m__io->read_u2le());
    m_n_direction = m__io->read_u2le();
    m_n_recs = m__io->read_u2le();
    m_max_trx_id = m__io->read_u8le();
    m_level = m__io->read_u2le();
    m_index_id = m__io->read_u8le();
}

innodb_page_index_t::index_header_t_t::~index_header_t_t() {
    _clean_up();
}

void innodb_page_index_t::index_header_t_t::_clean_up() {
}

int32_t innodb_page_index_t::index_header_t_t::actual_n_heap() {
    if (f_actual_n_heap)
        return m_actual_n_heap;
    f_actual_n_heap = true;
    m_actual_n_heap = n_heap() & 32767;
    return m_actual_n_heap;
}

bool innodb_page_index_t::index_header_t_t::is_compact() {
    if (f_is_compact)
        return m_is_compact;
    f_is_compact = true;
    m_is_compact = (n_heap() & 32768) != 0;
    return m_is_compact;
}

bool innodb_page_index_t::index_header_t_t::is_leaf() {
    if (f_is_leaf)
        return m_is_leaf;
    f_is_leaf = true;
    m_is_leaf = level() == 0;
    return m_is_leaf;
}

innodb_page_index_t::infimum_supremum_record_t_t::infimum_supremum_record_t_t(kaitai::kstream* p__io, innodb_page_index_t::system_records_t_t* p__parent, innodb_page_index_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_index_t::infimum_supremum_record_t_t::_read() {
    m_record_header = m__io->read_bytes(5);
    m_data = m__io->read_bytes(8);
}

innodb_page_index_t::infimum_supremum_record_t_t::~infimum_supremum_record_t_t() {
    _clean_up();
}

void innodb_page_index_t::infimum_supremum_record_t_t::_clean_up() {
}

innodb_page_index_t::system_records_t_t::system_records_t_t(kaitai::kstream* p__io, innodb_page_index_t* p__parent, innodb_page_index_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_infimum = 0;
    m_supremum = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_index_t::system_records_t_t::_read() {
    m_infimum = new infimum_supremum_record_t_t(m__io, this, m__root);
    m_supremum = new infimum_supremum_record_t_t(m__io, this, m__root);
}

innodb_page_index_t::system_records_t_t::~system_records_t_t() {
    _clean_up();
}

void innodb_page_index_t::system_records_t_t::_clean_up() {
    if (m_infimum) {
        delete m_infimum; m_infimum = 0;
    }
    if (m_supremum) {
        delete m_supremum; m_supremum = 0;
    }
}
