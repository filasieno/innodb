#ifndef INNODB_TABLESPACE_H_
#define INNODB_TABLESPACE_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_tablespace_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_page_fsp_hdr.h"
#include "innodb_common.h"
#include "innodb_page_zblob2.h"
#include "innodb_page_undo_log.h"
#include "innodb_page_lob_data.h"
#include "innodb_page_sdi_blob.h"
#include "innodb_page_index.h"
#include "innodb_page_lob_index.h"
#include "innodb_page_lob_first.h"
#include "innodb_page_zlob_frag_entry.h"
#include "innodb_page_zlob_frag.h"
#include "innodb_page_sys.h"
#include "innodb_page_ibuf_bitmap.h"
#include "innodb_page_blob.h"
#include "innodb_page_allocated.h"
#include "innodb_page_sdi_zblob.h"
#include "innodb_page_zlob_index.h"
#include "innodb_page_rtree.h"
#include "innodb_page_trx_sys.h"
#include "innodb_page_ibuf_free_list.h"
#include "innodb_page_inode.h"
#include "innodb_page_xdes.h"
#include "innodb_page_zlob_first.h"
#include "innodb_page_zlob_data.h"
#include "innodb_page_zblob.h"
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_tablespace_t : public kaitai::kstruct {

public:
    class page_dispatcher_t_t;
    class page_wrapper_t_t;

    innodb_tablespace_t(uint32_t p_page_size, kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_tablespace_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_tablespace_t();

    /**
     * Dispatcher that parses page based on FIL header page type
     */

    class page_dispatcher_t_t : public kaitai::kstruct {

    public:

        page_dispatcher_t_t(kaitai::kstream* p__io, innodb_tablespace_t::page_wrapper_t_t* p__parent = 0, innodb_tablespace_t* p__root = 0);

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
        innodb_tablespace_t* m__root;
        innodb_tablespace_t::page_wrapper_t_t* m__parent;
        std::string m__raw_page_body;
        kaitai::kstream* m__io__raw_page_body;

    public:

        /**
         * Read FIL header to determine page type
         */
        innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

        /**
         * Page body parsed according to type
         */
        kaitai::kstruct* page_body() const { return m_page_body; }
        innodb_tablespace_t* _root() const { return m__root; }
        innodb_tablespace_t::page_wrapper_t_t* _parent() const { return m__parent; }
        std::string _raw_page_body() const { return m__raw_page_body; }
        kaitai::kstream* _io__raw_page_body() const { return m__io__raw_page_body; }
    };

    /**
     * Wrapper that reads FIL header and dispatches to appropriate page type
     */

    class page_wrapper_t_t : public kaitai::kstruct {

    public:

        page_wrapper_t_t(kaitai::kstream* p__io, innodb_tablespace_t* p__parent = 0, innodb_tablespace_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~page_wrapper_t_t();

    private:
        page_dispatcher_t_t* m_page_data;
        innodb_tablespace_t* m__root;
        innodb_tablespace_t* m__parent;
        std::string m__raw_page_data;
        kaitai::kstream* m__io__raw_page_data;

    public:

        /**
         * Page data (size determined by page_size parameter)
         */
        page_dispatcher_t_t* page_data() const { return m_page_data; }
        innodb_tablespace_t* _root() const { return m__root; }
        innodb_tablespace_t* _parent() const { return m__parent; }
        std::string _raw_page_data() const { return m__raw_page_data; }
        kaitai::kstream* _io__raw_page_data() const { return m__io__raw_page_data; }
    };

private:
    bool f_actual_page_size;
    uint32_t m_actual_page_size;

public:

    /**
     * Actual page size being used
     */
    uint32_t actual_page_size();

private:
    std::vector<page_wrapper_t_t*>* m_pages;
    uint32_t m_page_size;
    innodb_tablespace_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Sequence of pages in tablespace
     */
    std::vector<page_wrapper_t_t*>* pages() const { return m_pages; }

    /**
     * Page size in bytes (default 16KB, can be 4KB, 8KB, 32KB, or 64KB)
     */
    uint32_t page_size() const { return m_page_size; }
    innodb_tablespace_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_TABLESPACE_H_
