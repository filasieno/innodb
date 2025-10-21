// Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.
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

/// \file btr_sea.inl
/// \brief The index tree adaptive search
/// \details Originally created by Heikki Tuuri in 2/17/1996
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#include "dict_mem.hpp"
#include "btr_cur.hpp"
#include "buf_buf.hpp"

/// \brief Updates the search info.
/// \param [in,out] info search info
/// \param [in] cursor cursor which was just positioned
IB_INTERN void btr_search_info_update_slow(btr_search_t* info, btr_cur_t* cursor);

/// \brief Returns search info for an index.
/// \return search info; search mutex reserved
/// \param [in] dict_index dict_index
IB_INLINE btr_search_t* btr_search_get_info(dict_index_t* dict_index)
{
	ut_ad(dict_index);
	return dict_index->search_info;
}

/// \brief Updates the search info.
/// \param [in] dict_index dict_index of the cursor
/// \param [in] cursor cursor which was just positioned
IB_INLINE void btr_search_info_update(dict_index_t* dict_index, btr_cur_t* cursor)
{
#ifdef IB_SYNC_DEBUG
	ut_ad(!rw_lock_own(&btr_search_latch, RW_LOCK_SHARED));
	ut_ad(!rw_lock_own(&btr_search_latch, RW_LOCK_EX));
#endif /* IB_SYNC_DEBUG */

	btr_search_t* info = btr_search_get_info(dict_index);
	info->hash_analysis++;
	if (info->hash_analysis < BTR_SEARCH_HASH_ANALYSIS) {
		// Do nothing
		return;
	}
	ut_ad(cursor->flag != BTR_CUR_HASH);
	btr_search_info_update_slow(info, cursor);
}
