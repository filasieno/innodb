// Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.
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

/**
 * \file trx_purge.hpp
 * \brief Purge old versions
 *
 * Created 3/26/1996 Heikki Tuuri
 */

#ifndef trx0purge_h
#define trx0purge_h

#include "fil_fil.hpp"
#include "mtr_mtr.hpp"
#include "page_page.hpp"
#include "que_types.hpp"
#include "trx_sys.hpp"
#include "trx_types.hpp"
#include "univ.i"
#include "usr_sess.hpp"

/**
 * \brief The global data structure coordinating a purge
 */
extern trx_purge_t *purge_sys;

/**
 * \brief A dummy undo record used as a return value when we have a whole undo log
 * which needs no purge
 */
extern trx_undo_rec_t trx_purge_dummy_rec;

/**
 * \brief Calculates the file address of an undo log header when we have the file
 * address of its history list node.
 * \param node_addr File address of the history list node of the log
 * \return File address of the log
 */
UNIV_INLINE
fil_addr_t trx_purge_get_log_from_hist(fil_addr_t node_addr);
/**
 * \brief Checks if trx_id is >= purge_view: then it is guaranteed that its update
 * undo log still exists in the system.
 * \param trx_id Transaction id
 * \return TRUE if is sure that it is preserved, also if the function
 * returns FALSE, it is possible that the undo log still exists in the system
 */
UNIV_INTERN
ibool trx_purge_update_undo_must_exist(trx_id_t trx_id);
/**
 * \brief Creates the global purge system control structure and inits the history
 * mutex.
 */
UNIV_INTERN
void trx_purge_sys_create(void);
/**
 * \brief Frees the global purge system control structure.
 */
UNIV_INTERN
void trx_purge_sys_close(void);
/**
 * \brief Adds the update undo log as the first log in the history list. Removes the
 * update undo log segment from the rseg slot if it is too big for reuse.
 * \param trx Transaction
 * \param undo_page Update undo log header page, x-latched
 * \param mtr Mtr
 */
UNIV_INTERN
void trx_purge_add_update_undo_to_history(trx_t *trx, page_t *undo_page, mtr_t *mtr);
/**
 * \brief Fetches the next undo log record from the history list to purge. It must be
 * released with the corresponding release function.
 * \param roll_ptr Roll pointer to undo record
 * \param cell Storage cell for the record in the purge array
 * \param heap Memory heap where copied
 * \return Copy of an undo log record or pointer to trx_purge_dummy_rec,
 * if the whole undo log can skipped in purge; NULL if none left
 */
UNIV_INTERN
trx_undo_rec_t *trx_purge_fetch_next_rec(roll_ptr_t *roll_ptr, trx_undo_inf_t **cell, mem_heap_t *heap);
/**
 * \brief Releases a reserved purge undo record.
 * \param cell Storage cell
 */
UNIV_INTERN
void trx_purge_rec_release(trx_undo_inf_t *cell);
/**
 * \brief This function runs a purge batch.
 * \return Number of undo log pages handled in the batch
 */
UNIV_INTERN
ulint trx_purge(void);
/**
 * \brief Prints information of the purge system to stderr.
 */
UNIV_INTERN
void trx_purge_sys_print(void);
/**
 * \brief Reset the variables.
 */
UNIV_INTERN
void trx_purge_var_init(void);

/**
 * \brief The control structure used in the purge operation
 */
struct trx_purge_struct
{
    /// Purge system state
    ulint state;
    /// System session running the purge query
    sess_t *sess;
    /// System transaction running the purge query: this trx is not in the trx list
    /// of the trx system and it never ends
    trx_t *trx;
    /// The query graph which will do the parallelized purge operation
    que_t *query;
    /// The latch protecting the purge view. A purge operation must acquire an
    /// x-latch here for the instant at which it changes the purge view: an undo
    /// log operation can prevent this by obtaining an s-latch here.
    rw_lock_t latch;
    /// The purge will not remove undo logs which are >= this view (purge view)
    read_view_t *view;
    /// Mutex protecting the fields below
    mutex_t mutex;
    /// Approximate number of undo log pages processed in purge
    ulint n_pages_handled;
    /// Target of how many pages to get processed in the current purge
    ulint handle_limit;
    //------------------------------
    // The following two fields form the 'purge pointer' which advances
    // during a purge, and which is used in history list truncation

    /// Purge has advanced past all transactions whose number is less than this
    trx_id_t purge_trx_no;
    /// Purge has advanced past all records whose undo number is less than this
    undo_no_t purge_undo_no;
    //-----------------------------
    /// TRUE if the info of the next record to purge is stored below: if yes, then
    /// the transaction number and the undo number of the record are stored in
    /// purge_trx_no and purge_undo_no above
    ibool next_stored;
    /// Rollback segment for the next undo record to purge
    trx_rseg_t *rseg;
    /// Page number for the next undo record to purge, page number of the
    /// log header, if dummy record
    ulint page_no;
    /// Page offset for the next undo record to purge, 0 if the dummy record
    ulint offset;
    /// Header page of the undo log where the next record to purge belongs
    ulint hdr_page_no;
    /// Header byte offset on the page
    ulint hdr_offset;
    //-----------------------------
    /// Array of transaction numbers and undo numbers of the undo records
    /// currently under processing in purge
    trx_undo_arr_t *arr;
    /// Temporary storage used during a purge: can be emptied after purge completes
    mem_heap_t *heap;
};

/// Purge operation is running
#define TRX_PURGE_ON 1
/// Purge operation is stopped, or it should be stopped
#define TRX_STOP_PURGE 2

#ifndef UNIV_NONINL
#include "trx_purge.inl"
#endif

#endif
