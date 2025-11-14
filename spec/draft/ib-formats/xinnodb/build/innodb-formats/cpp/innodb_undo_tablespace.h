#ifndef INNODB_UNDO_TABLESPACE_H_
#define INNODB_UNDO_TABLESPACE_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_undo_tablespace_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_page_fsp_hdr.h"
#include "innodb_page_undo_log.h"
#include "innodb_page_xdes.h"
#include "innodb_common.h"
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_undo_tablespace_t : public kaitai::kstruct {

public:
    class page_dispatcher_t_t;
    class page_wrapper_t_t;

    innodb_undo_tablespace_t(uint32_t p_page_size, kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_undo_tablespace_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_undo_tablespace_t();

    /**
     * Dispatcher for undo tablespace page types
     */

    class page_dispatcher_t_t : public kaitai::kstruct {

    public:

        page_dispatcher_t_t(kaitai::kstream* p__io, innodb_undo_tablespace_t::page_wrapper_t_t* p__parent = 0, innodb_undo_tablespace_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~page_dispatcher_t_t();

    private:
        innodb_common_t::fil_header_t_t* m_fil_header;
        kaitai::kstruct* m_page_body;
        bool n_page_body;

    public:
        bool _is_null_page_body() { page_body(); return n_page_body; };

    private:
        innodb_undo_tablespace_t* m__root;
        innodb_undo_tablespace_t::page_wrapper_t_t* m__parent;
        std::string m__raw_page_body;
        kaitai::kstream* m__io__raw_page_body;

    public:
        innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }
        kaitai::kstruct* page_body() const { return m_page_body; }
        innodb_undo_tablespace_t* _root() const { return m__root; }
        innodb_undo_tablespace_t::page_wrapper_t_t* _parent() const { return m__parent; }
        std::string _raw_page_body() const { return m__raw_page_body; }
        kaitai::kstream* _io__raw_page_body() const { return m__io__raw_page_body; }
    };

    /**
     * Page wrapper for undo tablespace pages
     */

    class page_wrapper_t_t : public kaitai::kstruct {

    public:

        page_wrapper_t_t(kaitai::kstream* p__io, innodb_undo_tablespace_t* p__parent = 0, innodb_undo_tablespace_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~page_wrapper_t_t();

    private:
        page_dispatcher_t_t* m_page_data;
        innodb_undo_tablespace_t* m__root;
        innodb_undo_tablespace_t* m__parent;
        std::string m__raw_page_data;
        kaitai::kstream* m__io__raw_page_data;

    public:
        page_dispatcher_t_t* page_data() const { return m_page_data; }
        innodb_undo_tablespace_t* _root() const { return m__root; }
        innodb_undo_tablespace_t* _parent() const { return m__parent; }
        std::string _raw_page_data() const { return m__raw_page_data; }
        kaitai::kstream* _io__raw_page_data() const { return m__io__raw_page_data; }
    };

private:
    bool f_fsp_header;
    page_wrapper_t_t* m_fsp_header;

public:

    /**
     * FSP header (page 0)
     */
    page_wrapper_t_t* fsp_header();

private:
    bool f_rseg_array;
    page_wrapper_t_t* m_rseg_array;

public:

    /**
     * Rollback segment array (typically page 3)
     */
    page_wrapper_t_t* rseg_array();

private:
    std::vector<page_wrapper_t_t*>* m_pages;
    uint32_t m_page_size;
    innodb_undo_tablespace_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Pages in undo tablespace
     */
    std::vector<page_wrapper_t_t*>* pages() const { return m_pages; }

    /**
     * Page size (default 16KB)
     */
    uint32_t page_size() const { return m_page_size; }
    innodb_undo_tablespace_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_UNDO_TABLESPACE_H_
