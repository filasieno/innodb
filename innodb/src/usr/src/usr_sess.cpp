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

/// \file usr_sess.c
/// \brief Sessions
/// \details Originally created created 6/25/1996 Heikki Tuuri

#include "usr_sess.hpp"

#include "trx_trx.hpp"

IB_INTERN ib_sess *sess_open(innodb_state* state)
{
	ut_ad(mutex_own(&kernel_mutex));
	ib_sess* sess = mem_alloc(sizeof(ib_sess));
	sess->state = SESS_ACTIVE;
	sess->trx = trx_create(sess);
	UT_LIST_INIT(sess->graphs);
	return sess;
}

IB_INTERN void sess_close(innodb_state* state, ib_sess *sess)
{
	ut_ad(!mutex_own(&kernel_mutex));
	ut_a(UT_LIST_GET_LEN(sess->graphs) == 0);
	trx_free_for_background(sess->trx);
	mem_free(sess);
}
