// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_record_format.h"
std::set<innodb_record_format_t::field_type_enum_t> innodb_record_format_t::_build_values_field_type_enum_t() {
    std::set<innodb_record_format_t::field_type_enum_t> _t;
    _t.insert(innodb_record_format_t::FIELD_TYPE_ENUM_VARCHAR);
    _t.insert(innodb_record_format_t::FIELD_TYPE_ENUM_CHAR);
    _t.insert(innodb_record_format_t::FIELD_TYPE_ENUM_BINARY);
    _t.insert(innodb_record_format_t::FIELD_TYPE_ENUM_VARBINARY);
    _t.insert(innodb_record_format_t::FIELD_TYPE_ENUM_BLOB);
    _t.insert(innodb_record_format_t::FIELD_TYPE_ENUM_BLOB_TYPE);
    return _t;
}
const std::set<innodb_record_format_t::field_type_enum_t> innodb_record_format_t::_values_field_type_enum_t = innodb_record_format_t::_build_values_field_type_enum_t();
bool innodb_record_format_t::_is_defined_field_type_enum_t(innodb_record_format_t::field_type_enum_t v) {
    return innodb_record_format_t::_values_field_type_enum_t.find(v) != innodb_record_format_t::_values_field_type_enum_t.end();
}
std::set<innodb_record_format_t::record_type_enum_t> innodb_record_format_t::_build_values_record_type_enum_t() {
    std::set<innodb_record_format_t::record_type_enum_t> _t;
    _t.insert(innodb_record_format_t::RECORD_TYPE_ENUM_CONVENTIONAL);
    _t.insert(innodb_record_format_t::RECORD_TYPE_ENUM_NODE_POINTER);
    _t.insert(innodb_record_format_t::RECORD_TYPE_ENUM_INFIMUM);
    _t.insert(innodb_record_format_t::RECORD_TYPE_ENUM_SUPREMUM);
    return _t;
}
const std::set<innodb_record_format_t::record_type_enum_t> innodb_record_format_t::_values_record_type_enum_t = innodb_record_format_t::_build_values_record_type_enum_t();
bool innodb_record_format_t::_is_defined_record_type_enum_t(innodb_record_format_t::record_type_enum_t v) {
    return innodb_record_format_t::_values_record_type_enum_t.find(v) != innodb_record_format_t::_values_record_type_enum_t.end();
}

innodb_record_format_t::innodb_record_format_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_record_format_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_record_format_t::_read() {
}

innodb_record_format_t::~innodb_record_format_t() {
    _clean_up();
}

void innodb_record_format_t::_clean_up() {
}

innodb_record_format_t::blob_reference_t_t::blob_reference_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_record_format_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_record_format_t::blob_reference_t_t::_read() {
    m_space_id = m__io->read_u4le();
    m_page_no = m__io->read_u4le();
    m_offset = m__io->read_u4le();
    m_blob_length = m__io->read_u8le();
}

innodb_record_format_t::blob_reference_t_t::~blob_reference_t_t() {
    _clean_up();
}

void innodb_record_format_t::blob_reference_t_t::_clean_up() {
}

innodb_record_format_t::compact_record_header_t_t::compact_record_header_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_record_format_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    f_is_deleted = false;
    f_is_min_rec = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_record_format_t::compact_record_header_t_t::_read() {
    m_info_flags = m__io->read_u1();
    m_n_owned = m__io->read_u1();
    m_heap_no = m__io->read_u2le();
    m_record_type = static_cast<innodb_record_format_t::record_type_enum_t>(m__io->read_u1());
    m_next_record_offset = m__io->read_s2le();
}

innodb_record_format_t::compact_record_header_t_t::~compact_record_header_t_t() {
    _clean_up();
}

void innodb_record_format_t::compact_record_header_t_t::_clean_up() {
}

bool innodb_record_format_t::compact_record_header_t_t::is_deleted() {
    if (f_is_deleted)
        return m_is_deleted;
    f_is_deleted = true;
    m_is_deleted = (info_flags() & 8) != 0;
    return m_is_deleted;
}

bool innodb_record_format_t::compact_record_header_t_t::is_min_rec() {
    if (f_is_min_rec)
        return m_is_min_rec;
    f_is_min_rec = true;
    m_is_min_rec = (info_flags() & 16) != 0;
    return m_is_min_rec;
}
