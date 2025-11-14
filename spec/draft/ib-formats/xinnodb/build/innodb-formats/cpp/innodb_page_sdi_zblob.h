#ifndef INNODB_PAGE_SDI_ZBLOB_H_
#define INNODB_PAGE_SDI_ZBLOB_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_sdi_zblob_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_sdi_zblob_t : public kaitai::kstruct {

public:
    class sdi_zblob_header_t_t;

    innodb_page_sdi_zblob_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_sdi_zblob_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_sdi_zblob_t();

    /**
     * Header for compressed SDI BLOB pages.
     */

    class sdi_zblob_header_t_t : public kaitai::kstruct {

    public:

        sdi_zblob_header_t_t(kaitai::kstream* p__io, innodb_page_sdi_zblob_t* p__parent = 0, innodb_page_sdi_zblob_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~sdi_zblob_header_t_t();

    private:
        uint32_t m_sdi_version;
        uint32_t m_sdi_type;
        uint64_t m_sdi_id;
        uint32_t m_compressed_len;
        uint32_t m_uncompressed_len;
        uint32_t m_next_page_no;
        innodb_page_sdi_zblob_t* m__root;
        innodb_page_sdi_zblob_t* m__parent;

    public:

        /**
         * SDI version
         */
        uint32_t sdi_version() const { return m_sdi_version; }

        /**
         * SDI object type
         */
        uint32_t sdi_type() const { return m_sdi_type; }

        /**
         * Object ID
         */
        uint64_t sdi_id() const { return m_sdi_id; }

        /**
         * Compressed length
         */
        uint32_t compressed_len() const { return m_compressed_len; }

        /**
         * Uncompressed length
         */
        uint32_t uncompressed_len() const { return m_uncompressed_len; }

        /**
         * Next page (0xFFFFFFFF = last)
         */
        uint32_t next_page_no() const { return m_next_page_no; }
        innodb_page_sdi_zblob_t* _root() const { return m__root; }
        innodb_page_sdi_zblob_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    sdi_zblob_header_t_t* m_sdi_zblob_header;
    std::string m_compressed_sdi_data;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_sdi_zblob_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Standard FIL header (38 bytes)
     */
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

    /**
     * Compressed SDI header
     */
    sdi_zblob_header_t_t* sdi_zblob_header() const { return m_sdi_zblob_header; }

    /**
     * Compressed SDI JSON data (zlib)
     */
    std::string compressed_sdi_data() const { return m_compressed_sdi_data; }

    /**
     * Standard FIL trailer (8 bytes)
     */
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_sdi_zblob_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_SDI_ZBLOB_H_
