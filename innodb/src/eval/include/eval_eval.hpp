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

/// \file eval_eval.hpp
/// \brief SQL evaluator: evaluates simple data structures, like expressions, in a query graph
/// \details Originally created by Heikki Tuuri in 12/29/1997
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "defs.hpp"
#include "que_types.hpp"
#include "pars_sym.hpp"
#include "pars_pars.hpp"

/// \brief Free the buffer from global dynamic memory for a value of a que_node, if it has been allocated in the above function.
/// \details The freeing for pushed column values is done in sel_col_prefetch_buf_free.
/// \param [in] node query graph node
IB_INTERN void eval_node_free_val_buf(que_node_t* node);

/// \brief Evaluates a symbol table symbol.
/// \param [in] sym_node symbol table node
IB_INLINE void eval_sym(sym_node_t* sym_node);

/// \brief Evaluates an expression.
/// \param [in] exp_node expression
IB_INLINE void eval_exp(que_node_t* exp_node);

/// \brief Sets an integer value as the value of an expression node.
/// \param [in] node expression node
/// \param [in] val value to set
IB_INLINE void eval_node_set_int_val(que_node_t* node, lint val);

/// \brief Gets an integer value from an expression node.
/// \param [in] node expression node
/// \return integer value
IB_INLINE lint eval_node_get_int_val(que_node_t* node);

/// \brief Copies a binary string value as the value of a query graph node.
/// \details Allocates a new buffer if necessary.
/// \param [in] node query graph node
/// \param [in] str binary string
/// \param [in] len string length or IB_SQL_NULL
IB_INLINE void eval_node_copy_and_alloc_val(que_node_t* node, const byte* str, ulint len);

/// \brief Copies a query node value to another node.
/// \param [in] node1 node to copy to
/// \param [in] node2 node to copy from
IB_INLINE void eval_node_copy_val(que_node_t* node1, que_node_t* node2);

/// \brief Gets a iboolean value from a query node.
/// \param [in] node query graph node
/// \return iboolean value
IB_INLINE ibool eval_node_get_ibool_val(que_node_t* node);

/// \brief Evaluates a comparison node.
/// \param [in] cmp_node comparison node
/// \return the result of the comparison
IB_INTERN ibool eval_cmp(func_node_t* cmp_node);

#ifndef IB_DO_NOT_INLINE
	#include "eval_eval.inl"
#endif
