// Copyright (c) 1997, 2009, Innobase Oy. All Rights Reserved.
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

/// \file row_umod.hpp
/// \brief Undo modify of a row
/// \details Originally created on 2/27/1997 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "data_data.hpp"
#include "dict_types.hpp"
#include "trx_types.hpp"
#include "que_types.hpp"
#include "row_types.hpp"
#include "mtr_mtr.hpp"

/// \brief Undoes a modify operation on a row of a table.
/// \param node row undo node
/// \param thr query thread
/// \return DB_SUCCESS or error code
IB_INTERN ulint row_undo_mod(undo_node_t* node, que_thr_t* thr);


#ifndef IB_DO_NOT_INLINE
	#include "row_umod.inl"
#endif

