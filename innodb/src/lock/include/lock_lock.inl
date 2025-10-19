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
// Place, Suite 330, Boston, MA 02111-1307 USA


#include "sync_sync.hpp"
#include "srv_srv.hpp"
#include "dict_dict.hpp"
#include "row_row.hpp"
#include "trx_sys.hpp"
#include "trx_trx.hpp"
#include "buf_buf.hpp"
#include "page_page.hpp"
#include "page_cur.hpp"
#include "row_vers.hpp"
#include "que_que.hpp"
#include "btr_cur.hpp"
#include "read_read.hpp"
#include "log_recv.hpp"

IB_INLINE ulint lock_rec_fold(ulint space, ulint page_no)
{
	return ut_fold_ulint_pair(space, page_no);
}

IB_INLINE ulint lock_rec_hash(ulint space, ulint page_no)
{
	return hash_calc_hash(lock_rec_fold(space, page_no), lock_sys->rec_hash);
}

IB_INLINE trx_t* lock_clust_rec_some_has_impl(const rec_t* rec, ib_dict_index_t* dict_index, const ulint* offsets)
{
	trx_id_t trx_id;

	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(dict_index_is_clust(dict_index));
	ut_ad(page_rec_is_user_rec(rec));

	trx_id = row_get_rec_trx_id(rec, dict_index, offsets);

	if (trx_is_active(trx_id)) {
		// The modifying or inserting transaction is active
		return trx_get_on_id(trx_id);
	}

	return NULL;
}

IB_INLINE ulint lock_get_min_heap_no(const buf_block_t* block)
{
	const page_t* page = block->frame;

	if (page_is_comp(page)) {
		return rec_get_heap_no_new(page + rec_get_next_offs(page + PAGE_NEW_INFIMUM, TRUE));
	} else {
		return rec_get_heap_no_old(page + rec_get_next_offs(page + PAGE_OLD_INFIMUM, FALSE));
	}
}
