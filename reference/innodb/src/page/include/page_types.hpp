// Copyright (c) 1994, 2009, Innobase Oy. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; version 2 of the License.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA

/// \file page_types.hpp
/// \brief Index page routines
/// \details Originally created by Heikki Tuuri in 2/2/1994
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#pragma once

#include "univ.i"
#include "dict_types.hpp"
#include "mtr_types.hpp"

/** Eliminates a name collision on HP-UX */
#define page_t ib_page_t

/** Type of the index page */
typedef	byte page_t;

/** Index page cursor */
typedef struct page_cur_struct	page_cur_t;

/** Compressed index page */
typedef byte page_zip_t;

/** Compressed page descriptor */
typedef struct page_zip_des_struct	page_zip_des_t;

/* The following definitions would better belong to page0zip.h,
but we cannot include page0zip.h from rem0rec.ic, because
page0*.h includes rem0rec.h and may include rem0rec.ic. */


/** Number of supported compressed page sizes */
#define PAGE_ZIP_NUM_SSIZE (IB_PAGE_SIZE_SHIFT - PAGE_ZIP_MIN_SIZE_SHIFT + 2)
#if PAGE_ZIP_NUM_SSIZE > (1 << PAGE_ZIP_SSIZE_BITS)
# error "PAGE_ZIP_NUM_SSIZE > (1 << PAGE_ZIP_SSIZE_BITS)"
#endif

/** Compressed page descriptor */
struct page_zip_des_struct
{
	page_zip_t*	data;		/*!< compressed page data */

#ifdef IB_DEBUG
	unsigned	m_start:16;	/*!< start offset of modification log */
#endif /* IB_DEBUG */
	unsigned	m_end:16;	/*!< end offset of modification log */
	unsigned	m_nonempty:1;	/*!< TRUE if the modification log is not empty */
	unsigned	n_blobs:12;	/*!< number of externally stored columns on the page; the maximum is 744 on a 16 KiB page */
	unsigned	ssize:PAGE_ZIP_SSIZE_BITS; /*!< 0 or compressed page size; the size in bytes is PAGE_ZIP_MIN_SIZE << (ssize - 1). */
};

/** Compression statistics for a given page size */
struct page_zip_stat_struct {
	/** Number of page compressions */
	ulint		compressed;
	/** Number of successful page compressions */
	ulint		compressed_ok;
	/** Number of page decompressions */
	ulint		decompressed;
	/** Duration of page compressions in microseconds */
	ib_uint64_t	compressed_usec;
	/** Duration of page decompressions in microseconds */
	ib_uint64_t	decompressed_usec;
};

/** Compression statistics */
typedef struct page_zip_stat_struct page_zip_stat_t;

/** Statistics on compression, indexed by page_zip_des_struct::ssize - 1 */
extern page_zip_stat_t page_zip_stat[PAGE_ZIP_NUM_SSIZE - 1];


