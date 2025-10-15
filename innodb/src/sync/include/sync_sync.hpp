// Copyright (c) 2025, Fabio N. Filasieno and Roberto Boati
// Copyright (c) 1995, 2010, Innobase Oy. All Rights Reserved.
// Copyright (c) 2008, Google Inc.
//
// Portions of this file contain modifications contributed and copyrighted by
// Google, Inc. Those modifications are gratefully acknowledged and are described
// briefly in the InnoDB documentation. The contributions by Google are
// incorporated with their permission, and subject to the conditions contained in
// the file COPYING.Google.
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

#pragma once

#include "defs.hpp"
#include "sync_types.hpp"
#include "ut_lst.hpp"
#include "ut_mem.hpp"
#include "os_thread.hpp"
#include "os_sync.hpp"
#include "sync_arr.hpp"

/// \file sync_sync.h
/// \brief Mutex, the basic synchronization primitive
/// \details Originally crated by 9/5/1995 Heikki Tuuri

/// ## Latching order within the db
/// 
/// The mutex or latch in the central memory object, for instance, a rollback
/// segment object, must be acquired before acquiring the latch or latches to
/// the corresponding file data structure. In the latching order below, these
/// file page object latches are placed immediately below the corresponding
/// central memory object latch or mutex.
/// 
/// Synchronization object			Notes
/// ----------------------			-----
/// Dictionary mutex			If we have a pointer to a dictionary
/// |					object, e.g., a table, it can be
/// |					accessed without reserving the
/// |					dictionary mutex. We must have a
/// |					reservation, a memoryfix, to the
/// |					appropriate table object in this case,
/// |					and the table must be explicitly
/// |					released later.
/// V
/// Dictionary header
/// |
/// V
/// Secondary index tree latch		The tree latch protects also all
/// |					the B-tree non-leaf pages. These
/// V					can be read with the page only
/// Secondary index non-leaf		bufferfixed to save CPU time,
/// |					no s-latch is needed on the page.
/// |					Modification of a page requires an
/// |					x-latch on the page, however. If a
/// |					thread owns an x-latch to the tree,
/// |					it is allowed to latch non-leaf pages
/// |					even after it has acquired the fsp
/// |					latch.
/// V
/// Secondary index leaf			The latch on the secondary index leaf
/// |					can be kept while accessing the
/// |					clustered index, to save CPU time.
/// V
/// Clustered index tree latch		To increase concurrency, the tree
/// |					latch is usually released when the
/// |					leaf page latch has been acquired.
/// V
/// Clustered index non-leaf
/// |
/// V
/// Clustered index leaf
/// |
/// V
/// Transaction system header
/// |
/// V
/// Transaction undo mutex			The undo log entry must be written
/// |					before any index page is modified.
/// |					Transaction undo mutex is for the undo
/// |					logs the analogue of the tree latch
/// |					for a B-tree. If a thread has the
/// |					trx undo mutex reserved, it is allowed
/// |					to latch the undo log pages in any
/// |					order, and also after it has acquired
/// |					the fsp latch.
/// V
/// Rollback segment mutex			The rollback segment mutex must be
/// |					reserved, if, e.g., a new page must
/// |					be added to an undo log. The rollback
/// |					segment and the undo logs in its
/// |					history list can be seen as an
/// |					analogue of a B-tree, and the latches
/// |					reserved similarly, using a version of
/// |					lock-coupling. If an undo log must be
/// |					extended by a page when inserting an
/// |					undo log record, this corresponds to
/// |					a pessimistic insert in a B-tree.
/// V
/// Rollback segment header
/// |
/// V
/// Purge system latch
/// |
/// V
/// Undo log pages				If a thread owns the trx undo mutex,
/// |					or for a log in the history list, the
/// |					rseg mutex, it is allowed to latch
/// |					undo log pages in any order, and even
/// |					after it has acquired the fsp latch.
/// |					If a thread does not have the
/// |					appropriate mutex, it is allowed to
/// |					latch only a single undo log page in
/// |					a mini-transaction.
/// V
/// File space management latch		If a mini-transaction must allocate
/// |					several file pages, it can do that,
/// |					because it keeps the x-latch to the
/// |					file space management in its memo.
/// V
/// File system pages
/// |
/// V
/// Kernel mutex				If a kernel operation needs a file
/// |					page allocation, it must reserve the
/// |					fsp x-latch before acquiring the kernel
/// |					mutex.
/// V
/// Search system mutex
/// |
/// V
/// Buffer pool mutex
/// |
/// V
/// Log mutex
/// |
/// Any other latch
/// |
/// V
/// Memory pool mutex 

/// \author Fabio N. Filasieno
/// \date 2025-10-15

/// \brief Initializes the synchronization data structures.
/// \param state The state
IB_INTERN void sync_init(innodb_t* state);

/// \brief Frees the resources in synchronization data structures.
/// \param state The state
IB_INTERN void sync_close(innodb_t* state);


/** Creates, or rather, initializes a mutex object to a specified memory location (which must be appropriately aligned). 
The mutex is initialized in the reset state. 
Explicit freeing of the mutex with mutex_free is
necessary only if the memory block containing it is freed. 
*/

/**
Creates, or rather, initializes a mutex object in a specified memory
location (which must be appropriately aligned). The mutex is initialized
in the reset state. Explicit freeing of the mutex with mutex_free is
necessary only if the memory block containing it is freed. */


/// \brief Creates, or rather, initializes a mutex object in a specified memory location (which must be appropriately aligned). 
/// \details The mutex is initialized in the reset state. 
/// Explicit freeing of the mutex with mutex_free is necessary only if the memory block containing it is freed.
/// \param mutex The mutex to create
/// \param cmutex_name The mutex name
/// \param level The level
/// \param cfile_name The file name where created
/// \param cline The line where created

IB_INTERN void mutex_create(mutex_t* mutex, const char* cmutex_name, ulint level, const char* cfile_name = __FILE__, ulint cline = __LINE__); //IB_DEBUG && IB_SYNC_DEBUG
IB_INTERN void mutex_create(mutex_t* mutex, const char*	cmutex_name, const char* cfile_name = __FILE__, ulint cline = __LINE__); // IB_DEBUG && ! IB_SYNC_DEBUG
IB_INTERN void mutex_create(mutex_t* mutex, const char* cfile_name = __FILE__, ulint cline = __LINE__); // ! IB_DEBUG

/**
Calling this function is obligatory only if the memory buffer containing
the mutex is freed. Removes a mutex object from the mutex list. The mutex
is checked to be in the reset state. */
IB_INTERN void mutex_free(innodb_t* state, mutex_t* mutex);	/*!< in: mutex */

/**
NOTE! Use the corresponding macro in the header file, not this function
directly. Locks a mutex for the current thread. If the mutex is reserved
the function spins a preset time (controlled by state->srv.n_spin_wait_rounds) waiting
for the mutex before suspending the thread. */
IB_INLINE void mutex_enter(
	mutex_t* mutex,		/*!< in: pointer to mutex */
	const char*	file_name == __FILE__ ,	/*!< in: file name where locked */
	ulint line == __LINE__ );		/*!< in: line where locked */
/**************************************************************//**
NOTE! The following macro should be used in mutex locking, not the
corresponding function. */


/********************************************************************/

/**
NOTE! Use the corresponding macro in the header file, not this function
directly. Tries to lock the mutex for the current thread. If the lock is not
acquired immediately, returns with return value 1.
@return	0 if succeed, 1 if not */
IB_INTERN ulint mutex_enter_nowait(
	mutex_t*	mutex,		/*!< in: pointer to mutex */
	const char*	file_name,	/*!< in: file name where mutex requested */
	ulint		line);		/*!< in: line where requested */

/** Unlocks a mutex owned by the current thread. */
/*!< in: pointer to mutex */
IB_INLINE void mutex_exit(mutex_t* mutex);	

#ifdef IB_SYNC_DEBUG

	/// \brief Returns TRUE if no mutex or rw-lock is currently locked.
	/// \details Works only in the debug version.
	/// \return TRUE if no mutexes and rw-locks reserved
	IB_INTERN ibool sync_all_freed(void);

#endif // IB_SYNC_DEBUG

/** Prints wait info of the sync system. */
/*!< in: stream where to print */
IB_INTERN void sync_print_wait_info(innodb_t* state);	

/** Prints info of the sync system. */
/*!< in: stream where to print */
IB_INTERN void sync_print(innodb_t* state);	

#ifdef IB_DEBUG
	/******************************************************************//**
	Checks that the mutex has been initialized.
	@return	TRUE */
	IB_INTERN
	ibool
	mutex_validate(
	/*===========*/
		const mutex_t*	mutex);	/*!< in: mutex */
	/******************************************************************//**
	Checks that the current thread owns the mutex. Works only
	in the debug version.
	@return	TRUE if owns */
	IB_INTERN
	ibool
	mutex_own(
	/*======*/
		const mutex_t*	mutex)	/*!< in: mutex */
		__attribute__((warn_unused_result));
	#endif /* IB_DEBUG */
	#ifdef IB_SYNC_DEBUG
	/******************************************************************//**
	Adds a latch and its level in the thread level array. Allocates the memory
	for the array if called first time for this OS thread. Makes the checks
	against other latch levels stored in the array for this thread. */
	IB_INTERN
	void
	sync_thread_add_level(
	/*==================*/
		void*	latch,	/*!< in: pointer to a mutex or an rw-lock */
		ulint	level);	/*!< in: level in the latching order; if
				SYNC_LEVEL_VARYING, nothing is done */
	/******************************************************************//**
	Removes a latch from the thread level array if it is found there.
	@return TRUE if found in the array; it is no error if the latch is
	not found, as we presently are not able to determine the level for
	every latch reservation the program does */
	IB_INTERN
	ibool
	sync_thread_reset_level(
	/*====================*/
		void*	latch);	/*!< in: pointer to a mutex or an rw-lock */
	/******************************************************************//**
	Checks that the level array for the current thread is empty.
	@return	TRUE if empty */
	IB_INTERN
	ibool
	sync_thread_levels_empty(void);
	/*==========================*/
	/******************************************************************//**
	Checks if the level array for the current thread contains a
	mutex or rw-latch at the specified level.
	@return	a matching latch, or NULL if not found */
	IB_INTERN
	void*
	sync_thread_levels_contains(
	/*========================*/
		ulint	level);			/*!< in: latching order level
						(SYNC_DICT, ...)*/
	/******************************************************************//**
	Checks if the level array for the current thread is empty.
	@return	a latch, or NULL if empty except the exceptions specified below */
	IB_INTERN
	void*
	sync_thread_levels_nonempty_gen(
	/*============================*/
		ibool	dict_mutex_allowed);	/*!< in: TRUE if dictionary mutex is
						allowed to be owned by the thread,
						also purge_is_running mutex is
						allowed */
	#define sync_thread_levels_empty_gen(d) (!sync_thread_levels_nonempty_gen(d))
	/******************************************************************//**
	Gets the debug information for a reserved mutex. */
	IB_INTERN
	void
	mutex_get_debug_info(
	/*=================*/
		mutex_t*	mutex,		/*!< in: mutex */
		const char**	file_name,	/*!< out: file where requested */
		ulint*		line,		/*!< out: line where requested */
		os_thread_id_t* thread_id);	/*!< out: id of the thread which owns
						the mutex */
	/******************************************************************//**
	Counts currently reserved mutexes. Works only in the debug version.
	@return	number of reserved mutexes */
	IB_INTERN
	ulint
	mutex_n_reserved(void);
	
#endif // IB_SYNC_DEBUG


/** NOT to be used outside this module except in debugging! Gets the value of the lock word. */
/*!< in: mutex */
IB_INLINE lock_word_t mutex_get_lock_word(const mutex_t* mutex);

#ifdef IB_SYNC_DEBUG
	/**
	NOT to be used outside this module except in debugging! Gets the waiters field in a mutex. @return	value to set */
	IB_INLINE ulint mutex_get_waiters(const mutex_t* mutex);	/*!< in: mutex */
#endif // IB_SYNC_DEBUG

/// \brief Reset variables.
IB_INTERN void sync_var_init(innodb_t* state);

/* Latching order levels */

/* User transaction locks are higher than any of the latch levels below:
no latches are allowed when a thread goes to wait for a normal table
or row lock! */
/// \brief User transaction lock level (highest priority)
constinit ulint SYNC_USER_TRX_LOCK = 9999;

/// \brief Suppress latching order checking
constinit ulint SYNC_NO_ORDER_CHECK = 3000;

/// \brief Level is varying. Only used with buffer pool page locks, which do not
/// have a fixed level, but instead have their level set after the page is
/// locked; see e.g. ibuf_bitmap_get_map_page().
constinit ulint SYNC_LEVEL_VARYING = 2000;

/// \brief Used for trx_i_s_cache_t::rw_lock
constinit ulint SYNC_TRX_I_S_RWLOCK = 1910;

/// \brief Used for trx_i_s_cache_t::last_read_mutex
constinit ulint SYNC_TRX_I_S_LAST_READ = 1900;

/// \brief Used to serialize access to the file format tag
constinit ulint SYNC_FILE_FORMAT_TAG = 1200;

/// \brief Table create, drop, etc. reserve this in X-mode, implicit or background
/// operations purge, rollback, foreign key checks reserve this in S-mode
constinit ulint SYNC_DICT_OPERATION = 1001;

/// \brief Dictionary synchronization level
constinit ulint SYNC_DICT = 1000;

/// \brief Dictionary auto-increment mutex
constinit ulint SYNC_DICT_AUTOINC_MUTEX = 999;

/// \brief Dictionary header synchronization level
constinit ulint SYNC_DICT_HEADER = 995;

/// \brief Insert buffer header synchronization level
constinit ulint SYNC_IBUF_HEADER = 914;

/// \brief Insert buffer pessimistic insert mutex
constinit ulint SYNC_IBUF_PESS_INSERT_MUTEX = 912;

/// \brief Insert buffer mutex is really below SYNC_FSP_PAGE: we assign a value this
/// high only to make the program to pass the debug checks
constinit ulint SYNC_IBUF_MUTEX = 910;

/*-------------------------------*/
/// \brief Index tree synchronization level
constinit ulint SYNC_INDEX_TREE = 900;

/// \brief New tree node synchronization level
constinit ulint SYNC_TREE_NODE_NEW = 892;

/// \brief Tree node from hash synchronization level
constinit ulint SYNC_TREE_NODE_FROM_HASH = 891;

/// \brief Tree node synchronization level
constinit ulint SYNC_TREE_NODE = 890;

/// \brief Purge system synchronization level
constinit ulint SYNC_PURGE_SYS = 810;

/// \brief Purge latch synchronization level
constinit ulint SYNC_PURGE_LATCH = 800;

/// \brief Transaction undo synchronization level
constinit ulint SYNC_TRX_UNDO = 700;

/// \brief Rollback segment synchronization level
constinit ulint SYNC_RSEG = 600;

/// \brief New rollback segment header synchronization level
constinit ulint SYNC_RSEG_HEADER_NEW = 591;

/// \brief Rollback segment header synchronization level
constinit ulint SYNC_RSEG_HEADER = 590;

/// \brief Transaction undo page synchronization level
constinit ulint SYNC_TRX_UNDO_PAGE = 570;

/// \brief External storage synchronization level
constinit ulint SYNC_EXTERN_STORAGE = 500;

/// \brief File space synchronization level
constinit ulint SYNC_FSP = 400;

/// \brief File space page synchronization level
constinit ulint SYNC_FSP_PAGE = 395;

/*------------------------------------- Insert buffer headers */
/*------------------------------------- ibuf_mutex */
/*------------------------------------- Insert buffer tree */
/// \brief Insert buffer bitmap mutex synchronization level
constinit ulint SYNC_IBUF_BITMAP_MUTEX = 351;

/// \brief Insert buffer bitmap synchronization level
constinit ulint SYNC_IBUF_BITMAP = 350;

/*-------------------------------*/
/// \brief Kernel synchronization level
constinit ulint SYNC_KERNEL = 300;

/// \brief Record lock synchronization level
constinit ulint SYNC_REC_LOCK = 299;

/// \brief Transaction lock heap synchronization level
constinit ulint SYNC_TRX_LOCK_HEAP = 298;

/// \brief Transaction system header synchronization level
constinit ulint SYNC_TRX_SYS_HEADER = 290;

/// \brief Log synchronization level
constinit ulint SYNC_LOG = 170;

/// \brief Recovery synchronization level
constinit ulint SYNC_RECV = 168;

/// \brief Work queue synchronization level
constinit ulint SYNC_WORK_QUEUE = 162;

/// \brief Search system configuration synchronization level (for assigning btr_search_enabled)
constinit ulint SYNC_SEARCH_SYS_CONF = 161;

/// \brief Search system synchronization level. NOTE that if we have a memory
/// heap that can be extended to the buffer pool, its logical level is
/// SYNC_SEARCH_SYS, as memory allocation can call routines there! Otherwise
/// the level is SYNC_MEM_HASH.
constinit ulint SYNC_SEARCH_SYS = 160;

/// \brief Buffer pool synchronization level
constinit ulint SYNC_BUF_POOL = 150;

/// \brief Buffer block synchronization level
constinit ulint SYNC_BUF_BLOCK = 149;

/// \brief Doublewrite synchronization level
constinit ulint SYNC_DOUBLEWRITE = 140;

/// \brief Any latch synchronization level
constinit ulint SYNC_ANY_LATCH = 135;

/// \brief Thread local synchronization level
constinit ulint SYNC_THR_LOCAL = 133;

/// \brief Memory hash synchronization level
constinit ulint SYNC_MEM_HASH = 131;

/// \brief Memory pool synchronization level
constinit ulint SYNC_MEM_POOL = 130;

/* Codes used to designate lock operations */

/// \brief Read-write lock not locked state
constinit ulint RW_LOCK_NOT_LOCKED = 350;

/// \brief Read-write lock exclusive mode
constinit ulint RW_LOCK_EX = 351;

/// \brief Read-write lock exclusive mode (alias for RW_LOCK_EX)
constinit ulint RW_LOCK_EXCLUSIVE = 351;

/// \brief Read-write lock shared mode
constinit ulint RW_LOCK_SHARED = 352;

/// \brief Read-write lock waiting for exclusive mode
constinit ulint RW_LOCK_WAIT_EX = 353;

/// \brief Synchronization mutex
constinit ulint SYNC_MUTEX = 354;

/// \brief InnoDB mutex
struct mutex_struct {

	static constinit ulint MAGIC_N = 979585;

	os_event_t           event;	//!< Used by sync_arr.c for the wait queue
	volatile lock_word_t lock_word;	/*!< lock_word is the target of the atomic test-and-set instruction when atomic operations are enabled. */

#if !defined(IB_HAVE_ATOMIC_BUILTINS)
	os_fast_mutex_t
	os_fast_mutex;	/*!< We use this OS mutex in place of lock_word when atomic operations are not enabled */
#endif

	ulint waiters;	/*!< This ulint is set to 1 if there are (or may be) threads waiting in the global wait array for this mutex to be released. Otherwise, this is 0. */
	UT_LIST_NODE_T(mutex_t)	list; /*!< All allocated mutexes are put into a list.	Pointers to the next and prev. */

#ifdef IB_SYNC_DEBUG
	const char*	file_name;     //!< File where the mutex was locked */
	ulint line;		           //!< Line where the mutex was locked */
	ulint level;		       //!< Level in the global latching order */
#endif // IB_SYNC_DEBUG

	const char*	cfile_name;    //!< File name where mutex created */
	ulint cline;               //!< Line where created */

#ifdef IB_DEBUG
	os_thread_id_t thread_id;  //!< The thread id of the thread which locked the mutex. */
	ulint magic_n;	           //!< Equal to MUTEX_MAGIC_N
#endif // IB_DEBUG

	ulong count_os_wait;	 //!< count of os_wait */

#ifdef IB_DEBUG
	ulong		count_using;	   /*!< count of times mutex used */
	ulong		count_spin_loop;   /*!< count of spin loops */
	ulong		count_spin_rounds; /*!< count of spin rounds */
	ulong		count_os_yield;	   /*!< count of os_wait */
	ib_uint64_t	lspent_time;	   /*!< mutex os_wait timer msec */
	ib_uint64_t	lmax_spent_time;   /*!< mutex os_wait timer msec */
	const char*	cmutex_name;	   /*!< mutex name */
	ulint		mutex_type;	       /*!< 0=usual mutex, 1=rw_lock mutex */
#endif // IB_DEBUG
};


/// \brief Global list of database mutexes (not OS mutexes) created
typedef UT_LIST_BASE_NODE_T(mutex_t) ut_list_base_node_t;


/** The global array of wait cells for implementation of the databases own
mutexes and read-write locks. */
extern sync_array_t* sync_primary_wait_array; /* Appears here for debugging purposes only! */

/** The number of mutex_exit calls. Intended for performance monitoring. */
extern ib_int64_t mutex_exit_count;

/// \brief Latching order checks start when this is set TRUE
extern ibool sync_order_checks_on;

/// \brief This variable is set to TRUE when sync_init is called
extern ibool sync_initialized;

///  Global list of database mutexes (not OS mutexes) created.
extern ut_list_base_node_t  mutex_list;

/// \brief  Mutex protecting the mutex_list variable
extern mutex_t mutex_list_mutex;


#ifndef IB_DO_NOT_INLINE
	#include "sync_sync.inl"
#endif

#endif
