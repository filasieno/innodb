/*****************************************************************************

Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.

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
@file include/usr0sess.h
Sessions

Created 6/25/1996 Heikki Tuuri
*******************************************************/

#pragma once

#include "univ.inl"
#include "ut_byte.hpp"
#include "trx_types.hpp"
#include "srv_srv.hpp"
#include "trx_types.hpp"
#include "usr_types.hpp"
#include "que_types.hpp"
#include "data_data.hpp"
#include "rem_rec.hpp"

/// \brief Opens a session.
/// \return A session object
/// \internal
IB_INTERN sess_t* sess_open(void);

/// \brief Closes a session, freeing the memory occupied by it.
/// \param sess Session object
/// \internal
IB_INTERN void sess_close(sess_t* sess);

/// \brief The session handle. 
/// \details All fields are protected by the kernel mutex
/// \internal
struct sess_struct
{
	///  state of the session 
	ulint state;		/*!<  */
	trx_t* trx;		/*!< transaction object permanently
					assigned for the session: the
					transaction instance designated by the
					trx id changes, but the memory
					structure is preserved */
	UT_LIST_BASE_NODE_T(que_t) graphs; /*!< query graphs belonging to this session */
};

/* Session states */
#define SESS_ACTIVE		1

/// Session contains an error message
/// which has not yet been communicated
/// session contains an error message
/// which has not yet been communicated
/// to the client
#define SESS_ERROR		2	/*  */

#ifndef UNIV_NONINL
#include "usr_sess.inl"

#endif

