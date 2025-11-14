#ifndef INNODB_PAGE_RTREE_H_
#define INNODB_PAGE_RTREE_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class innodb_page_rtree_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include "innodb_common.h"
#include "innodb_page_index.h"

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
#endif

class innodb_page_rtree_t : public kaitai::kstruct {

public:
    class mbr_t_t;
    class rtree_header_t_t;

    innodb_page_rtree_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_rtree_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~innodb_page_rtree_t();

    /**
     * Minimum Bounding Rectangle coordinates.
     * Format: xmin, xmax, ymin, ymax
     */

    class mbr_t_t : public kaitai::kstruct {

    public:

        mbr_t_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, innodb_page_rtree_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~mbr_t_t();

    private:
        double m_xmin;
        double m_xmax;
        double m_ymin;
        double m_ymax;
        innodb_page_rtree_t* m__root;
        kaitai::kstruct* m__parent;

    public:

        /**
         * Minimum X coordinate
         */
        double xmin() const { return m_xmin; }

        /**
         * Maximum X coordinate
         */
        double xmax() const { return m_xmax; }

        /**
         * Minimum Y coordinate
         */
        double ymin() const { return m_ymin; }

        /**
         * Maximum Y coordinate
         */
        double ymax() const { return m_ymax; }
        innodb_page_rtree_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    /**
     * R-tree specific header information.
     */

    class rtree_header_t_t : public kaitai::kstruct {

    public:

        rtree_header_t_t(kaitai::kstream* p__io, innodb_page_rtree_t* p__parent = 0, innodb_page_rtree_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~rtree_header_t_t();

    private:
        uint16_t m_mbr_count;
        uint16_t m_level;
        std::string m_reserved;
        innodb_page_rtree_t* m__root;
        innodb_page_rtree_t* m__parent;

    public:

        /**
         * Number of MBRs in this node
         */
        uint16_t mbr_count() const { return m_mbr_count; }

        /**
         * Level in R-tree (0 = leaf)
         */
        uint16_t level() const { return m_level; }

        /**
         * Reserved for future use
         */
        std::string reserved() const { return m_reserved; }
        innodb_page_rtree_t* _root() const { return m__root; }
        innodb_page_rtree_t* _parent() const { return m__parent; }
    };

private:
    innodb_common_t::fil_header_t_t* m_fil_header;
    innodb_page_index_t::index_header_t_t* m_index_header;
    rtree_header_t_t* m_rtree_header;
    std::string m_mbr_data;
    innodb_common_t::fil_trailer_t_t* m_fil_trailer;
    innodb_page_rtree_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Standard FIL header (38 bytes)
     */
    innodb_common_t::fil_header_t_t* fil_header() const { return m_fil_header; }

    /**
     * Index page header (same as B-tree)
     */
    innodb_page_index_t::index_header_t_t* index_header() const { return m_index_header; }

    /**
     * R-tree specific header
     */
    rtree_header_t_t* rtree_header() const { return m_rtree_header; }

    /**
     * Minimum Bounding Rectangle (MBR) data and records.
     * Each record contains MBR coordinates and child pointer.
     */
    std::string mbr_data() const { return m_mbr_data; }

    /**
     * Standard FIL trailer (8 bytes)
     */
    innodb_common_t::fil_trailer_t_t* fil_trailer() const { return m_fil_trailer; }
    innodb_page_rtree_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // INNODB_PAGE_RTREE_H_
