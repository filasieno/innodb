// Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.
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

/// \file fsp_types.hpp
/// \brief File space management types
/// \details Originally created by Vasil Dimov in May 26, 2009
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"

#include "fil_fil.hpp" /* for FIL_PAGE_DATA */

/** @name Flags for inserting records in order
If records are inserted in order, there are the following
flags to tell this (their type is made byte for the compiler
to warn if direction and hint parameters are switched in
fseg_alloc_free_page) */
/* @{ */
constinit ulint FSP_UP = 111;		/*!< alphabetically upwards */
constinit ulint FSP_DOWN = 112;		/*!< alphabetically downwards */
constinit ulint FSP_NO_DIR = 113;	/*!< no order */
/* @} */

/** File space extent size (one megabyte) in pages */
constinit ulint FSP_EXTENT_SIZE = (1 << (20 - IB_PAGE_SIZE_SHIFT));

/** On a page of any file segment, data may be put starting from this
offset */
constinit ulint FSEG_PAGE_DATA = FIL_PAGE_DATA;

/** @name File segment header
The file segment header points to the inode describing the file segment. */
/* @{ */
/** Data type for file segment header */
typedef	byte	fseg_header_t;

constinit ulint FSEG_HDR_SPACE = 0;		/*!< space id of the inode */
constinit ulint FSEG_HDR_PAGE_NO = 4;	/*!< page number of the inode */
constinit ulint FSEG_HDR_OFFSET = 8;		/*!< byte offset of the inode */

constinit ulint FSEG_HEADER_SIZE = 10;	/*!< Length of the file system
					header, in bytes */
/* @} */

/** Flags for fsp_reserve_free_extents @{ */
constinit ulint FSP_NORMAL = 1000000;
constinit ulint FSP_UNDO = 2000000;
constinit ulint FSP_CLEANING = 3000000;
/* @} */

/* Number of pages described in a single descriptor page: currently each page
description takes less than 1 byte; a descriptor page is repeated every
this many file pages */
/* #define XDES_DESCRIBED_PER_PAGE		IB_PAGE_SIZE */
/* This has been replaced with either IB_PAGE_SIZE or page_zip->size. */

/** @name The space low address page map
The pages at FSP_XDES_OFFSET and FSP_IBUF_BITMAP_OFFSET are repeated
every XDES_DESCRIBED_PER_PAGE pages in every tablespace. */
/* @{ */
/*--------------------------------------*/
constinit ulint FSP_XDES_OFFSET = 0;		/* !< extent descriptor */
constinit ulint FSP_IBUF_BITMAP_OFFSET = 1;	/* !< insert buffer bitmap */
				/* The ibuf bitmap pages are the ones whose
				page number is the number above plus a
				multiple of XDES_DESCRIBED_PER_PAGE */

constinit ulint FSP_FIRST_INODE_PAGE_NO = 2;	/*!< in every tablespace */
				/* The following pages exist
				in the system tablespace (space 0). */
constinit ulint FSP_IBUF_HEADER_PAGE_NO = 3;	/*!< insert buffer
						header page, in
						tablespace 0 */
constinit ulint FSP_IBUF_TREE_ROOT_PAGE_NO = 4;	/*!< insert buffer
						B-tree root page in
						tablespace 0 */
				/* The ibuf tree root page number in
				tablespace 0; its fseg inode is on the page
				number FSP_FIRST_INODE_PAGE_NO */
constinit ulint FSP_TRX_SYS_PAGE_NO = 5;	/*!< transaction
						system header, in
						tablespace 0 */
constinit ulint FSP_FIRST_RSEG_PAGE_NO = 6;	/*!< first rollback segment
						page, in tablespace 0 */
constinit ulint FSP_DICT_HDR_PAGE_NO = 7;	/*!< data dictionary header
						page, in tablespace 0 */
/*--------------------------------------*/
/* @} */

