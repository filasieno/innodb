#ifndef INNODB_PAGE_INODE_H_
#define INNODB_PAGE_INODE_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_inode_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include "innodb_page_fsp_hdr.h"
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_inode_t : public kaitai::kstruct {

public:
    class fseg_inode_t_t;

    innodb_page_inode_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_inode_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_inode_t();

    class fseg_inode_t_t : public kaitai::kstruct {

    public:

        fseg_inode_t_t(kaitai::kstream* p__io, innodb_page_inode_t* p__parent = 0, innodb_page_inode_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~fseg_inode_t_t();

    private:
        bool f_is_used;
        bool m_is_used;

    public:
        bool is_used();

    private:
        bool f_magic_valid;
        bool m_magic_valid;

    public:
        bool magic_valid();

    private:
        uint64_t m_fseg_id;
        uint32_t m_not_full_n_used;
        innodb_page_fsp_hdr_t::flst_base_node_t_t* m_free_list;
        innodb_page_fsp_hdr_t::flst_base_node_t_t* m_not_full_list;
        innodb_page_fsp_hdr_t::flst_base_node_t_t* m_full_list;
        uint32_t m_magic_n;
        std::vector<uint32_t>* m_frag_arr;
        innodb_page_inode_t* m__root;
        innodb_page_inode_t* m__parent;

    public:
        uint64_t fseg_id() const { return m_fseg_id; }
        uint32_t not_full_n_used() const { return m_not_full_n_used; }
        innodb_page_fsp_hdr_t::flst_base_node_t_t* free_list() const { return m_free_list; }
        innodb_page_fsp_hdr_t::flst_base_node_t_t* not_full_list() const { return m_not_full_list; }
        innodb_page_fsp_hdr_t::flst_base_node_t_t* full_list() const { return m_full_list; }
        uint32_t magic_n() const { return m_magic_n; }
        std::vector<uint32_t>* frag_arr() const { return m_frag_arr; }
        innodb_page_inode_t* _root() const { return m__root; }
        innodb_page_inode_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    innodb_page_fsp_hdr_t::flst_node_t_t* m_list_node;
    std::vector<fseg_inode_t_t*>* m_inodes;
    std::string m_empty_space;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_inode_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }
    innodb_page_fsp_hdr_t::flst_node_t_t* list_node() const { return m_list_node; }
    std::vector<fseg_inode_t_t*>* inodes() const { return m_inodes; }
    std::string empty_space() const { return m_empty_space; }
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_inode_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_INODE_H_
