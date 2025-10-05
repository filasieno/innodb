// Copyright (c) 1997, 2009, Innobase Oy. All Rights Reserved.
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; version 2 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA

// @file include/row_sel.inl
// Select
// Created 12/19/1997 Heikki Tuuri

#include "que_que.hpp"

/// \brief Gets the plan node for the nth table in a join.
/// \return plan node
/// \param node select node
/// \param i get ith plan node
UNIV_INLINE
plan_t *sel_node_get_nth_plan(sel_node_t *node, ulint i)
{
    ut_ad(i < node->n_tables);

    return (node->plans + i);
}

/// \brief Resets the cursor defined by sel_node to the SEL_NODE_OPEN state, which means
/// that it will start fetching from the start of the result set again, regardless
/// of where it was before, and it will set intention locks on the tables.
/// \param node select node
UNIV_INLINE
void sel_node_reset_cursor(sel_node_t *node)
{
    node->state = SEL_NODE_OPEN;
}

/// \brief Performs an execution step of an open or close cursor statement node.
/// \return query thread to run next or NULL
/// \param thr query thread
UNIV_INLINE
que_thr_t *open_step(que_thr_t *thr)
{
    sel_node_t *sel_node;
    open_node_t *node;
    ulint err;

    ut_ad(thr);

    node = (open_node_t *)thr->run_node;
    ut_ad(que_node_get_type(node) == QUE_NODE_OPEN);

    sel_node = node->cursor_def;

    err = DB_SUCCESS;

    if (node->op_type == ROW_SEL_OPEN_CURSOR) {

        // if (sel_node->state == SEL_NODE_CLOSED) {

        sel_node_reset_cursor(sel_node);
        // } else {
        // err = DB_ERROR;
        // }
    } else {
        if (sel_node->state != SEL_NODE_CLOSED) {

            sel_node->state = SEL_NODE_CLOSED;
        } else {
            err = DB_ERROR;
        }
    }

    if (UNIV_EXPECT(err, DB_SUCCESS) != DB_SUCCESS) {
        // SQL error detected
        ib_logger(ib_stream, "SQL error %lu\n", (ulong)err);

        ut_error;
    }

    thr->run_node = que_node_get_parent(node);

    return (thr);
}
