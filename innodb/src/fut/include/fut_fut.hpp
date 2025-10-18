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

/// \file fut_fut.hpp
/// \brief File-based utilities
/// \details Originally created by Heikki Tuuri in 12/13/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025


#pragma once

#include "univ.i"

#include "fil_fil.hpp"
#include "mtr_mtr.hpp"

/// \brief Gets a pointer to a file address and latches the page.
/// \param [in] space space id
/// \param [in] zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param [in] addr file address
/// \param [in] rw_latch RW_S_LATCH, RW_X_LATCH
/// \param [in] mtr mtr handle
/// \return pointer to a byte in a frame; the file page in the frame is bufferfixed and latched
IB_INLINE byte* fut_get_ptr(ulint space, ulint zip_size, fil_addr_t addr, ulint rw_latch, mtr_t* mtr);

#ifndef IB_DO_NOT_INLINE
#include "fut0fut.inl"
#endif

