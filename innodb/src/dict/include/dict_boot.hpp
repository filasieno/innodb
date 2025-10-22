// Copyright (c) 1996, 2010, Innobase Oy. All Rights Reserved.
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

/// \file dict_boot.hpp
/// \brief Data dictionary creation and booting
/// \details Originally created by Heikki Tuuri in 4/18/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"

#include "mtr_mtr.hpp"
#include "mtr_log.hpp"
#include "ut_byte.hpp"
#include "buf_buf.hpp"
#include "fsp_fsp.hpp"
#include "dict_dict.hpp"

typedef	byte dict_hdr_t;

// Space id and page no where the dictionary header resides

constinit ulint DICT_HDR_SPACE   = 0;                    // the SYSTEM tablespace 
constinit ulint DICT_HDR_PAGE_NO = FSP_DICT_HDR_PAGE_NO;

/// \defgroup dict_header_offsets Dictionary Header Offsets
/// \brief Byte offsets for various fields in the dictionary header page
/// \details These constants define the byte positions of different data structures
///          stored in the dictionary header page (page 0 of the system tablespace).
///          The dictionary header contains metadata about the data dictionary itself,
///          including root page numbers for various index trees and ID counters.
/// @{

/// \brief Offset for the latest assigned row ID
constinit ulint DICT_HDR_ROW_ID = 0;

/// \brief Offset for the latest assigned table ID
constinit ulint DICT_HDR_TABLE_ID = 8;

/// \brief Offset for the latest assigned index ID
constinit ulint DICT_HDR_INDEX_ID = 16;

/// \brief Offset for obsolete mix ID field (always 0)
constinit ulint DICT_HDR_MIX_ID = 24;

/// \brief Offset for root page of the table index tree
constinit ulint DICT_HDR_TABLES = 32;

/// \brief Offset for root page of the table ID index tree
constinit ulint DICT_HDR_TABLE_IDS = 36;

/// \brief Offset for root page of the column index tree
constinit ulint DICT_HDR_COLUMNS = 40;

/// \brief Offset for root page of the index index tree
constinit ulint DICT_HDR_INDEXES = 44;

/// \brief Offset for root page of the index field index tree
constinit ulint DICT_HDR_FIELDS = 48;

/// \brief Offset for segment header of the tablespace segment containing the dictionary header
constinit ulint DICT_HDR_FSEG_HEADER = 56;

/// @}


// The field number of the page number field in the sys_indexes table clustered index 

constinit ulint DICT_SYS_INDEXES_PAGE_NO_FIELD = 8;
constinit ulint DICT_SYS_INDEXES_SPACE_NO_FIELD = 7;
constinit ulint DICT_SYS_INDEXES_TYPE_FIELD = 6;
constinit ulint DICT_SYS_INDEXES_NAME_FIELD = 4;

// When a row id which is zero modulo this number (which must be a power of two) is assigned, the field DICT_HDR_ROW_ID on the dictionary header page is updated
constinit ulint DICT_HDR_ROW_ID_WRITE_MARGIN = 256;


/// \brief Gets a pointer to the dictionary header and x-latches its page.
/// \return pointer to the dictionary header, page x-latched
/// \param [in] mtr mtr
IB_INTERN dict_hdr_t* dict_hdr_get(mtr_t* mtr);

/// \brief Returns a new row, table, index, or tree id.
/// \return the new id
/// \param [in] type DICT_HDR_ROW_ID, ...
IB_INTERN dulint dict_hdr_get_new_id(ulint type);

/// \brief Returns a new row id.
/// \return the new id
IB_INLINE dulint dict_sys_get_new_row_id(void);

/// \brief Reads a row id from a record or other 6-byte stored form.
/// \return row id
/// \param [in] field record field
IB_INLINE dulint dict_sys_read_row_id(byte* field);

/// \brief Writes a row id to a record or other 6-byte stored form.
/// \param [in] field record field
/// \param [in] row_id row id
IB_INLINE void dict_sys_write_row_id(byte* field, dulint row_id);

/// \brief Initializes the data dictionary memory structures when the database is started.
/// \details This function is also called when the data dictionary is created.
IB_INTERN void dict_boot(void);

/// \brief Creates and initializes the data dictionary at the database creation.
IB_INTERN void dict_create(void);

// The ids for the basic system tables and their indexes
#define DICT_TABLES_ID		ut_dulint_create(0, 1)
#define DICT_COLUMNS_ID		ut_dulint_create(0, 2)
#define DICT_INDEXES_ID		ut_dulint_create(0, 3)
#define DICT_FIELDS_ID		ut_dulint_create(0, 4)
// The following is a secondary index on SYS_TABLES 
#define DICT_TABLE_IDS_ID	ut_dulint_create(0, 5)

/// \brief The ids for tables etc. start from this number, except for basic system tables and their above defined indexes; ibuf tables and indexes are assigned as the id the number DICT_IBUF_ID_MIN plus the space id
constinit ulint DICT_HDR_FIRST_ID = 10;	

#define DICT_IBUF_ID_MIN	ut_dulint_create(0xFFFFFFFFUL, 0)

// The offset of the dictionary header on the page 
#define	DICT_HDR FSEG_PAGE_DATA

#ifndef IB_DO_NOT_INLINE
	#include "dict_boot.inl"
#endif
