#ifndef INNODB_PAGE_SDI_BLOB_H_
#define INNODB_PAGE_SDI_BLOB_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_sdi_blob_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_sdi_blob_t : public kaitai::kstruct {

public:
    class sdi_header_t_t;

    innodb_page_sdi_blob_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_sdi_blob_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_sdi_blob_t();

    /**
     * Header for SDI BLOB pages.
     */

    class sdi_header_t_t : public kaitai::kstruct {

    public:

        sdi_header_t_t(kaitai::kstream* p__io, innodb_page_sdi_blob_t* p__parent = 0, innodb_page_sdi_blob_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~sdi_header_t_t();

    private:
        uint32_t m_sdi_version;
        uint32_t m_sdi_type;
        uint64_t m_sdi_id;
        uint32_t m_data_len;
        uint32_t m_next_page_no;
        innodb_page_sdi_blob_t* m__root;
        innodb_page_sdi_blob_t* m__parent;

    public:

        /**
         * SDI version number
         */
        uint32_t sdi_version() const { return m_sdi_version; }

        /**
         * Type of SDI object (table, tablespace, etc.)
         */
        uint32_t sdi_type() const { return m_sdi_type; }

        /**
         * Object ID
         */
        uint64_t sdi_id() const { return m_sdi_id; }

        /**
         * Length of JSON data
         */
        uint32_t data_len() const { return m_data_len; }

        /**
         * Next SDI BLOB page (0xFFFFFFFF = last)
         */
        uint32_t next_page_no() const { return m_next_page_no; }
        innodb_page_sdi_blob_t* _root() const { return m__root; }
        innodb_page_sdi_blob_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    sdi_header_t_t* m_sdi_header;
    std::string m_sdi_json_data;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_sdi_blob_t* m__root;
    kaitai::kstruct* m__parent;

public:
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }
    sdi_header_t_t* sdi_header() const { return m_sdi_header; }
    std::string sdi_json_data() const { return m_sdi_json_data; }
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_sdi_blob_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_SDI_BLOB_H_
