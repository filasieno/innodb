#ifndef INNODB_PAGE_TRX_SYS_H_
#define INNODB_PAGE_TRX_SYS_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_trx_sys_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include "innodb_page_fsp_hdr.h"
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_trx_sys_t : public kaitai::kstruct {

public:
    class binlog_info_t_t;
    class trx_sys_header_t_t;

    innodb_page_trx_sys_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_trx_sys_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_trx_sys_t();

    class binlog_info_t_t : public kaitai::kstruct {

    public:

        binlog_info_t_t(kaitai::kstream* p__io, innodb_page_trx_sys_t::trx_sys_header_t_t* p__parent = 0, innodb_page_trx_sys_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~binlog_info_t_t();

    private:
        uint32_t m_binlog_file_name_len;
        std::string m_binlog_file_name;
        uint64_t m_binlog_offset;
        innodb_page_trx_sys_t* m__root;
        innodb_page_trx_sys_t::trx_sys_header_t_t* m__parent;

    public:
        uint32_t binlog_file_name_len() const { return m_binlog_file_name_len; }
        std::string binlog_file_name() const { return m_binlog_file_name; }
        uint64_t binlog_offset() const { return m_binlog_offset; }
        innodb_page_trx_sys_t* _root() const { return m__root; }
        innodb_page_trx_sys_t::trx_sys_header_t_t* _parent() const { return m__parent; }
    };

    class trx_sys_header_t_t : public kaitai::kstruct {

    public:

        trx_sys_header_t_t(kaitai::kstream* p__io, innodb_page_trx_sys_t* p__parent = 0, innodb_page_trx_sys_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~trx_sys_header_t_t();

    private:
        uint32_t m_trx_sys_magic;
        uint64_t m_trx_id_high;
        uint32_t m_doublewrite_magic;
        innodb_page_fsp_hdr_t::fil_addr_t_t* m_doublewrite_block1;
        innodb_page_fsp_hdr_t::fil_addr_t_t* m_doublewrite_block2;
        std::string m_doublewrite_fseg_header;
        binlog_info_t_t* m_binlog_info;
        std::vector<uint32_t>* m_rseg_array;
        innodb_page_trx_sys_t* m__root;
        innodb_page_trx_sys_t* m__parent;

    public:
        uint32_t trx_sys_magic() const { return m_trx_sys_magic; }
        uint64_t trx_id_high() const { return m_trx_id_high; }
        uint32_t doublewrite_magic() const { return m_doublewrite_magic; }
        innodb_page_fsp_hdr_t::fil_addr_t_t* doublewrite_block1() const { return m_doublewrite_block1; }
        innodb_page_fsp_hdr_t::fil_addr_t_t* doublewrite_block2() const { return m_doublewrite_block2; }
        std::string doublewrite_fseg_header() const { return m_doublewrite_fseg_header; }
        binlog_info_t_t* binlog_info() const { return m_binlog_info; }
        std::vector<uint32_t>* rseg_array() const { return m_rseg_array; }
        innodb_page_trx_sys_t* _root() const { return m__root; }
        innodb_page_trx_sys_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    trx_sys_header_t_t* m_trx_sys_header;
    std::string m_empty_space;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_trx_sys_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }
    trx_sys_header_t_t* trx_sys_header() const { return m_trx_sys_header; }
    std::string empty_space() const { return m_empty_space; }
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_trx_sys_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_TRX_SYS_H_
