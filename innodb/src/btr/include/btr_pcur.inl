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

/// \file btr_pcur.inl
/// \brief The index tree persistent cursor
/// \details Originally created by Heikki Tuuri on 2/23/1996
/// \author Fabio N. Filasieno
/// \date 21/10/2025


IB_INLINE ulint btr_pcur_get_rel_pos(const btr_pcur_t* cursor);
{
	ut_ad(cursor);
	ut_ad(cursor->old_rec);
	ut_ad(cursor->old_stored == BTR_PCUR_OLD_STORED);
    ut_ad(cursor->pos_state == BTR_PCUR_WAS_POSITIONED || cursor->pos_state == BTR_PCUR_IS_POSITIONED);
    return cursor->rel_pos;
}

IB_INLINE void btr_pcur_set_mtr(btr_pcur_t* cursor, mtr_t* mtr);
{
	ut_ad(cursor);
	cursor->mtr = mtr;
}

IB_INLINE mtr_t* btr_pcur_get_mtr(btr_pcur_t* cursor);
{
	ut_ad(cursor);
    return cursor->mtr;
}

#ifdef IB_DEBUG
IB_INLINE btr_cur_t* btr_pcur_get_btr_cur(const btr_pcur_t* cursor);
{
    const btr_cur_t* btr_cur = &cursor->btr_cur;
    return (btr_cur_t*)btr_cur;
}

IB_INLINE page_cur_t* btr_pcur_get_page_cur(const btr_pcur_t* cursor);
{
    return btr_cur_get_page_cur(btr_pcur_get_btr_cur(cursor));
}
#endif /* IB_DEBUG */
IB_INLINE page_t* btr_pcur_get_page(btr_pcur_t* cursor);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
    return btr_cur_get_page(btr_pcur_get_btr_cur(cursor));
}

IB_INLINE buf_block_t* btr_pcur_get_block(btr_pcur_t* cursor);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
    return btr_cur_get_block(btr_pcur_get_btr_cur(cursor));
}

IB_INLINE rec_t* btr_pcur_get_rec(btr_pcur_t* cursor);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
    return btr_cur_get_rec(btr_pcur_get_btr_cur(cursor));
}

IB_INLINE ulint btr_pcur_get_up_match(btr_pcur_t* cursor);
{
    btr_cur_t* btr_cursor;
    ut_ad((cursor->pos_state == BTR_PCUR_WAS_POSITIONED) || (cursor->pos_state == BTR_PCUR_IS_POSITIONED));
	btr_cursor = btr_pcur_get_btr_cur(cursor);
	ut_ad(btr_cursor->up_match != ULINT_UNDEFINED);
    return btr_cursor->up_match;
}

IB_INLINE ulint btr_pcur_get_low_match(btr_pcur_t* cursor);
{
    btr_cur_t* btr_cursor;
    ut_ad((cursor->pos_state == BTR_PCUR_WAS_POSITIONED) || (cursor->pos_state == BTR_PCUR_IS_POSITIONED));
	btr_cursor = btr_pcur_get_btr_cur(cursor);
	ut_ad(btr_cursor->low_match != ULINT_UNDEFINED);
    return btr_cursor->low_match;
}

IB_INLINE ibool btr_pcur_is_after_last_on_page(const btr_pcur_t* cursor);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
    return page_cur_is_after_last(btr_pcur_get_page_cur(cursor));
}

IB_INLINE ibool btr_pcur_is_before_first_on_page(const btr_pcur_t* cursor);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
    return page_cur_is_before_first(btr_pcur_get_page_cur(cursor));
}

IB_INLINE ibool btr_pcur_is_on_user_rec(const btr_pcur_t* cursor);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
    if (btr_pcur_is_before_first_on_page(cursor) || btr_pcur_is_after_last_on_page(cursor)) {
        return FALSE;
    }
    return TRUE;
}

IB_INLINE ibool btr_pcur_is_before_first_in_tree(btr_pcur_t* cursor, mtr_t* mtr);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	if (btr_page_get_prev(btr_pcur_get_page(cursor), mtr) != FIL_NULL) {
        return FALSE;
    }
    return page_cur_is_before_first(btr_pcur_get_page_cur(cursor));
}

IB_INLINE ibool btr_pcur_is_after_last_in_tree(btr_pcur_t* cursor, mtr_t* mtr);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	if (btr_page_get_next(btr_pcur_get_page(cursor), mtr) != FIL_NULL) {
        return FALSE;
    }
    return page_cur_is_after_last(btr_pcur_get_page_cur(cursor));
}

IB_INLINE void btr_pcur_move_to_next_on_page(btr_pcur_t* cursor);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	page_cur_move_to_next(btr_pcur_get_page_cur(cursor));
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
}

IB_INLINE void btr_pcur_move_to_prev_on_page(btr_pcur_t* cursor);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	page_cur_move_to_prev(btr_pcur_get_page_cur(cursor));
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
}

IB_INLINE void btr_pcur_move_to_last_on_page(btr_pcur_t* cursor, mtr_t* mtr);
{
	UT_NOT_USED(mtr);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
    page_cur_set_after_last(btr_pcur_get_block(cursor), btr_pcur_get_page_cur(cursor));
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
}

IB_INLINE ibool btr_pcur_move_to_next_user_rec(btr_pcur_t* cursor, mtr_t* mtr);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
loop:
	if (btr_pcur_is_after_last_on_page(cursor)) {
		if (btr_pcur_is_after_last_in_tree(cursor, mtr)) {
            return FALSE;
		}
		btr_pcur_move_to_next_page(cursor, mtr);
	} else {
		btr_pcur_move_to_next_on_page(cursor);
	}
	if (btr_pcur_is_on_user_rec(cursor)) {
        return TRUE;
	}
	goto loop;
}

IB_INLINE ibool btr_pcur_move_to_prev_user_rec(btr_pcur_t* cursor, mtr_t* mtr);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
loop:
	if (btr_pcur_is_before_first_on_page(cursor)) {
		if (btr_pcur_is_before_first_in_tree(cursor, mtr)) {
            return FALSE;
		}
		btr_pcur_move_backward_from_page(cursor, mtr);
	} else {
		btr_pcur_move_to_prev_on_page(cursor);
	}
	if (btr_pcur_is_on_user_rec(cursor)) {
        return TRUE;
	}
	goto loop;
}

IB_INLINE ibool btr_pcur_move_to_next(btr_pcur_t* cursor, mtr_t* mtr);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
	if (btr_pcur_is_after_last_on_page(cursor)) {
		if (btr_pcur_is_after_last_in_tree(cursor, mtr)) {
            return FALSE;
		}
		btr_pcur_move_to_next_page(cursor, mtr);
        return TRUE;
	}
	btr_pcur_move_to_next_on_page(cursor);
    return TRUE;
}

IB_INLINE ibool btr_pcur_move_to_prev(btr_pcur_t* cursor, mtr_t* mtr);
{
	ut_ad(cursor->pos_state == BTR_PCUR_IS_POSITIONED);
	ut_ad(cursor->latch_mode != BTR_NO_LATCHES);
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
	if (btr_pcur_is_before_first_on_page(cursor)) {
		if (btr_pcur_is_before_first_in_tree(cursor, mtr)) {
            return FALSE;
		}
		btr_pcur_move_backward_from_page(cursor, mtr);
        return TRUE;
	}
	btr_pcur_move_to_prev_on_page(cursor);
    return TRUE;
}

IB_INLINE void btr_pcur_commit_specify_mtr(btr_pcur_t* pcur, mtr_t* mtr);
{
	ut_a(pcur->pos_state == BTR_PCUR_IS_POSITIONED);
	pcur->latch_mode = BTR_NO_LATCHES;
	mtr_commit(mtr);
	pcur->pos_state = BTR_PCUR_WAS_POSITIONED;
}

IB_INLINE void btr_pcur_detach(btr_pcur_t* pcur);
{
	ut_a(pcur->pos_state == BTR_PCUR_IS_POSITIONED);
	pcur->latch_mode = BTR_NO_LATCHES;
	pcur->pos_state = BTR_PCUR_WAS_POSITIONED;
}

IB_INLINE ibool btr_pcur_is_detached(btr_pcur_t* pcur);
{
	if (pcur->latch_mode == BTR_NO_LATCHES) {
        return TRUE;
    }
    return FALSE;
}

IB_INLINE void btr_pcur_init(btr_pcur_t* pcur);
{
	pcur->old_stored = BTR_PCUR_OLD_NOT_STORED;
	pcur->old_rec_buf = NULL;
	pcur->old_rec = NULL;
}

IB_INLINE void btr_pcur_open_func(dict_index_t* dict_index, const dtuple_t* tuple, ulint mode, ulint latch_mode, btr_pcur_t* cursor, const char* file, ulint line, mtr_t* mtr);
{
    btr_cur_t* btr_cursor;
    // Initialize the cursor
	btr_pcur_init(cursor);
	cursor->latch_mode = latch_mode;
	cursor->search_mode = mode;
    // Search with the tree cursor
	btr_cursor = btr_pcur_get_btr_cur(cursor);
    btr_cur_search_to_nth_level(dict_index, 0, tuple, mode, latch_mode, btr_cursor, 0, file, line, mtr);
	cursor->pos_state = BTR_PCUR_IS_POSITIONED;
	cursor->trx_if_known = NULL;
}

IB_INLINE void btr_pcur_open_with_no_init_func(dict_index_t* dict_index, const dtuple_t* tuple, ulint mode, ulint latch_mode, btr_pcur_t* cursor, ulint has_search_latch, const char* file, ulint line, mtr_t* mtr);
{
    btr_cur_t* btr_cursor;
	cursor->latch_mode = latch_mode;
	cursor->search_mode = mode;
    // Search with the tree cursor
	btr_cursor = btr_pcur_get_btr_cur(cursor);
    btr_cur_search_to_nth_level(dict_index, 0, tuple, mode, latch_mode, btr_cursor, has_search_latch, file, line, mtr);
	cursor->pos_state = BTR_PCUR_IS_POSITIONED;
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
	cursor->trx_if_known = NULL;
}

IB_INLINE void btr_pcur_open_at_index_side(ibool from_left, dict_index_t* dict_index, ulint latch_mode, btr_pcur_t* pcur, ibool do_init, mtr_t* mtr);
{
	pcur->latch_mode = latch_mode;
	if (from_left) {
		pcur->search_mode = PAGE_CUR_G;
	} else {
		pcur->search_mode = PAGE_CUR_L;
	}
	if (do_init) {
		btr_pcur_init(pcur);
	}
    btr_cur_open_at_index_side(from_left, dict_index, latch_mode, btr_pcur_get_btr_cur(pcur), mtr);
	pcur->pos_state = BTR_PCUR_IS_POSITIONED;
	pcur->old_stored = BTR_PCUR_OLD_NOT_STORED;
	pcur->trx_if_known = NULL;
}

IB_INLINE void btr_pcur_open_at_rnd_pos_func(dict_index_t* dict_index, ulint latch_mode, btr_pcur_t* cursor, const char* file, ulint line, mtr_t* mtr);
{
    // Initialize the cursor
	cursor->latch_mode = latch_mode;
	cursor->search_mode = PAGE_CUR_G;
	btr_pcur_init(cursor);
    btr_cur_open_at_rnd_pos_func(dict_index, latch_mode, btr_pcur_get_btr_cur(cursor), file, line, mtr);
	cursor->pos_state = BTR_PCUR_IS_POSITIONED;
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
	cursor->trx_if_known = NULL;
}

IB_INLINE void btr_pcur_close(btr_pcur_t* cursor);
{
	if (cursor->old_rec_buf != NULL) {
		mem_free(cursor->old_rec_buf);
		cursor->old_rec = NULL;
		cursor->old_rec_buf = NULL;
	}
	cursor->btr_cur.page_cur.rec = NULL;
	cursor->btr_cur.page_cur.block = NULL;
	cursor->old_rec = NULL;
	cursor->old_stored = BTR_PCUR_OLD_NOT_STORED;
	cursor->latch_mode = BTR_NO_LATCHES;
	cursor->pos_state = BTR_PCUR_NOT_POSITIONED;
	cursor->trx_if_known = NULL;
}
