// Copyright (c) 2007, 2009, Innobase Oy. All Rights Reserved.
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

/// \file lock_iter.hpp
/// \brief Lock queue iterator type and function prototypes.
/// \details Originally created by Vasil Dimov on July 16, 2007.
/// \author Fabio N. Filasieno
/// \date 2025-10-20

#pragma once

#include "defs.hpp"
#include "lock_types.hpp"

/// \brief Initialize lock queue iterator so that it starts to iterate from lock.
/// \details bit_no specifies the record number within the heap where the record is stored. It can be undefined (ULINT_UNDEFINED) in two cases: 1) table lock queue; 2) record wait lock where bit_no is derived via lock_rec_find_set_bit().
/// \param [out] iter iterator
/// \param [in] lock lock to start from
/// \param [in] bit_no record number in the heap
IB_INTERN void lock_queue_iterator_reset(ib_lock_queue_iterator_t* iter, const ib_lock_t* lock, ulint bit_no);

/// \brief Gets the previous lock in the lock queue.
/// \details Returns NULL if there are no more locks (i.e. the current lock is the first one). The iterator is receded if a non-NULL value is returned.
/// \param [in,out] iter iterator
/// \return previous lock or NULL
const ib_lock_t* lock_queue_iterator_get_prev(ib_lock_queue_iterator_t* iter);

