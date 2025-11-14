#ifndef INNODB_PAGE_RSEG_ARRAY_H_
#define INNODB_PAGE_RSEG_ARRAY_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_rseg_array_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_rseg_array_t : public kaitai::kstruct {

public:
    class rseg_array_header_t_t;

    innodb_page_rseg_array_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_rseg_array_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_rseg_array_t();

    class rseg_array_header_t_t : public kaitai::kstruct {

    public:

        rseg_array_header_t_t(kaitai::kstream* p__io, innodb_page_rseg_array_t* p__parent = 0, innodb_page_rseg_array_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~rseg_array_header_t_t();

    private:
        uint32_t m_max_rollback_segments;
        uint32_t m_rseg_array_size;
        uint32_t m_rseg_array_version;
        innodb_page_rseg_array_t* m__root;
        innodb_page_rseg_array_t* m__parent;

    public:
        uint32_t max_rollback_segments() const { return m_max_rollback_segments; }
        uint32_t rseg_array_size() const { return m_rseg_array_size; }
        uint32_t rseg_array_version() const { return m_rseg_array_version; }
        innodb_page_rseg_array_t* _root() const { return m__root; }
        innodb_page_rseg_array_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    rseg_array_header_t_t* m_rseg_array_header;
    std::vector<uint32_t>* m_rseg_slots;
    std::string m_empty_space;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_rseg_array_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }
    rseg_array_header_t_t* rseg_array_header() const { return m_rseg_array_header; }
    std::vector<uint32_t>* rseg_slots() const { return m_rseg_slots; }
    std::string empty_space() const { return m_empty_space; }
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_rseg_array_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_RSEG_ARRAY_H_
