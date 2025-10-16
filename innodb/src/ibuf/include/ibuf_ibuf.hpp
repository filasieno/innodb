/*****************************************************************************

Copyright (c) 1997, 2009, Innobase Oy. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/

/**************************************************//**
@file include/ibuf0ibuf.h
Insert buffer

Created 7/19/1997 Heikki Tuuri
*******************************************************/

#ifndef ibuf0ibuf_h
#define ibuf0ibuf_h

#include "defs.hpp"

#include "mtr_mtr.hpp"
#include "dict_mem.hpp"
#include "fsp_fsp.hpp"

/*********************************************************************//**
Parses a redo log record of an ibuf bitmap page init.
@return	end of log record or NULL */
IB_INTERN
byte*
ibuf_parse_bitmap_init(
/*===================*/
	byte*		ptr,	/*!< in: buffer */
	byte*		end_ptr,/*!< in: buffer end */
	buf_block_t*	block,	/*!< in: block or NULL */
	mtr_t*		mtr);	/*!< in: mtr or NULL */
#ifndef IB_HOTBACKUP
#ifdef IB_IBUF_COUNT_DEBUG
/******************************************************************//**
Gets the ibuf count for a given page.
@return number of entries in the insert buffer currently buffered for
this page */
IB_INTERN
ulint
ibuf_count_get(
/*===========*/
	ulint	space,	/*!< in: space id */
	ulint	page_no);/*!< in: page number */
#endif
/******************************************************************//**
Looks if the insert buffer is empty.
@return	TRUE if empty */
IB_INTERN
ibool
ibuf_is_empty(void);
/*===============*/
/******************************************************************//**
Prints info of ibuf. */
IB_INTERN
void
ibuf_print(
/*=======*/
	ib_stream_t	ib_stram);	/*!< in: stream where to print */
/**********************************************************************
Reset the variables. */
IB_INTERN
void
ibuf_var_init(void);
/*===============*/
/**********************************************************************
Closes insert buffer and frees the data structures. */
IB_INTERN
void
ibuf_close(void);
/*============*/

#define IBUF_HEADER_PAGE_NO	FSP_IBUF_HEADER_PAGE_NO
#define IBUF_TREE_ROOT_PAGE_NO	FSP_IBUF_TREE_ROOT_PAGE_NO

#endif /* !IB_HOTBACKUP */

/* The ibuf header page currently contains only the file segment header
for the file segment from which the pages for the ibuf tree are allocated */
#define IBUF_HEADER		PAGE_DATA
#define	IBUF_TREE_SEG_HEADER	0	/* fseg header for ibuf tree */

/// \brief The insert buffer tree itself is always located in space 0.
constexpr ulint IBUF_SPACE_ID = 0;

#ifndef IB_DO_NOT_INLINE
#include "ibuf0ibuf.inl"
#endif

#endif
