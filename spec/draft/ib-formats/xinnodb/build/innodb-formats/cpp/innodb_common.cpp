// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_common.h"
std::set<innodb_common_t::checksum_algorithm_enum_t> innodb_common_t::_build_values_checksum_algorithm_enum_t() {
    std::set<innodb_common_t::checksum_algorithm_enum_t> _t;
    _t.insert(innodb_common_t::CHECKSUM_ALGORITHM_ENUM_CRC32);
    _t.insert(innodb_common_t::CHECKSUM_ALGORITHM_ENUM_INNODB);
    _t.insert(innodb_common_t::CHECKSUM_ALGORITHM_ENUM_NONE);
    _t.insert(innodb_common_t::CHECKSUM_ALGORITHM_ENUM_STRICT_CRC32);
    _t.insert(innodb_common_t::CHECKSUM_ALGORITHM_ENUM_STRICT_INNODB);
    _t.insert(innodb_common_t::CHECKSUM_ALGORITHM_ENUM_STRICT_NONE);
    return _t;
}
const std::set<innodb_common_t::checksum_algorithm_enum_t> innodb_common_t::_values_checksum_algorithm_enum_t = innodb_common_t::_build_values_checksum_algorithm_enum_t();
bool innodb_common_t::_is_defined_checksum_algorithm_enum_t(innodb_common_t::checksum_algorithm_enum_t v) {
    return innodb_common_t::_values_checksum_algorithm_enum_t.find(v) != innodb_common_t::_values_checksum_algorithm_enum_t.end();
}
std::set<innodb_common_t::page_type_enum_t> innodb_common_t::_build_values_page_type_enum_t() {
    std::set<innodb_common_t::page_type_enum_t> _t;
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ALLOCATED);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_UNDO_LOG);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_INODE);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_IBUF_FREE_LIST);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_IBUF_BITMAP);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_SYS);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_TRX_SYS);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_FSP_HDR);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_XDES);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_BLOB);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZLOB_FIRST);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZLOB_DATA);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZLOB_INDEX);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZBLOB);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZBLOB2);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_UNKNOWN);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_INDEX);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_SDI_BLOB);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_SDI_ZBLOB);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_LOB_INDEX);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_LOB_DATA);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_LOB_FIRST);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZLOB_FIRST_V2);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZLOB_DATA_V2);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZLOB_INDEX_V2);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZLOB_FRAG);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_ZLOB_FRAG_ENTRY);
    _t.insert(innodb_common_t::PAGE_TYPE_ENUM_RTREE);
    return _t;
}
const std::set<innodb_common_t::page_type_enum_t> innodb_common_t::_values_page_type_enum_t = innodb_common_t::_build_values_page_type_enum_t();
bool innodb_common_t::_is_defined_page_type_enum_t(innodb_common_t::page_type_enum_t v) {
    return innodb_common_t::_values_page_type_enum_t.find(v) != innodb_common_t::_values_page_type_enum_t.end();
}
std::set<innodb_common_t::row_format_enum_t> innodb_common_t::_build_values_row_format_enum_t() {
    std::set<innodb_common_t::row_format_enum_t> _t;
    _t.insert(innodb_common_t::ROW_FORMAT_ENUM_REDUNDANT);
    _t.insert(innodb_common_t::ROW_FORMAT_ENUM_COMPACT);
    _t.insert(innodb_common_t::ROW_FORMAT_ENUM_DYNAMIC);
    _t.insert(innodb_common_t::ROW_FORMAT_ENUM_COMPRESSED);
    return _t;
}
const std::set<innodb_common_t::row_format_enum_t> innodb_common_t::_values_row_format_enum_t = innodb_common_t::_build_values_row_format_enum_t();
bool innodb_common_t::_is_defined_row_format_enum_t(innodb_common_t::row_format_enum_t v) {
    return innodb_common_t::_values_row_format_enum_t.find(v) != innodb_common_t::_values_row_format_enum_t.end();
}

innodb_common_t::innodb_common_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_common_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_common_t::_read() {
}

innodb_common_t::~innodb_common_t() {
    _clean_up();
}

void innodb_common_t::_clean_up() {
}

innodb_common_t::fil_header_t_t::fil_header_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_common_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_common_t::fil_header_t_t::_read() {
    m_checksum = m__io->read_u4le();
    m_page_no = m__io->read_u4le();
    m_prev_page_lsn = m__io->read_u8le();
    m_page_type = static_cast<innodb_common_t::page_type_enum_t>(m__io->read_u2le());
    n_flush_lsn = true;
    if (page_no() == 0) {
        n_flush_lsn = false;
        m_flush_lsn = m__io->read_u8le();
    }
    m_space_id = m__io->read_u4le();
}

innodb_common_t::fil_header_t_t::~fil_header_t_t() {
    _clean_up();
}

void innodb_common_t::fil_header_t_t::_clean_up() {
    if (!n_flush_lsn) {
    }
}

innodb_common_t::fil_trailer_t_t::fil_trailer_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_common_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_common_t::fil_trailer_t_t::_read() {
    m_old_checksum = m__io->read_u4le();
    m_lsn_low32 = m__io->read_u4le();
}

innodb_common_t::fil_trailer_t_t::~fil_trailer_t_t() {
    _clean_up();
}

void innodb_common_t::fil_trailer_t_t::_clean_up() {
}

innodb_common_t::mach_compressed_uint_t_t::mach_compressed_uint_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_common_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    f_value = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_common_t::mach_compressed_uint_t_t::_read() {
    m_first_byte = m__io->read_u1();
    n_second_byte = true;
    if (first_byte() >= 128) {
        n_second_byte = false;
        m_second_byte = m__io->read_u1();
    }
    n_third_byte = true;
    if (first_byte() >= 192) {
        n_third_byte = false;
        m_third_byte = m__io->read_u1();
    }
    n_fourth_byte = true;
    if (first_byte() >= 224) {
        n_fourth_byte = false;
        m_fourth_byte = m__io->read_u1();
    }
    n_fifth_byte = true;
    if (first_byte() >= 240) {
        n_fifth_byte = false;
        m_fifth_byte = m__io->read_u1();
    }
}

innodb_common_t::mach_compressed_uint_t_t::~mach_compressed_uint_t_t() {
    _clean_up();
}

void innodb_common_t::mach_compressed_uint_t_t::_clean_up() {
    if (!n_second_byte) {
    }
    if (!n_third_byte) {
    }
    if (!n_fourth_byte) {
    }
    if (!n_fifth_byte) {
    }
}

int32_t innodb_common_t::mach_compressed_uint_t_t::value() {
    if (f_value)
        return m_value;
    f_value = true;
    m_value = ((first_byte() < 128) ? (first_byte()) : (((first_byte() < 192) ? ((first_byte() & 63) << 8 | second_byte()) : (((first_byte() < 224) ? (((first_byte() & 31) << 16 | second_byte() << 8) | third_byte()) : (((first_byte() < 240) ? ((((first_byte() & 15) << 24 | second_byte() << 16) | third_byte() << 8) | fourth_byte()) : (((static_cast<uint64_t>(second_byte()) << 24 | static_cast<uint64_t>(third_byte()) << 16) | static_cast<uint64_t>(fourth_byte()) << 8) | fifth_byte()))))))));
    return m_value;
}

innodb_common_t::space_flags_t_t::space_flags_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_common_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    f_atomic_blobs = false;
    f_data_dir = false;
    f_encryption = false;
    f_page_ssize = false;
    f_post_antelope = false;
    f_sdi = false;
    f_shared = false;
    f_temporary = false;
    f_zip_ssize = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_common_t::space_flags_t_t::_read() {
    m_flags_value = m__io->read_u4le();
}

innodb_common_t::space_flags_t_t::~space_flags_t_t() {
    _clean_up();
}

void innodb_common_t::space_flags_t_t::_clean_up() {
}

int32_t innodb_common_t::space_flags_t_t::atomic_blobs() {
    if (f_atomic_blobs)
        return m_atomic_blobs;
    f_atomic_blobs = true;
    m_atomic_blobs = flags_value() >> 5 & 1;
    return m_atomic_blobs;
}

int32_t innodb_common_t::space_flags_t_t::data_dir() {
    if (f_data_dir)
        return m_data_dir;
    f_data_dir = true;
    m_data_dir = flags_value() >> 10 & 1;
    return m_data_dir;
}

int32_t innodb_common_t::space_flags_t_t::encryption() {
    if (f_encryption)
        return m_encryption;
    f_encryption = true;
    m_encryption = flags_value() >> 13 & 1;
    return m_encryption;
}

int32_t innodb_common_t::space_flags_t_t::page_ssize() {
    if (f_page_ssize)
        return m_page_ssize;
    f_page_ssize = true;
    m_page_ssize = flags_value() >> 6 & 15;
    return m_page_ssize;
}

bool innodb_common_t::space_flags_t_t::post_antelope() {
    if (f_post_antelope)
        return m_post_antelope;
    f_post_antelope = true;
    m_post_antelope = (flags_value() & 1) != 0;
    return m_post_antelope;
}

int32_t innodb_common_t::space_flags_t_t::sdi() {
    if (f_sdi)
        return m_sdi;
    f_sdi = true;
    m_sdi = flags_value() >> 14 & 1;
    return m_sdi;
}

int32_t innodb_common_t::space_flags_t_t::shared() {
    if (f_shared)
        return m_shared;
    f_shared = true;
    m_shared = flags_value() >> 11 & 1;
    return m_shared;
}

int32_t innodb_common_t::space_flags_t_t::temporary() {
    if (f_temporary)
        return m_temporary;
    f_temporary = true;
    m_temporary = flags_value() >> 12 & 1;
    return m_temporary;
}

int32_t innodb_common_t::space_flags_t_t::zip_ssize() {
    if (f_zip_ssize)
        return m_zip_ssize;
    f_zip_ssize = true;
    m_zip_ssize = flags_value() >> 1 & 15;
    return m_zip_ssize;
}
