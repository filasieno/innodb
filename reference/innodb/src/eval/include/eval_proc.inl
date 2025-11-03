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

/// \file eval_proc.inl
/// \brief Executes SQL stored procedures and their control structures
/// \details Originally created by Heikki Tuuri in 1/20/1998
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "pars_pars.hpp"
#include "que_que.hpp"
#include "eval_eval.hpp"

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Performs an execution step of a procedure node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INLINE que_thr_t* proc_step(que_thr_t* thr)
{
	ut_ad(thr);
	proc_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_PROC);
	if (thr->prev_node == que_node_get_parent(node)) {
		// Start execution from the first statement in the statement list. This is the initial entry point when a procedure begins execution.
		thr->run_node = node->stat_list;
	} else {
		// Move to the next statement. When a statement completes, we check if there are more statements to execute in sequence.
		ut_ad(que_node_get_next(thr->prev_node) == NULL);
		thr->run_node = NULL;
	}
	if (thr->run_node == NULL) {
		thr->run_node = que_node_get_parent(node);
	}
	return thr;
}

/// \brief Performs an execution step of a procedure call node.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INLINE que_thr_t* proc_eval_step(que_thr_t* thr)
{
	ut_ad(thr);
	func_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_FUNC);
	// Evaluate the procedure. This executes the function call and stores the result for use in the calling context.
	eval_exp(node);
	thr->run_node = que_node_get_parent(node);
	return thr;
}
