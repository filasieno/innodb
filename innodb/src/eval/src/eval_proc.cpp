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

/// \file eval_proc.cpp
/// \brief Executes SQL stored procedures and their control structures
/// \details Originally created by Heikki Tuuri in 1/20/1998
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "eval_proc.hpp"

#ifdef IB_DO_NOT_INLINE
	#include "eval_proc.inl"
#endif

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Performs an execution step of an if-statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* if_step(que_thr_t* thr)
{
	ut_ad(thr);
	if_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_IF);
	if (thr->prev_node == que_node_get_parent(node)) {
		eval_exp(node->cond);
		if (eval_node_get_ibool_val(node->cond)) {
			thr->run_node = node->stat_list;
		} else if (node->else_part) {
			thr->run_node = node->else_part;
		} else if (node->elsif_list) {
			elsif_node_t* elsif_node = node->elsif_list;
			for (;;) {
				eval_exp(elsif_node->cond);
				if (eval_node_get_ibool_val(elsif_node->cond)) {
					thr->run_node = elsif_node->stat_list;
					break;
				}
				elsif_node = que_node_get_next(elsif_node);
				if (elsif_node == NULL) {
					thr->run_node = NULL;
					break;
				}
			}
		} else {
			thr->run_node = NULL;
		}
	} else {
		ut_ad(que_node_get_next(thr->prev_node) == NULL);
		thr->run_node = NULL;
	}
	if (thr->run_node == NULL) {
		thr->run_node = que_node_get_parent(node);
	}
	return thr;
}

/// \brief Performs an execution step of a while-statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* while_step(que_thr_t* thr)
{
	ut_ad(thr);
	while_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_WHILE);
	ut_ad((thr->prev_node == que_node_get_parent(node)) || (que_node_get_next(thr->prev_node) == NULL));
	eval_exp(node->cond);
	if (eval_node_get_ibool_val(node->cond)) {
		thr->run_node = node->stat_list;
	} else {
		thr->run_node = que_node_get_parent(node);
	}
	return thr;
}

/// \brief Performs an execution step of an assignment statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* assign_step(que_thr_t* thr)
{
	ut_ad(thr);
	assign_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_ASSIGNMENT);
	eval_exp(node->val);
	eval_node_copy_val(node->var->alias, node->val);
	thr->run_node = que_node_get_parent(node);
	return thr;
}

/// \brief Performs an execution step of a for-loop node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* for_step(que_thr_t* thr)
{
	ut_ad(thr);
	for_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_FOR);
	que_node_t* parent = que_node_get_parent(node);
	lint loop_var_value;
	if (thr->prev_node != parent) {
		// Move to the next statement
		thr->run_node = que_node_get_next(thr->prev_node);
		if (thr->run_node != NULL) {
			return thr;
		}
		// Increment the value of loop_var
		loop_var_value = 1 + eval_node_get_int_val(node->loop_var);
	} else {
		// Initialize the loop
		eval_exp(node->loop_start_limit);
		eval_exp(node->loop_end_limit);
		loop_var_value = eval_node_get_int_val(node->loop_start_limit);
		node->loop_end_value = eval_node_get_int_val(node->loop_end_limit);
	}
	// Check if we should do another loop
	if (loop_var_value > node->loop_end_value) {
		// Enough loops done
		thr->run_node = parent;
	} else {
		eval_node_set_int_val(node->loop_var, loop_var_value);
		thr->run_node = node->stat_list;
	}
	return thr;
}

/// \brief Performs an execution step of an exit statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* exit_step(que_thr_t* thr)
{
	ut_ad(thr);
	exit_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_EXIT);
	// Loops exit by setting thr->run_node as the loop node's parent, so find our containing loop node and get its parent. If someone uses an EXIT statement outside of a loop, this will trigger an assertion failure.
	que_node_t* loop_node = que_node_get_containing_loop_node(node);
	// If someone uses an EXIT statement outside of a loop, this will trigger.
	ut_a(loop_node);
	thr->run_node = que_node_get_parent(loop_node);
	return thr;
}

/// \brief Performs an execution step of a return-statement node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* return_step(que_thr_t* thr)
{
	ut_ad(thr);
	return_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_RETURN);
	que_node_t* parent = node;
	while (que_node_get_type(parent) != QUE_NODE_PROC) {
		parent = que_node_get_parent(parent);
	}
	ut_a(parent);
	thr->run_node = que_node_get_parent(parent);
	return thr;
}
