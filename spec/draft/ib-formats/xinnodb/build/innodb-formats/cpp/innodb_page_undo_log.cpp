// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_page_undo_log.h"
std::set<innodb_page_undo_log_t::undo_page_type_enum_t> innodb_page_undo_log_t::_build_values_undo_page_type_enum_t() {
    std::set<innodb_page_undo_log_t::undo_page_type_enum_t> _t;
    _t.insert(innodb_page_undo_log_t::UNDO_PAGE_TYPE_ENUM_INSERT);
    _t.insert(innodb_page_undo_log_t::UNDO_PAGE_TYPE_ENUM_UPDATE);
    return _t;
}
const std::set<innodb_page_undo_log_t::undo_page_type_enum_t> innodb_page_undo_log_t::_values_undo_page_type_enum_t = innodb_page_undo_log_t::_build_values_undo_page_type_enum_t();
bool innodb_page_undo_log_t::_is_defined_undo_page_type_enum_t(innodb_page_undo_log_t::undo_page_type_enum_t v) {
    return innodb_page_undo_log_t::_values_undo_page_type_enum_t.find(v) != innodb_page_undo_log_t::_values_undo_page_type_enum_t.end();
}
std::set<innodb_page_undo_log_t::undo_record_type_enum_t> innodb_page_undo_log_t::_build_values_undo_record_type_enum_t() {
    std::set<innodb_page_undo_log_t::undo_record_type_enum_t> _t;
    _t.insert(innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_INSERT);
    _t.insert(innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_UPDATE_EXISTING);
    _t.insert(innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_UPDATE_DELETED);
    _t.insert(innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_DELETE);
    _t.insert(innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_PURGE);
    _t.insert(innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_INSERT_TRUNCATE);
    _t.insert(innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_UPDATE_TRUNCATE);
    return _t;
}
const std::set<innodb_page_undo_log_t::undo_record_type_enum_t> innodb_page_undo_log_t::_values_undo_record_type_enum_t = innodb_page_undo_log_t::_build_values_undo_record_type_enum_t();
bool innodb_page_undo_log_t::_is_defined_undo_record_type_enum_t(innodb_page_undo_log_t::undo_record_type_enum_t v) {
    return innodb_page_undo_log_t::_values_undo_record_type_enum_t.find(v) != innodb_page_undo_log_t::_values_undo_record_type_enum_t.end();
}

innodb_page_undo_log_t::innodb_page_undo_log_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_fil_header = 0;
    m_undo_page_header = 0;
    m_undo_records = 0;
    m_fil_trailer = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::_read() {
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m_undo_page_header = new undo_page_header_t_t(m__io, this, m__root);
    m_undo_records = new undo_record_list_t_t(m__io, this, m__root);
    m_fil_trailer = new innodb_common_t::fil_trailer_t_t(m__io);
}

innodb_page_undo_log_t::~innodb_page_undo_log_t() {
    _clean_up();
}

void innodb_page_undo_log_t::_clean_up() {
    if (m_fil_header) {
        delete m_fil_header; m_fil_header = 0;
    }
    if (m_undo_page_header) {
        delete m_undo_page_header; m_undo_page_header = 0;
    }
    if (m_undo_records) {
        delete m_undo_records; m_undo_records = 0;
    }
    if (m_fil_trailer) {
        delete m_fil_trailer; m_fil_trailer = 0;
    }
}

innodb_page_undo_log_t::undo_delete_data_t_t::undo_delete_data_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_t_t* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_record_header = 0;
    m_field_data = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_delete_data_t_t::_read() {
    m_record_header = new undo_record_header_t_t(m__io, this, m__root);
    m_null_bitmap = m__io->read_bytes((record_header()->num_fields() + 7) / 8);
    m_field_data = new std::vector<undo_field_data_t_t*>();
    {
        int i = 0;
        undo_field_data_t_t* _;
        do {
            _ = new undo_field_data_t_t(m__io, this, m__root);
            m_field_data->push_back(_);
            i++;
        } while (!(_io()->pos() >= _io()->size() - 2));
    }
}

innodb_page_undo_log_t::undo_delete_data_t_t::~undo_delete_data_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_delete_data_t_t::_clean_up() {
    if (m_record_header) {
        delete m_record_header; m_record_header = 0;
    }
    if (m_field_data) {
        for (std::vector<undo_field_data_t_t*>::iterator it = m_field_data->begin(); it != m_field_data->end(); ++it) {
            delete *it;
        }
        delete m_field_data; m_field_data = 0;
    }
}

innodb_page_undo_log_t::undo_field_data_t_t::undo_field_data_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_field_data_t_t::_read() {
    m_len_field_value = m__io->read_u4le();
    m_field_value = m__io->read_bytes(len_field_value());
}

innodb_page_undo_log_t::undo_field_data_t_t::~undo_field_data_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_field_data_t_t::_clean_up() {
}

innodb_page_undo_log_t::undo_insert_data_t_t::undo_insert_data_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_t_t* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_primary_key_fields = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_insert_data_t_t::_read() {
    m_primary_key_fields = new std::vector<undo_field_data_t_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_primary_key_fields->push_back(new undo_field_data_t_t(m__io, this, m__root));
            i++;
        }
    }
}

innodb_page_undo_log_t::undo_insert_data_t_t::~undo_insert_data_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_insert_data_t_t::_clean_up() {
    if (m_primary_key_fields) {
        for (std::vector<undo_field_data_t_t*>::iterator it = m_primary_key_fields->begin(); it != m_primary_key_fields->end(); ++it) {
            delete *it;
        }
        delete m_primary_key_fields; m_primary_key_fields = 0;
    }
}

innodb_page_undo_log_t::undo_page_header_t_t::undo_page_header_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_page_header_t_t::_read() {
    m_page_type = static_cast<innodb_page_undo_log_t::undo_page_type_enum_t>(m__io->read_u2le());
    m_latest_log_record_offset = m__io->read_u2le();
    m_free_offset = m__io->read_u2le();
    m_page_list_node = m__io->read_bytes(12);
}

innodb_page_undo_log_t::undo_page_header_t_t::~undo_page_header_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_page_header_t_t::_clean_up() {
}

innodb_page_undo_log_t::undo_record_header_t_t::undo_record_header_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_delete_data_t_t* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_record_header_t_t::_read() {
    m_info_flags = m__io->read_u1();
    m_num_fields = m__io->read_u4le();
}

innodb_page_undo_log_t::undo_record_header_t_t::~undo_record_header_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_record_header_t_t::_clean_up() {
}

innodb_page_undo_log_t::undo_record_list_t_t::undo_record_list_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_first_record = 0;
    f_first_record = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_record_list_t_t::_read() {
}

innodb_page_undo_log_t::undo_record_list_t_t::~undo_record_list_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_record_list_t_t::_clean_up() {
    if (f_first_record && !n_first_record) {
        if (m_first_record) {
            delete m_first_record; m_first_record = 0;
        }
    }
}

innodb_page_undo_log_t::undo_record_with_next_t_t* innodb_page_undo_log_t::undo_record_list_t_t::first_record() {
    if (f_first_record)
        return m_first_record;
    f_first_record = true;
    n_first_record = true;
    if (_parent()->undo_page_header()->latest_log_record_offset() != 0) {
        n_first_record = false;
        std::streampos _pos = m__io->pos();
        m__io->seek(_parent()->undo_page_header()->latest_log_record_offset());
        m_first_record = new undo_record_with_next_t_t(m__io, this, m__root);
        m__io->seek(_pos);
    }
    return m_first_record;
}

innodb_page_undo_log_t::undo_record_t_t::undo_record_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_with_next_t_t* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_undo_no = 0;
    m_table_id = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_record_t_t::_read() {
    m_undo_rec_type = static_cast<innodb_page_undo_log_t::undo_record_type_enum_t>(m__io->read_u1());
    m_undo_no = new innodb_common_t::mach_compressed_uint_t_t(m__io);
    m_table_id = new innodb_common_t::mach_compressed_uint_t_t(m__io);
    m_info_bits = m__io->read_u1();
    m_trx_id = m__io->read_u8le();
    m_roll_ptr = m__io->read_u8le();
    n_data = true;
    switch (undo_rec_type()) {
    case innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_DELETE: {
        n_data = false;
        m_data = new undo_delete_data_t_t(m__io, this, m__root);
        break;
    }
    case innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_INSERT: {
        n_data = false;
        m_data = new undo_insert_data_t_t(m__io, this, m__root);
        break;
    }
    case innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_INSERT_TRUNCATE: {
        n_data = false;
        m_data = new undo_truncate_data_t_t(m__io, this, m__root);
        break;
    }
    case innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_PURGE: {
        n_data = false;
        m_data = new undo_delete_data_t_t(m__io, this, m__root);
        break;
    }
    case innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_UPDATE_DELETED: {
        n_data = false;
        m_data = new undo_update_data_t_t(m__io, this, m__root);
        break;
    }
    case innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_UPDATE_EXISTING: {
        n_data = false;
        m_data = new undo_update_data_t_t(m__io, this, m__root);
        break;
    }
    case innodb_page_undo_log_t::UNDO_RECORD_TYPE_ENUM_UPDATE_TRUNCATE: {
        n_data = false;
        m_data = new undo_truncate_data_t_t(m__io, this, m__root);
        break;
    }
    }
    m_next_record_offset = m__io->read_u2le();
}

innodb_page_undo_log_t::undo_record_t_t::~undo_record_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_record_t_t::_clean_up() {
    if (m_undo_no) {
        delete m_undo_no; m_undo_no = 0;
    }
    if (m_table_id) {
        delete m_table_id; m_table_id = 0;
    }
    if (!n_data) {
        if (m_data) {
            delete m_data; m_data = 0;
        }
    }
}

innodb_page_undo_log_t::undo_record_with_next_t_t::undo_record_with_next_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_record = 0;
    m_next_record = 0;
    f_next_record = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_record_with_next_t_t::_read() {
    m_record = new undo_record_t_t(m__io, this, m__root);
}

innodb_page_undo_log_t::undo_record_with_next_t_t::~undo_record_with_next_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_record_with_next_t_t::_clean_up() {
    if (m_record) {
        delete m_record; m_record = 0;
    }
    if (f_next_record && !n_next_record) {
        if (m_next_record) {
            delete m_next_record; m_next_record = 0;
        }
    }
}

innodb_page_undo_log_t::undo_record_with_next_t_t* innodb_page_undo_log_t::undo_record_with_next_t_t::next_record() {
    if (f_next_record)
        return m_next_record;
    f_next_record = true;
    n_next_record = true;
    if ( ((record()->next_record_offset() != 0) && (record()->next_record_offset() < _root()->_io()->size())) ) {
        n_next_record = false;
        std::streampos _pos = m__io->pos();
        m__io->seek(record()->next_record_offset());
        m_next_record = new undo_record_with_next_t_t(m__io, this, m__root);
        m__io->seek(_pos);
    }
    return m_next_record;
}

innodb_page_undo_log_t::undo_truncate_data_t_t::undo_truncate_data_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_t_t* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_truncate_table_id = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_truncate_data_t_t::_read() {
    m_truncate_table_id = new innodb_common_t::mach_compressed_uint_t_t(m__io);
    m_truncate_flags = m__io->read_u4le();
    m_truncate_index_id = m__io->read_u8le();
    m_truncate_extra_data = m__io->read_bytes_full();
}

innodb_page_undo_log_t::undo_truncate_data_t_t::~undo_truncate_data_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_truncate_data_t_t::_clean_up() {
    if (m_truncate_table_id) {
        delete m_truncate_table_id; m_truncate_table_id = 0;
    }
}

innodb_page_undo_log_t::undo_update_data_t_t::undo_update_data_t_t(kaitai::kstream* p__io, innodb_page_undo_log_t::undo_record_t_t* p__parent, innodb_page_undo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_field_numbers = 0;
    m_field_old_values = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_page_undo_log_t::undo_update_data_t_t::_read() {
    m_num_field_numbers = m__io->read_u4le();
    m_field_numbers = new std::vector<innodb_common_t::mach_compressed_uint_t_t*>();
    const int l_field_numbers = num_field_numbers();
    for (int i = 0; i < l_field_numbers; i++) {
        m_field_numbers->push_back(new innodb_common_t::mach_compressed_uint_t_t(m__io));
    }
    m_field_old_values = new std::vector<undo_field_data_t_t*>();
    const int l_field_old_values = num_field_numbers();
    for (int i = 0; i < l_field_old_values; i++) {
        m_field_old_values->push_back(new undo_field_data_t_t(m__io, this, m__root));
    }
}

innodb_page_undo_log_t::undo_update_data_t_t::~undo_update_data_t_t() {
    _clean_up();
}

void innodb_page_undo_log_t::undo_update_data_t_t::_clean_up() {
    if (m_field_numbers) {
        for (std::vector<innodb_common_t::mach_compressed_uint_t_t*>::iterator it = m_field_numbers->begin(); it != m_field_numbers->end(); ++it) {
            delete *it;
        }
        delete m_field_numbers; m_field_numbers = 0;
    }
    if (m_field_old_values) {
        for (std::vector<undo_field_data_t_t*>::iterator it = m_field_old_values->begin(); it != m_field_old_values->end(); ++it) {
            delete *it;
        }
        delete m_field_old_values; m_field_old_values = 0;
    }
}
