// Copyright (c) 1998, 2009, Innobase Oy. All Rights Reserved.
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

/// \file eval_proc.hpp
/// \brief Executes SQL stored procedures and their control structures
/// \details Originally created by Heikki Tuuri in 1/20/1998
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "que_types.hpp"
#include "pars_sym.hpp"
#include "pars_pars.hpp"

/// \brief Performs an execution step of a procedure node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INLINE que_thr_t* proc_step(que_thr_t* thr);

/// \brief Performs an execution step of an if-statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* if_step(que_thr_t* thr);

/// \brief Performs an execution step of a while-statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* while_step(que_thr_t* thr);

/// \brief Performs an execution step of a for-loop node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* for_step(que_thr_t* thr);

/// \brief Performs an execution step of an assignment statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* assign_step(que_thr_t* thr);

/// \brief Performs an execution step of a procedure call node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INLINE que_thr_t* proc_eval_step(que_thr_t* thr);

/// \brief Performs an execution step of an exit statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* exit_step(que_thr_t* thr);

/// \brief Performs an execution step of a return-statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* return_step(que_thr_t* thr);

#ifndef IB_DO_NOT_INLINE
#include "eval_proc.inl"
#endif
