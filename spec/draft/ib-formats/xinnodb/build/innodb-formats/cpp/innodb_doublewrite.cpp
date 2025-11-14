// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "innodb_doublewrite.h"
#include "kaitai/exceptions.h"

innodb_doublewrite_t::innodb_doublewrite_t(uint32_t p_page_size, kaitai::kstream* p__io, kaitai::kstruct* p__parent, innodb_doublewrite_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root ? p__root : this;
    m_page_size = p_page_size;
    m_dblwr_header = 0;
    m_pages = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_doublewrite_t::_read() {
    m_dblwr_header = new dblwr_header_t_t(m__io, this, m__root);
    m_pages = new std::vector<dblwr_page_t_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_pages->push_back(new dblwr_page_t_t(m__io, this, m__root));
            i++;
        }
    }
}

innodb_doublewrite_t::~innodb_doublewrite_t() {
    _clean_up();
}

void innodb_doublewrite_t::_clean_up() {
    if (m_dblwr_header) {
        delete m_dblwr_header; m_dblwr_header = 0;
    }
    if (m_pages) {
        for (std::vector<dblwr_page_t_t*>::iterator it = m_pages->begin(); it != m_pages->end(); ++it) {
            delete *it;
        }
        delete m_pages; m_pages = 0;
    }
}

innodb_doublewrite_t::dblwr_header_t_t::dblwr_header_t_t(kaitai::kstream* p__io, innodb_doublewrite_t* p__parent, innodb_doublewrite_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_doublewrite_t::dblwr_header_t_t::_read() {
    m_magic = m__io->read_bytes(4);
    if (!(m_magic == std::string("\x44\x42\x4C\x57", 4))) {
        throw kaitai::validation_not_equal_error<std::string>(std::string("\x44\x42\x4C\x57", 4), m_magic, m__io, std::string("/types/dblwr_header_t/seq/0"));
    }
    m_version = m__io->read_u4le();
    m_page_size = m__io->read_u4le();
    m_max_pages = m__io->read_u4le();
    m_reserved = m__io->read_bytes(_root()->page_size() - 16);
}

innodb_doublewrite_t::dblwr_header_t_t::~dblwr_header_t_t() {
    _clean_up();
}

void innodb_doublewrite_t::dblwr_header_t_t::_clean_up() {
}

innodb_doublewrite_t::dblwr_page_t_t::dblwr_page_t_t(kaitai::kstream* p__io, innodb_doublewrite_t* p__parent, innodb_doublewrite_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_fil_header = 0;
    f_fil_header = false;
    f_is_valid = false;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void innodb_doublewrite_t::dblwr_page_t_t::_read() {
    m_page_copy = m__io->read_bytes(_root()->page_size());
}

innodb_doublewrite_t::dblwr_page_t_t::~dblwr_page_t_t() {
    _clean_up();
}

void innodb_doublewrite_t::dblwr_page_t_t::_clean_up() {
    if (f_fil_header) {
        if (m_fil_header) {
            delete m_fil_header; m_fil_header = 0;
        }
    }
}

innodb_common_t::fil_header_t_t* innodb_doublewrite_t::dblwr_page_t_t::fil_header() {
    if (f_fil_header)
        return m_fil_header;
    f_fil_header = true;
    std::streampos _pos = m__io->pos();
    m__io->seek(0);
    m_fil_header = new innodb_common_t::fil_header_t_t(m__io);
    m__io->seek(_pos);
    return m_fil_header;
}

bool innodb_doublewrite_t::dblwr_page_t_t::is_valid() {
    if (f_is_valid)
        return m_is_valid;
    f_is_valid = true;
    m_is_valid = fil_header()->page_no() != 4294967295UL;
    return m_is_valid;
}
