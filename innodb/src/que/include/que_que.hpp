// Copyright (c) 1996, 2010, Innobase Oy. All Rights Reserved.
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

/// \file que_que.hpp
/// \brief Query graph
/// \details Originally created by Heikki Tuuri on 5/27/1996
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#pragma once

#include "univ.i"
#include "data_data.hpp"
#include "dict_types.hpp"
#include "trx_trx.hpp"
#include "trx_roll.hpp"
#include "srv_srv.hpp"
#include "usr_types.hpp"
#include "que_types.hpp"
#include "row_types.hpp"
#include "pars_types.hpp"

/* If the following flag is set TRUE, the module will print trace info
of SQL execution in the IB_SQL_DEBUG version */
extern ibool	que_trace_on;


/// \brief Adds a query graph to the session's list of graphs.
/// \param [in] graph
/// \param [in] sess
IB_INTERN void que_graph_publish(que_t* graph, sess_t* sess);

/// \brief Creates a query graph fork node.
/// \return own: fork node
/// \param [in] graph if NULL then this fork node is assumed to be the graph root
/// \param [in] parent parent node
/// \param [in] fork_type fork type
/// \param [in] heap memory heap where created
IB_INTERN que_fork_t* que_fork_create(que_t* graph, que_node_t* parent, ulint fork_type, mem_heap_t* heap);

/// \brief Gets the first thr in a fork.
/// \param [in] fork query fork
IB_INLINE que_thr_t* que_fork_get_first_thr(que_fork_t* fork);

/// \brief Gets the child node of the first thr in a fork.
/// \param [in] fork query fork
IB_INLINE que_node_t* que_fork_get_child(que_fork_t* fork);

/// \brief Sets the parent of a graph node.
/// \param [in] node graph node
/// \param [in] parent parent
IB_INLINE void que_node_set_parent(que_node_t* node, que_node_t* parent);

/// \brief Creates a query graph thread node.
/// \return own: query thread node
/// \param [in] parent parent node, i.e., a fork node
/// \param [in] heap memory heap where created
IB_INTERN que_thr_t* que_thr_create(que_fork_t* parent, mem_heap_t* heap);

/// \brief Frees a query graph, but not the heap where it was created. Does not free explicit cursor declarations, they are freed in que_graph_free.
/// \param [in] node query graph node
IB_INTERN void que_graph_free_recursive(que_node_t* node);

/// \brief Frees a query graph.
/// \param [in] graph query graph; we assume that the memory heap where this graph was created is private to this graph: if not, then use que_graph_free_recursive and free the heap afterwards!
IB_INTERN void que_graph_free(que_t* graph);

/// \brief Stops a query thread if graph or trx is in a state requiring it. The conditions are tested in the order (1) graph, (2) trx. The kernel mutex has to be reserved.
/// \return TRUE if stopped
/// \param [in] thr query thread
IB_INTERN ibool que_thr_stop(que_thr_t* thr);

/// \brief Moves a thread from another state to the QUE_THR_RUNNING state. Increments the n_active_thrs counters of the query graph and transaction if thr was not active.
/// \param [in] thr an query thread
/// \param [in] trx transaction
IB_INTERN void que_thr_move_to_run_state_for_client(que_thr_t* thr, trx_t* trx);

/// \brief The query thread is stopped and made inactive, except in the case where it was put to the lock wait state in lock0lock.c, but the lock has already been granted or the transaction chosen as a victim in deadlock resolution.
/// \param [in] thr query thread
IB_INTERN void que_thr_stop_client(que_thr_t* thr);

/// \brief Run a query thread. Handles lock waits.
/// \param [in] thr query thread
IB_INTERN void que_run_threads(que_thr_t* thr);

/// \brief After signal handling is finished, returns control to a query graph error handling routine. (Currently, just returns the control to the root of the graph so that the graph can communicate an error message to the client.)
/// \param [in] trx trx
/// \param [in] fork query graph which was run before signal handling started, NULL not allowed
IB_INTERN void que_fork_error_handle(trx_t* trx, que_t* fork);

/// \brief Moves a suspended query thread to the QUE_THR_RUNNING state and releases a single worker thread to execute it. This function should be used to end the wait state of a query thread waiting for a lock or a stored procedure completion.
/// \param [in] thr query thread in the QUE_THR_LOCK_WAIT, or QUE_THR_PROCEDURE_WAIT, or QUE_THR_SIG_REPLY_WAIT state
/// \param [in,out] next_thr next query thread to run; if the value which is passed in is a pointer to a NULL pointer, then the calling function can start running a new query thread
IB_INTERN void que_thr_end_wait(que_thr_t* thr, que_thr_t** next_thr);

/// \brief Same as que_thr_end_wait, but no parameter next_thr available.
/// \param [in] thr query thread in the QUE_THR_LOCK_WAIT, or QUE_THR_PROCEDURE_WAIT, or QUE_THR_SIG_REPLY_WAIT state
IB_INTERN void que_thr_end_wait_no_next_thr(que_thr_t* thr);

/// \brief Starts execution of a command in a query fork.
/// \details Picks a query thread which is not in the QUE_THR_RUNNING state and moves it to that state. If none can be chosen, a situation which may arise in parallelized fetches, NULL is returned.
/// \return a query thread of the graph moved to QUE_THR_RUNNING state, or NULL; the query thread should be executed by que_run_threads by the caller
/// \param [in] fork a query fork
IB_INTERN que_thr_t* que_fork_start_command(que_fork_t* fork);

/// \brief Gets the trx of a query thread.
/// \param [in] thr query thread
IB_INLINE trx_t* thr_get_trx(que_thr_t* thr);

/// \brief Determines if this thread is rolling back an incomplete transaction in crash recovery.
/// \return TRUE if thr is rolling back an incomplete transaction in crash recovery
/// \param [in] thr query thread
IB_INLINE ibool thr_is_recv(const que_thr_t* thr);

/// \brief Gets the type of a graph node.
/// \param [in] node graph node
IB_INLINE ulint que_node_get_type(que_node_t* node);

/// \brief Gets pointer to the value data type field of a graph node.
/// \param [in] node graph node
IB_INLINE dtype_t* que_node_get_data_type(que_node_t* node);

/// \brief Gets pointer to the value dfield of a graph node.
/// \param [in] node graph node
IB_INLINE dfield_t* que_node_get_val(que_node_t* node);

/// \brief Gets the value buffer size of a graph node.
/// \return val buffer size, not defined if val.data == NULL in node
/// \param [in] node graph node
IB_INLINE ulint que_node_get_val_buf_size(que_node_t* node);

/// \brief Sets the value buffer size of a graph node.
/// \param [in] node graph node
/// \param [in] size size
IB_INLINE void que_node_set_val_buf_size(que_node_t* node, ulint size);

/// \brief Gets the next list node in a list of query graph nodes.
/// \param [in] node node in a list
IB_INLINE que_node_t* que_node_get_next(que_node_t* node);

/// \brief Gets the parent node of a query graph node.
/// \return parent node or NULL
/// \param [in] node node
IB_INLINE que_node_t* que_node_get_parent(que_node_t* node);

/// \brief Get the first containing loop node (e.g. while_node_t or for_node_t) for the given node, or NULL if the node is not within a loop.
/// \return containing loop node, or NULL.
/// \param [in] node node
IB_INTERN que_node_t* que_node_get_containing_loop_node(que_node_t* node);

/// \brief Catenates a query graph node to a list of them, possible empty list.
/// \return one-way list of nodes
/// \param [in] node_list node list, or NULL
/// \param [in] node node
IB_INLINE que_node_t* que_node_list_add_last(que_node_t* node_list, que_node_t* node);

/// \brief Gets a query graph node list length.
/// \return length, for NULL list 0
/// \param [in] node_list node list, or NULL
IB_INLINE ulint que_node_list_get_len(que_node_t* node_list);

/// \brief Checks if graph, trx, or session is in a state where the query thread should be stopped.
/// \return TRUE if should be stopped; NOTE that if the peek is made without reserving the kernel mutex, then another peek with the mutex reserved is necessary before deciding the actual stopping
/// \param [in] thr query thread
IB_INLINE ibool que_thr_peek_stop(que_thr_t* thr);

/// \brief Returns TRUE if the query graph is for a SELECT statement.
/// \return TRUE if a select
/// \param [in] graph graph
IB_INLINE ibool que_graph_is_select(que_t* graph);

/// \brief Prints info of an SQL query graph node.
/// \param [in] node query graph node
IB_INTERN void que_node_print_info(que_node_t* node);

/// \brief Evaluate the given SQL
/// \return error code or DB_SUCCESS
/// \param [in] info info struct, or NULL
/// \param [in] sql SQL string
/// \param [in] reserve_dict_mutex if TRUE, acquire/release dict_sys->mutex around call to pars_sql.
/// \param [in] trx trx
IB_INTERN ulint que_eval_sql(pars_info_t* info, const char* sql, ibool reserve_dict_mutex, trx_t* trx);

/// \brief Moves a thread from another state to the QUE_THR_RUNNING state. Increments the n_active_thrs counters of the query graph and transaction if thr was not active.
/// \details ***NOTE***: This is the only functions in which such a transition is allowed to happen!
/// \param [in] thr an query thread
IB_INTERN void que_thr_move_to_run_state(que_thr_t* thr);


/// \brief A patch for a client used to 'stop' a dummy query thread used in client select, when there is no error or lock wait.
/// \details TODO: Currently only called from row0merge, needs to be removed.
/// \param [in] thr query thread
/// \param [in] trx transaction
IB_INTERN void que_thr_stop_for_client_no_error(que_thr_t* thr, trx_t* trx);

/// \brief Reset the variables.
IB_INTERN void que_var_init(void);

/*==============*/
/* Query graph query thread node: the fields are protected by the kernel mutex with the exceptions named below */

struct que_thr_struct{
	que_common_t	common;		/*!< type: QUE_NODE_THR */
	ulint		magic_n;	/*!< magic number to catch memory corruption */
	que_node_t*	child;		/*!< graph child node */
	que_t*		graph;		/*!< graph where this node belongs */
	ibool		is_active;	/*!< TRUE if the thread has been set to the run state in que_thr_move_to_run_state, but not deactivated in que_thr_dec_reference_count */
	ulint		state;		/*!< state of the query thread */
	UT_LIST_NODE_T(que_thr_t) thrs;		/*!< list of thread nodes of the fork node */
	UT_LIST_NODE_T(que_thr_t) trx_thrs;	/*!< lists of threads in wait list of the trx */
	UT_LIST_NODE_T(que_thr_t) queue;		/*!< list of runnable thread nodes in the server task queue */
	/*------------------------------*/
	/* The following fields are private to the OS thread executing the query thread, and are not protected by the kernel mutex: */

	que_node_t*	run_node;	/*!< pointer to the node where the subgraph down from this node is currently executed */
	que_node_t*	prev_node;	/*!< pointer to the node from which the control came */
	ulint		resource;	/*!< resource usage of the query thread thus far */
	ulint		lock_state;	/*!< lock state of thread (table or row) */
};

constinit ulint QUE_THR_MAGIC_N = 8476583;
constinit ulint QUE_THR_MAGIC_FREED = 123461526;

/* Query graph fork node: its fields are protected by the kernel mutex */
struct que_fork_struct{
	que_common_t	common;		/*!< type: QUE_NODE_FORK */
	que_t*		graph;		/*!< query graph of this node */
	ulint		fork_type;	/*!< fork type */
	ulint		n_active_thrs;	/*!< if this is the root of a graph, the number query threads that have been started in que_thr_move_to_run_state but for which que_thr_dec_refer_count has not yet been called */
	trx_t*		trx;		/*!< transaction: this is set only in the root node */
	ulint		state;		/*!< state of the fork node */
	que_thr_t*	caller;		/*!< pointer to a possible calling query thread */
	UT_LIST_BASE_NODE_T(que_thr_t)
			thrs;		/*!< list of query threads */
	/*------------------------------*/
	/* The fields in this section are defined only in the root node */
	sym_tab_t*	sym_tab;	/*!< symbol table of the query,
					generated by the parser, or NULL
					if the graph was created 'by hand' */
	pars_info_t*	info;		/*!< info struct, or NULL */
	/* The following cur_... fields are relevant only in a select graph */

	ulint		cur_end;	/*!< QUE_CUR_NOT_DEFINED, QUE_CUR_START, QUE_CUR_END */
	ulint		cur_pos;	/*!< if there are n rows in the result set, values 0 and n + 1 mean before first row, or after last row, depending on cur_end; values 1...n mean a row index */
	ibool		cur_on_row;	/*!< TRUE if cursor is on a row, i.e., it is not before the first row or after the last row */
	dulint		n_inserts;	/*!< number of rows inserted */
	dulint		n_updates;	/*!< number of rows updated */
	dulint		n_deletes;	/*!< number of rows deleted */
	sel_node_t*	last_sel_node;	/*!< last executed select node, or NULL if none */
	UT_LIST_NODE_T(que_fork_t) graphs;		/*!< list of query graphs of a session or a stored procedure */
	/*------------------------------*/
	mem_heap_t*	heap;		/*!< memory heap where the fork was created */

};

/* Query fork (or graph) types */
constinit ulint QUE_FORK_SELECT_NON_SCROLL = 1;	/* forward-only cursor */
constinit ulint QUE_FORK_SELECT_SCROLL = 2;	/* scrollable cursor */
constinit ulint QUE_FORK_INSERT = 3;
constinit ulint QUE_FORK_UPDATE = 4;
constinit ulint QUE_FORK_ROLLBACK = 5;
			/* This is really the undo graph used in rollback,
			no signal-sending roll_node in this graph */
constinit ulint QUE_FORK_PURGE = 6;
constinit ulint QUE_FORK_EXECUTE = 7;
constinit ulint QUE_FORK_PROCEDURE = 8;
constinit ulint QUE_FORK_PROCEDURE_CALL = 9;
constinit ulint QUE_FORK_USER_INTERFACE = 10;
constinit ulint QUE_FORK_RECOVERY = 11;

/* Query fork (or graph) states */
constinit ulint QUE_FORK_ACTIVE = 1;
constinit ulint QUE_FORK_COMMAND_WAIT = 2;
constinit ulint QUE_FORK_INVALID = 3;
constinit ulint QUE_FORK_BEING_FREED = 4;

/* Flag which is ORed to control structure statement node types */
constinit ulint QUE_NODE_CONTROL_STAT = 1024;

/* Query graph node types */
constinit ulint QUE_NODE_LOCK = 1;
constinit ulint QUE_NODE_INSERT = 2;
constinit ulint QUE_NODE_UPDATE = 4;
constinit ulint QUE_NODE_CURSOR = 5;
constinit ulint QUE_NODE_SELECT = 6;
constinit ulint QUE_NODE_AGGREGATE = 7;
constinit ulint QUE_NODE_FORK = 8;
constinit ulint QUE_NODE_THR = 9;
constinit ulint QUE_NODE_UNDO = 10;
constinit ulint QUE_NODE_COMMIT = 11;
constinit ulint QUE_NODE_ROLLBACK = 12;
constinit ulint QUE_NODE_PURGE = 13;
constinit ulint QUE_NODE_CREATE_TABLE = 14;
constinit ulint QUE_NODE_CREATE_INDEX = 15;
constinit ulint QUE_NODE_SYMBOL = 16;
constinit ulint QUE_NODE_RES_WORD = 17;
constinit ulint QUE_NODE_FUNC = 18;
constinit ulint QUE_NODE_ORDER = 19;
constinit ulint QUE_NODE_PROC = (20 + QUE_NODE_CONTROL_STAT);
constinit ulint QUE_NODE_IF = (21 + QUE_NODE_CONTROL_STAT);
constinit ulint QUE_NODE_WHILE = (22 + QUE_NODE_CONTROL_STAT);
constinit ulint QUE_NODE_ASSIGNMENT = 23;
constinit ulint QUE_NODE_FETCH = 24;
constinit ulint QUE_NODE_OPEN = 25;
constinit ulint QUE_NODE_COL_ASSIGNMENT = 26;
constinit ulint QUE_NODE_FOR = (27 + QUE_NODE_CONTROL_STAT);
constinit ulint QUE_NODE_RETURN = 28;
constinit ulint QUE_NODE_ROW_PRINTF = 29;
constinit ulint QUE_NODE_ELSIF = 30;
constinit ulint QUE_NODE_CALL = 31;
constinit ulint QUE_NODE_EXIT = 32;

/* Query thread states */
constinit ulint QUE_THR_RUNNING = 1;
constinit ulint QUE_THR_PROCEDURE_WAIT = 2;
constinit ulint QUE_THR_COMPLETED = 3;	/* in selects this means that the thread is at the end of its result set (or start, in case of a scroll cursor); in other statements, this means the thread has done its task */
constinit ulint QUE_THR_COMMAND_WAIT = 4;
constinit ulint QUE_THR_LOCK_WAIT = 5;
constinit ulint QUE_THR_SIG_REPLY_WAIT = 6;
constinit ulint QUE_THR_SUSPENDED = 7;
constinit ulint QUE_THR_ERROR = 8;

/* Query thread lock states */
constinit ulint QUE_THR_LOCK_NOLOCK = 0;
constinit ulint QUE_THR_LOCK_ROW = 1;
constinit ulint QUE_THR_LOCK_TABLE = 2;

/* From where the cursor position is counted */
constinit ulint QUE_CUR_NOT_DEFINED = 1;
constinit ulint QUE_CUR_START = 2;
constinit ulint QUE_CUR_END = 3;


#ifndef IB_DO_NOT_INLINE
#include "que_que.inl"
#endif
