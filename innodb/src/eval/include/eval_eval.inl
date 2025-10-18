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

/// \file eval_eval.inl
/// \brief SQL evaluator: evaluates simple data structures, like expressions, in a query graph
/// \details Originally created by Heikki Tuuri in 12/29/1997
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "que_que.hpp"
#include "rem_cmp.hpp"
#include "pars_grm.hpp"

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

/// \brief Evaluates a function node.
/// \param [in] func_node function node
IB_INTERN void eval_func(func_node_t* func_node);

/// \brief Allocate a buffer from global dynamic memory for a value of a que_node.
/// \details NOTE that this memory must be explicitly freed when the query graph is freed. If the node already has allocated buffer, that buffer is freed here. NOTE that this is the only function where dynamic memory should be allocated for a query node val field.
/// \param [in] node query graph node; sets the val field data field to point to the new buffer, and len field equal to size
/// \param [in] size buffer size
/// \return pointer to allocated buffer
IB_INTERN byte* eval_node_alloc_val_buf(que_node_t* node, ulint size);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Allocates a new buffer if needed.
/// \param [in] node query graph node; sets the val field data field to point to the new buffer, and len field equal to size
/// \param [in] size buffer size
/// \return pointer to buffer
IB_INLINE byte* eval_node_ensure_val_buf(que_node_t* node, ulint size)
{
	dfield_t* dfield = que_node_get_val(node);
	dfield_set_len(dfield, size);
	byte* data = dfield_get_data(dfield);
	if (!data || que_node_get_val_buf_size(node) < size) {
		data = eval_node_alloc_val_buf(node, size);
	}
	return data;
}

/// \brief Evaluates a symbol table symbol.
/// \param [in] sym_node symbol table node
IB_INLINE void eval_sym(sym_node_t* sym_node)
{
	ut_ad(que_node_get_type(sym_node) == QUE_NODE_SYMBOL);
	if (sym_node->indirection) {
		// The symbol table node is an alias for a variable or a column
		dfield_copy_data(que_node_get_val(sym_node), que_node_get_val(sym_node->indirection));
	}
}

/// \brief Evaluates an expression.
/// \param [in] exp_node expression
IB_INLINE void eval_exp(que_node_t* exp_node)
{
	if (que_node_get_type(exp_node) == QUE_NODE_SYMBOL) {
		eval_sym((sym_node_t*)exp_node);
		return;
	}
	eval_func(exp_node);
}

/// \brief Sets an integer value as the value of an expression node.
/// \param [in] node expression node
/// \param [in] val value to set
IB_INLINE void eval_node_set_int_val(que_node_t* node, lint val)
{
	dfield_t* dfield = que_node_get_val(node);
	byte* data = dfield_get_data(dfield);
	if (data == NULL) {
		data = eval_node_alloc_val_buf(node, 4);
	}
	ut_ad(dfield_get_len(dfield) == 4);
	mach_write_to_4(data, (ulint)val);
}

/// \brief Gets an integer non-SQL null value from an expression node.
/// \param [in] node expression node
/// \return integer value
IB_INLINE lint eval_node_get_int_val(que_node_t* node)
{
	dfield_t* dfield = que_node_get_val(node);
	ut_ad(dfield_get_len(dfield) == 4);
	return (int)mach_read_from_4(dfield_get_data(dfield));
}

/// \brief Gets a iboolean value from a query node.
/// \param [in] node query graph node
/// \return iboolean value
IB_INLINE ibool eval_node_get_ibool_val(que_node_t* node)
{
	dfield_t* dfield = que_node_get_val(node);
	byte* data = dfield_get_data(dfield);
	ut_ad(data != NULL);
	return mach_read_from_1(data);
}

/// \brief Sets a iboolean value as the value of a function node.
/// \param [in] func_node function node
/// \param [in] val value to set
IB_INLINE void eval_node_set_ibool_val(func_node_t* func_node, ibool val)
{
	dfield_t* dfield = que_node_get_val(func_node);
	byte* data = dfield_get_data(dfield);
	if (data == NULL) {
		// Allocate 1 byte to hold the value
		data = eval_node_alloc_val_buf(func_node, 1);
	}
	ut_ad(dfield_get_len(dfield) == 1);
	mach_write_to_1(data, val);
}

/// \brief Copies a binary string value as the value of a query graph node.
/// \details Allocates a new buffer if necessary.
/// \param [in] node query graph node
/// \param [in] str binary string
/// \param [in] len string length or IB_SQL_NULL
IB_INLINE void eval_node_copy_and_alloc_val(que_node_t* node, const byte* str, ulint len)
{
	if (len == IB_SQL_NULL) {
		dfield_set_len(que_node_get_val(node), len);
		return;
	}
	byte* data = eval_node_ensure_val_buf(node, len);
	ut_memcpy(data, str, len);
}

/// \brief Copies a query node value to another node.
/// \param [in] node1 node to copy to
/// \param [in] node2 node to copy from
IB_INLINE void eval_node_copy_val(que_node_t* node1, que_node_t* node2)
{
	dfield_t* dfield2 = que_node_get_val(node2);
	eval_node_copy_and_alloc_val(node1, dfield_get_data(dfield2), dfield_get_len(dfield2));
}
