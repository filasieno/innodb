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

/// \file thr_loc.h
/// \brief The thread local storage
/// \details  This module implements storage private to each thread, a capability useful in some situations like storing the OS handle to the current thread, or its priority.
/// Originally created 10/5/1995 Heikki Tuuri

#pragma once

#include "defs.hpp"
#include "os_thread.hpp"

/// \brief Initializes the thread local storage module.
/// \param [in, out] state Pointer to state
IB_INTERN void thr_local_init(innodb_t* state);

/// \brief Close the thread local storage module.
IB_INTERN void thr_local_close(void);

/// \brief Creates a local storage struct for the calling new thread.
IB_INTERN void thr_local_create(void);

/// \brief Frees the local storage struct for the specified thread.
/// \param [in] id Thread id
IB_INTERN void thr_local_free(os_thread_id_t id);

/// \brief Gets the slot number in the thread table of a thread.
/// \param [in] id Thread id
/// \return Slot number
IB_INTERN ulint thr_local_get_slot_no(os_thread_id_t id);	

/// \brief Sets in the local storage the slot number in the thread table of a thread.
/// \param [in] id Thread id
/// \param [in] slot_no Slot number
IB_INTERN void thr_local_set_slot_no(os_thread_id_t id,	ulint slot_no);

/// \brief Returns pointer to the 'in_ibuf' field within the current thread local storage.
/// \return Pointer to the in_ibuf field
IB_INTERN ibool* thr_local_get_in_ibuf_field(void);

#ifndef IB_DO_NOT_INLINE
  #include "thr_loc.inl"
#endif

