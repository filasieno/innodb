#ifndef INNODB_PAGE_BLOB_H_
#define INNODB_PAGE_BLOB_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_blob_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_blob_t : public kaitai::kstruct {

public:
    class blob_header_t_t;

    innodb_page_blob_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_blob_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_blob_t();

    class blob_header_t_t : public kaitai::kstruct {

    public:

        blob_header_t_t(kaitai::kstream* p__io, innodb_page_blob_t* p__parent = 0, innodb_page_blob_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~blob_header_t_t();

    private:
        bool f_has_next;
        bool m_has_next;

    public:
        bool has_next();

    private:
        uint32_t m_blob_part_len;
        uint32_t m_next_page_no;
        innodb_page_blob_t* m__root;
        innodb_page_blob_t* m__parent;

    public:
        uint32_t blob_part_len() const { return m_blob_part_len; }
        uint32_t next_page_no() const { return m_next_page_no; }
        innodb_page_blob_t* _root() const { return m__root; }
        innodb_page_blob_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    blob_header_t_t* m_blob_header;
    std::string m_blob_data;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_blob_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }
    blob_header_t_t* blob_header() const { return m_blob_header; }
    std::string blob_data() const { return m_blob_data; }
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_blob_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_BLOB_H_
