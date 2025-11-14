#ifndef INNODB_DOUBLEWRITE_H_
#define INNODB_DOUBLEWRITE_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_doublewrite_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_doublewrite_t : public kaitai::kstruct {

public:
    class dblwr_header_t_t;
    class dblwr_page_t_t;

    innodb_doublewrite_t(uint32_t p_page_size, kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_doublewrite_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_doublewrite_t();

    /**
     * Doublewrite buffer file header.
     */

    class dblwr_header_t_t : public kaitai::kstruct {

    public:

        dblwr_header_t_t(kaitai::kstream* p__io, innodb_doublewrite_t* p__parent = 0, innodb_doublewrite_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~dblwr_header_t_t();

    private:
        std::string m_magic;
        uint32_t m_version;
        uint32_t m_page_size;
        uint32_t m_max_pages;
        std::string m_reserved;
        innodb_doublewrite_t* m__root;
        innodb_doublewrite_t* m__parent;

    public:

        /**
         * Magic number identifying doublewrite buffer file
         */
        std::string magic() const { return m_magic; }

        /**
         * Doublewrite buffer format version
         */
        uint32_t version() const { return m_version; }

        /**
         * Page size for this doublewrite buffer
         */
        uint32_t page_size() const { return m_page_size; }

        /**
         * Maximum number of pages in doublewrite buffer
         */
        uint32_t max_pages() const { return m_max_pages; }

        /**
         * Reserved space (padding to one page)
         */
        std::string reserved() const { return m_reserved; }
        innodb_doublewrite_t* _root() const { return m__root; }
        innodb_doublewrite_t* _parent() const { return m__parent; }
    };

    /**
     * A page copy in the doublewrite buffer.
     * Contains a full page image for recovery purposes.
     */

    class dblwr_page_t_t : public kaitai::kstruct {

    public:

        dblwr_page_t_t(kaitai::kstream* p__io, innodb_doublewrite_t* p__parent = 0, innodb_doublewrite_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~dblwr_page_t_t();

    private:
        bool f_fil_header;
        innodb_common_t::fil_header_t_t* m_fil_header;

    public:

        /**
         * FIL header of the copied page
         */
        innodb_common_t::fil_header_t_t* fil_header();

    private:
        bool f_is_valid;
        bool m_is_valid;

    public:

        /**
         * True if this doublewrite slot contains a valid page
         */
        bool is_valid();

    private:
        std::string m_page_copy;
        innodb_doublewrite_t* m__root;
        innodb_doublewrite_t* m__parent;

    public:

        /**
         * Complete page copy. Can be parsed as any InnoDB page type
         * by reading the FIL header.
         */
        std::string page_copy() const { return m_page_copy; }
        innodb_doublewrite_t* _root() const { return m__root; }
        innodb_doublewrite_t* _parent() const { return m__parent; }
    };

private:
    dblwr_header_t_t* m_dblwr_header;
    std::vector<dblwr_page_t_t*>* m_pages;
    uint32_t m_page_size;
    innodb_doublewrite_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Doublewrite buffer file header
     */
    dblwr_header_t_t* dblwr_header() const { return m_dblwr_header; }

    /**
     * Array of doublewrite pages
     */
    std::vector<dblwr_page_t_t*>* pages() const { return m_pages; }

    /**
     * Page size
     */
    uint32_t page_size() const { return m_page_size; }
    innodb_doublewrite_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_DOUBLEWRITE_H_
