// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_redo_log.h"
#include "kaitai/exceptions.h"
std::set<innodb_redo_log_t::mlog_type_t> innodb_redo_log_t::_build_values_mlog_type_t() {
    std::set<innodb_redo_log_t::mlog_type_t> _t;
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_1BYTE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_2BYTES);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_4BYTES);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_8BYTES);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_REC_INSERT);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_REC_CLUST_DELETE_MARK);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_REC_SEC_DELETE_MARK);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_REC_UPDATE_IN_PLACE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_REC_DELETE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_LIST_END_DELETE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_LIST_START_DELETE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_LIST_END_COPY_CREATED);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_PAGE_REORGANIZE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_PAGE_CREATE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_UNDO_INSERT);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_UNDO_ERASE_END);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_UNDO_INIT);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_UNDO_HDR_REUSE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_UNDO_HDR_CREATE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_REC_MIN_MARK);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_IBUF_BITMAP_INIT);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_INIT_FILE_PAGE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_WRITE_STRING);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_MULTI_REC_END);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_CHECKPOINT);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_PAGE_CREATE_COMPRESSED);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_PAGE_CREATE_RTREE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_REC_MIN_MARK);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_PAGE_CREATE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_REC_INSERT);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_REC_CLUST_DELETE_MARK);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_REC_SEC_DELETE_MARK);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_REC_UPDATE_IN_PLACE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_REC_DELETE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_LIST_END_DELETE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_LIST_START_DELETE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_LIST_END_COPY_CREATED);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_COMP_PAGE_REORGANIZE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_FILE_CREATE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_FILE_RENAME);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_FILE_DELETE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_FILE_CREATE2);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_FILE_RENAME2);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_TRUNCATE);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_INDEX_LOAD);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_TABLE_DYNAMIC_META);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_PAGE_INIT);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_ZIP_PAGE_COMPRESS);
    _t.insert(innodb_redo_log_t::MLOG_TYPE_MLOG_TEST);
    return _t;
}
const std::set<innodb_redo_log_t::mlog_type_t> innodb_redo_log_t::_values_mlog_type_t = innodb_redo_log_t::_build_values_mlog_type_t();
bool innodb_redo_log_t::_is_defined_mlog_type_t(innodb_redo_log_t::mlog_type_t v) {
    return innodb_redo_log_t::_values_mlog_type_t.find(v) != innodb_redo_log_t::_values_mlog_type_t.end();
}

innodb_redo_log_t::innodb_redo_log_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_file_header = 0;
    m_checkpoint_1 = 0;
    m_checkpoint_2 = 0;
    m_log_blocks = 0;
    f_active_checkpoint = false;
    f_log_format_version = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::_read() {
    m_file_header = new file_header_t_t(m__io, this, m__root);
    m_checkpoint_1 = new checkpoint_block_t_t(m__io, this, m__root);
    m_checkpoint_2 = new checkpoint_block_t_t(m__io, this, m__root);
    m_log_blocks = new std::vector<log_block_t_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_log_blocks->push_back(new log_block_t_t(m__io, this, m__root));
            i++;
        }
    }
}

innodb_redo_log_t::~innodb_redo_log_t() {
    _clean_up();
}

void innodb_redo_log_t::_clean_up() {
    if (m_file_header) {
        delete m_file_header; m_file_header = 0;
    }
    if (m_checkpoint_1) {
        delete m_checkpoint_1; m_checkpoint_1 = 0;
    }
    if (m_checkpoint_2) {
        delete m_checkpoint_2; m_checkpoint_2 = 0;
    }
    if (m_log_blocks) {
        for (std::vector<log_block_t_t*>::iterator it = m_log_blocks->begin(); it != m_log_blocks->end(); ++it) {
            delete *it;
        }
        delete m_log_blocks; m_log_blocks = 0;
    }
}

innodb_redo_log_t::checkpoint_block_t_t::checkpoint_block_t_t(kaitai::kstream* p__io, innodb_redo_log_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_block_1 = 0;
    m_block_2 = 0;
    f_checkpoint_lsn = false;
    f_checkpoint_no = false;
    f_checkpoint_offset = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::checkpoint_block_t_t::_read() {
    m_block_1 = new log_block_t_t(m__io, this, m__root);
    m_block_2 = new log_block_t_t(m__io, this, m__root);
}

innodb_redo_log_t::checkpoint_block_t_t::~checkpoint_block_t_t() {
    _clean_up();
}

void innodb_redo_log_t::checkpoint_block_t_t::_clean_up() {
    if (m_block_1) {
        delete m_block_1; m_block_1 = 0;
    }
    if (m_block_2) {
        delete m_block_2; m_block_2 = 0;
    }
    if (f_checkpoint_lsn) {
    }
    if (f_checkpoint_no) {
    }
    if (f_checkpoint_offset) {
    }
}

uint64_t innodb_redo_log_t::checkpoint_block_t_t::checkpoint_lsn() {
    if (f_checkpoint_lsn)
        return m_checkpoint_lsn;
    f_checkpoint_lsn = true;
    std::streampos _pos = m__io->pos();
    m__io->seek(20);
    m_checkpoint_lsn = m__io->read_u8le();
    m__io->seek(_pos);
    return m_checkpoint_lsn;
}

uint64_t innodb_redo_log_t::checkpoint_block_t_t::checkpoint_no() {
    if (f_checkpoint_no)
        return m_checkpoint_no;
    f_checkpoint_no = true;
    std::streampos _pos = m__io->pos();
    m__io->seek(12);
    m_checkpoint_no = m__io->read_u8le();
    m__io->seek(_pos);
    return m_checkpoint_no;
}

uint64_t innodb_redo_log_t::checkpoint_block_t_t::checkpoint_offset() {
    if (f_checkpoint_offset)
        return m_checkpoint_offset;
    f_checkpoint_offset = true;
    std::streampos _pos = m__io->pos();
    m__io->seek(28);
    m_checkpoint_offset = m__io->read_u8le();
    m__io->seek(_pos);
    return m_checkpoint_offset;
}

innodb_redo_log_t::checkpoint_record_t_t::checkpoint_record_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::checkpoint_record_t_t::_read() {
    m_checkpoint_lsn = m__io->read_u8le();
    m_checkpoint_no = m__io->read_u8le();
}

innodb_redo_log_t::checkpoint_record_t_t::~checkpoint_record_t_t() {
    _clean_up();
}

void innodb_redo_log_t::checkpoint_record_t_t::_clean_up() {
}

innodb_redo_log_t::compressed_uint_t_t::compressed_uint_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
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

void innodb_redo_log_t::compressed_uint_t_t::_read() {
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

innodb_redo_log_t::compressed_uint_t_t::~compressed_uint_t_t() {
    _clean_up();
}

void innodb_redo_log_t::compressed_uint_t_t::_clean_up() {
    if (!n_second_byte) {
    }
    if (!n_third_byte) {
    }
    if (!n_fourth_byte) {
    }
    if (!n_fifth_byte) {
    }
}

int32_t innodb_redo_log_t::compressed_uint_t_t::value() {
    if (f_value)
        return m_value;
    f_value = true;
    m_value = ((first_byte() < 128) ? (first_byte()) : (((first_byte() < 192) ? ((first_byte() & 63) << 8 | second_byte()) : (((first_byte() < 224) ? (((first_byte() & 31) << 16 | second_byte() << 8) | third_byte()) : (((first_byte() < 240) ? ((((first_byte() & 15) << 24 | second_byte() << 16) | third_byte() << 8) | fourth_byte()) : ((((static_cast<uint64_t>(first_byte()) << 32 | static_cast<uint64_t>(second_byte()) << 24) | static_cast<uint64_t>(third_byte()) << 16) | static_cast<uint64_t>(fourth_byte()) << 8) | fifth_byte()))))))));
    return m_value;
}

innodb_redo_log_t::file_header_t_t::file_header_t_t(kaitai::kstream* p__io, innodb_redo_log_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::file_header_t_t::_read() {
    m_magic = m__io->read_bytes(5);
    if (!(m_magic == std::string("\x49\x42\x4C\x4F\x47", 5))) {
        throw kaitai::validation_not_equal_error<std::string>(std::string("\x49\x42\x4C\x4F\x47", 5), m_magic, m__io, std::string("/types/file_header_t/seq/0"));
    }
    m_format_version = m__io->read_u4le();
    m_start_lsn = m__io->read_u8le();
    m_creator_name = kaitai::kstream::bytes_to_str(m__io->read_bytes(32), "ASCII");
    m_log_flags = m__io->read_u4le();
    m_log_uuid = m__io->read_bytes(16);
    m_header_checksum = m__io->read_u4le();
    m_reserved = m__io->read_bytes(512 - _io()->pos());
}

innodb_redo_log_t::file_header_t_t::~file_header_t_t() {
    _clean_up();
}

void innodb_redo_log_t::file_header_t_t::_clean_up() {
}

innodb_redo_log_t::generic_record_data_t_t::generic_record_data_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::generic_record_data_t_t::_read() {
    m_data = m__io->read_bytes_full();
}

innodb_redo_log_t::generic_record_data_t_t::~generic_record_data_t_t() {
    _clean_up();
}

void innodb_redo_log_t::generic_record_data_t_t::_clean_up() {
}

innodb_redo_log_t::log_block_header_t_t::log_block_header_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_block_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    f_block_no_without_flush_bit = false;
    f_is_flush_bit_set = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::log_block_header_t_t::_read() {
    m_hdr_no = m__io->read_u4le();
    m_data_len = m__io->read_u2le();
    m_first_rec_group = m__io->read_u2le();
    m_checkpoint_no = m__io->read_u4le();
}

innodb_redo_log_t::log_block_header_t_t::~log_block_header_t_t() {
    _clean_up();
}

void innodb_redo_log_t::log_block_header_t_t::_clean_up() {
}

int32_t innodb_redo_log_t::log_block_header_t_t::block_no_without_flush_bit() {
    if (f_block_no_without_flush_bit)
        return m_block_no_without_flush_bit;
    f_block_no_without_flush_bit = true;
    m_block_no_without_flush_bit = hdr_no() & 2147483647;
    return m_block_no_without_flush_bit;
}

bool innodb_redo_log_t::log_block_header_t_t::is_flush_bit_set() {
    if (f_is_flush_bit_set)
        return m_is_flush_bit_set;
    f_is_flush_bit_set = true;
    m_is_flush_bit_set = (hdr_no() & 2147483648UL) != 0;
    return m_is_flush_bit_set;
}

innodb_redo_log_t::log_block_t_t::log_block_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_header = 0;
    m_trailer = 0;
    m_log_records = 0;
    m__io__raw_log_records = 0;
    f_block_number = false;
    f_has_valid_data = false;
    f_log_records = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::log_block_t_t::_read() {
    m_header = new log_block_header_t_t(m__io, this, m__root);
    m_data = m__io->read_bytes(496);
    m_trailer = new log_block_trailer_t_t(m__io, this, m__root);
}

innodb_redo_log_t::log_block_t_t::~log_block_t_t() {
    _clean_up();
}

void innodb_redo_log_t::log_block_t_t::_clean_up() {
    if (m_header) {
        delete m_header; m_header = 0;
    }
    if (m_trailer) {
        delete m_trailer; m_trailer = 0;
    }
    if (f_log_records && !n_log_records) {
        if (m__io__raw_log_records) {
            delete m__io__raw_log_records; m__io__raw_log_records = 0;
        }
        if (m_log_records) {
            delete m_log_records; m_log_records = 0;
        }
    }
}

uint32_t innodb_redo_log_t::log_block_t_t::block_number() {
    if (f_block_number)
        return m_block_number;
    f_block_number = true;
    m_block_number = header()->hdr_no();
    return m_block_number;
}

bool innodb_redo_log_t::log_block_t_t::has_valid_data() {
    if (f_has_valid_data)
        return m_has_valid_data;
    f_has_valid_data = true;
    m_has_valid_data =  ((header()->data_len() > 0) && (header()->data_len() <= 496)) ;
    return m_has_valid_data;
}

innodb_redo_log_t::log_records_t_t* innodb_redo_log_t::log_block_t_t::log_records() {
    if (f_log_records)
        return m_log_records;
    f_log_records = true;
    n_log_records = true;
    if (has_valid_data()) {
        n_log_records = false;
        std::streampos _pos = m__io->pos();
        m__io->seek(12);
        m__raw_log_records = m__io->read_bytes(header()->data_len());
        m__io__raw_log_records = new kaitai::kstream(m__raw_log_records);
        m_log_records = new log_records_t_t(m__io__raw_log_records, this, m__root);
        m__io->seek(_pos);
    }
    return m_log_records;
}

innodb_redo_log_t::log_block_trailer_t_t::log_block_trailer_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_block_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::log_block_trailer_t_t::_read() {
    m_checksum = m__io->read_u4le();
}

innodb_redo_log_t::log_block_trailer_t_t::~log_block_trailer_t_t() {
    _clean_up();
}

void innodb_redo_log_t::log_block_trailer_t_t::_clean_up() {
}

innodb_redo_log_t::log_record_t_t::log_record_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_records_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_space_id = 0;
    m_page_no = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::log_record_t_t::_read() {
    m_type = static_cast<innodb_redo_log_t::mlog_type_t>(m__io->read_u1());
    n_space_id = true;
    if (type() != innodb_redo_log_t::MLOG_TYPE_MLOG_MULTI_REC_END) {
        n_space_id = false;
        m_space_id = new compressed_uint_t_t(m__io, this, m__root);
    }
    n_page_no = true;
    if ( ((type() != innodb_redo_log_t::MLOG_TYPE_MLOG_MULTI_REC_END) && (type() != innodb_redo_log_t::MLOG_TYPE_MLOG_CHECKPOINT)) ) {
        n_page_no = false;
        m_page_no = new compressed_uint_t_t(m__io, this, m__root);
    }
    switch (type()) {
    case innodb_redo_log_t::MLOG_TYPE_MLOG_1BYTE: {
        m_record_data = new write_1byte_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_2BYTES: {
        m_record_data = new write_2bytes_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_4BYTES: {
        m_record_data = new write_4bytes_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_8BYTES: {
        m_record_data = new write_8bytes_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_CHECKPOINT: {
        m_record_data = new checkpoint_record_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_PAGE_CREATE: {
        m_record_data = new page_create_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_REC_CLUST_DELETE_MARK: {
        m_record_data = new rec_delete_mark_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_REC_INSERT: {
        m_record_data = new rec_insert_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_REC_UPDATE_IN_PLACE: {
        m_record_data = new rec_update_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_UNDO_ERASE_END: {
        m_record_data = new undo_erase_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_UNDO_INSERT: {
        m_record_data = new undo_insert_t_t(m__io, this, m__root);
        break;
    }
    case innodb_redo_log_t::MLOG_TYPE_MLOG_WRITE_STRING: {
        m_record_data = new write_string_t_t(m__io, this, m__root);
        break;
    }
    default: {
        m_record_data = new generic_record_data_t_t(m__io, this, m__root);
        break;
    }
    }
}

innodb_redo_log_t::log_record_t_t::~log_record_t_t() {
    _clean_up();
}

void innodb_redo_log_t::log_record_t_t::_clean_up() {
    if (!n_space_id) {
        if (m_space_id) {
            delete m_space_id; m_space_id = 0;
        }
    }
    if (!n_page_no) {
        if (m_page_no) {
            delete m_page_no; m_page_no = 0;
        }
    }
    if (m_record_data) {
        delete m_record_data; m_record_data = 0;
    }
}

innodb_redo_log_t::log_records_t_t::log_records_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_block_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_records = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::log_records_t_t::_read() {
    m_records = new std::vector<log_record_t_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_records->push_back(new log_record_t_t(m__io, this, m__root));
            i++;
        }
    }
}

innodb_redo_log_t::log_records_t_t::~log_records_t_t() {
    _clean_up();
}

void innodb_redo_log_t::log_records_t_t::_clean_up() {
    if (m_records) {
        for (std::vector<log_record_t_t*>::iterator it = m_records->begin(); it != m_records->end(); ++it) {
            delete *it;
        }
        delete m_records; m_records = 0;
    }
}

innodb_redo_log_t::page_create_t_t::page_create_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::page_create_t_t::_read() {
    m_page_type = m__io->read_u2le();
    m_index_id = m__io->read_u8le();
}

innodb_redo_log_t::page_create_t_t::~page_create_t_t() {
    _clean_up();
}

void innodb_redo_log_t::page_create_t_t::_clean_up() {
}

innodb_redo_log_t::rec_delete_mark_t_t::rec_delete_mark_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::rec_delete_mark_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_flags = m__io->read_u1();
}

innodb_redo_log_t::rec_delete_mark_t_t::~rec_delete_mark_t_t() {
    _clean_up();
}

void innodb_redo_log_t::rec_delete_mark_t_t::_clean_up() {
}

innodb_redo_log_t::rec_insert_t_t::rec_insert_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_rec_len = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::rec_insert_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_rec_len = new compressed_uint_t_t(m__io, this, m__root);
    m_record_data = m__io->read_bytes(rec_len()->value());
}

innodb_redo_log_t::rec_insert_t_t::~rec_insert_t_t() {
    _clean_up();
}

void innodb_redo_log_t::rec_insert_t_t::_clean_up() {
    if (m_rec_len) {
        delete m_rec_len; m_rec_len = 0;
    }
}

innodb_redo_log_t::rec_update_t_t::rec_update_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_update_vector_len = 0;
    m_update_fields = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::rec_update_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_update_vector_len = new compressed_uint_t_t(m__io, this, m__root);
    m_update_fields = new std::vector<update_field_t_t*>();
    const int l_update_fields = update_vector_len()->value();
    for (int i = 0; i < l_update_fields; i++) {
        m_update_fields->push_back(new update_field_t_t(m__io, this, m__root));
    }
}

innodb_redo_log_t::rec_update_t_t::~rec_update_t_t() {
    _clean_up();
}

void innodb_redo_log_t::rec_update_t_t::_clean_up() {
    if (m_update_vector_len) {
        delete m_update_vector_len; m_update_vector_len = 0;
    }
    if (m_update_fields) {
        for (std::vector<update_field_t_t*>::iterator it = m_update_fields->begin(); it != m_update_fields->end(); ++it) {
            delete *it;
        }
        delete m_update_fields; m_update_fields = 0;
    }
}

innodb_redo_log_t::undo_erase_t_t::undo_erase_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::undo_erase_t_t::_read() {
    m_offset = m__io->read_u2le();
}

innodb_redo_log_t::undo_erase_t_t::~undo_erase_t_t() {
    _clean_up();
}

void innodb_redo_log_t::undo_erase_t_t::_clean_up() {
}

innodb_redo_log_t::undo_insert_t_t::undo_insert_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_len = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::undo_insert_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_len = new compressed_uint_t_t(m__io, this, m__root);
    m_data = m__io->read_bytes(len()->value());
}

innodb_redo_log_t::undo_insert_t_t::~undo_insert_t_t() {
    _clean_up();
}

void innodb_redo_log_t::undo_insert_t_t::_clean_up() {
    if (m_len) {
        delete m_len; m_len = 0;
    }
}

innodb_redo_log_t::update_field_t_t::update_field_t_t(kaitai::kstream* p__io, innodb_redo_log_t::rec_update_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_field_no = 0;
    m_field_len = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::update_field_t_t::_read() {
    m_field_no = new compressed_uint_t_t(m__io, this, m__root);
    m_field_len = new compressed_uint_t_t(m__io, this, m__root);
    m_field_data = m__io->read_bytes(field_len()->value());
}

innodb_redo_log_t::update_field_t_t::~update_field_t_t() {
    _clean_up();
}

void innodb_redo_log_t::update_field_t_t::_clean_up() {
    if (m_field_no) {
        delete m_field_no; m_field_no = 0;
    }
    if (m_field_len) {
        delete m_field_len; m_field_len = 0;
    }
}

innodb_redo_log_t::write_1byte_t_t::write_1byte_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::write_1byte_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_value = m__io->read_u1();
}

innodb_redo_log_t::write_1byte_t_t::~write_1byte_t_t() {
    _clean_up();
}

void innodb_redo_log_t::write_1byte_t_t::_clean_up() {
}

innodb_redo_log_t::write_2bytes_t_t::write_2bytes_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::write_2bytes_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_value = m__io->read_u2le();
}

innodb_redo_log_t::write_2bytes_t_t::~write_2bytes_t_t() {
    _clean_up();
}

void innodb_redo_log_t::write_2bytes_t_t::_clean_up() {
}

innodb_redo_log_t::write_4bytes_t_t::write_4bytes_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::write_4bytes_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_value = m__io->read_u4le();
}

innodb_redo_log_t::write_4bytes_t_t::~write_4bytes_t_t() {
    _clean_up();
}

void innodb_redo_log_t::write_4bytes_t_t::_clean_up() {
}

innodb_redo_log_t::write_8bytes_t_t::write_8bytes_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::write_8bytes_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_value = m__io->read_u8le();
}

innodb_redo_log_t::write_8bytes_t_t::~write_8bytes_t_t() {
    _clean_up();
}

void innodb_redo_log_t::write_8bytes_t_t::_clean_up() {
}

innodb_redo_log_t::write_string_t_t::write_string_t_t(kaitai::kstream* p__io, innodb_redo_log_t::log_record_t_t* p__parent, innodb_redo_log_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_length = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_redo_log_t::write_string_t_t::_read() {
    m_offset = m__io->read_u2le();
    m_length = new compressed_uint_t_t(m__io, this, m__root);
    m_data = m__io->read_bytes(length()->value());
}

innodb_redo_log_t::write_string_t_t::~write_string_t_t() {
    _clean_up();
}

void innodb_redo_log_t::write_string_t_t::_clean_up() {
    if (m_length) {
        delete m_length; m_length = 0;
    }
}

innodb_redo_log_t::checkpoint_block_t_t* innodb_redo_log_t::active_checkpoint() {
    if (f_active_checkpoint)
        return m_active_checkpoint;
    f_active_checkpoint = true;
    m_active_checkpoint = ((checkpoint_1()->checkpoint_no() > checkpoint_2()->checkpoint_no()) ? (checkpoint_1()) : (checkpoint_2()));
    return m_active_checkpoint;
}

uint32_t innodb_redo_log_t::log_format_version() {
    if (f_log_format_version)
        return m_log_format_version;
    f_log_format_version = true;
    m_log_format_version = file_header()->format_version();
    return m_log_format_version;
}
