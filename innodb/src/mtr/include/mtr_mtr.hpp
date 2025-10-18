/*****************************************************************************

Copyright (c) 1995, 2025, Innobase Oy. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/

/// \file mtr_mtr.hpp
/// \brief Mini-transaction buffer
/// \details Originally created by Heikki Tuuri in 11/26/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "mem_mem.hpp"
#include "dyn_dyn.hpp"
#include "buf_types.hpp"
#include "sync_rw.hpp"
#include "ut_byte.hpp"
#include "mtr_types.hpp"
#include "page_types.hpp"

// -----------------------------------------------------------------------------------------
// type definitions
// -----------------------------------------------------------------------------------------

/* Type definition of a mini-transaction memo stack slot. */
typedef	struct mtr_memo_slot_struct	mtr_memo_slot_t;
struct mtr_memo_slot_struct{
	ulint	type;	/*!< type of the stored object (MTR_MEMO_S_LOCK, ...) */
	void*	object;	/*!< pointer to the object */
};

/* Mini-transaction handle and buffer */
struct mtr_struct{
#ifdef IB_DEBUG
	ulint		state;	/*!< MTR_ACTIVE, MTR_COMMITTING, MTR_COMMITTED */
#endif
	dyn_array_t	memo;	/*!< memo stack for locks etc. */
	dyn_array_t	log;	/*!< mini-transaction log */
	ibool		modifications;
				/* TRUE if the mtr made modifications to
				buffer pool pages */
	ulint		n_log_recs;
				/* count of how many page initial log records
				have been written to the mtr log */
	ulint		log_mode; /* specifies which operations should be
				logged; default value MTR_LOG_ALL */
	ib_uint64_t	start_lsn;/* start lsn of the possible log entry for
				this mtr */
	ib_uint64_t	end_lsn;/* end lsn of the possible log entry for
				this mtr */
#ifdef IB_DEBUG
	ulint		magic_n;
#endif /* IB_DEBUG */
};

// -----------------------------------------------------------------------------------------
// macro constants
// -----------------------------------------------------------------------------------------

constinit ulint MTR_LOG_ALL = 21;
constinit ulint MTR_LOG_NONE = 22;
/*#define	MTR_LOG_SPACE	23 */	/* log only operations modifying
					file space page allocation data
					(operations in fsp0fsp.* ) */
constinit ulint MTR_LOG_SHORT_INSERTS = 24;

constinit ulint MTR_MEMO_PAGE_S_FIX = RW_S_LATCH;
constinit ulint MTR_MEMO_PAGE_X_FIX = RW_X_LATCH;
constinit ulint MTR_MEMO_BUF_FIX = RW_NO_LATCH;
constinit ulint MTR_MEMO_MODIFY = 54;
constinit ulint MTR_MEMO_S_LOCK = 55;
constinit ulint MTR_MEMO_X_LOCK = 56;

/** @name Log item types
The log items are declared 'byte' so that the compiler can warn if val
and type parameters are switched in a call to mlog_write_ulint. NOTE!
For 1 - 8 bytes, the flag value must give the length also! @{ */
constinit ulint MLOG_SINGLE_REC_FLAG = 128;
constinit ulint MLOG_1BYTE = (1);
constinit ulint MLOG_2BYTES = (2);
constinit ulint MLOG_4BYTES = (4);
constinit ulint MLOG_8BYTES = (8);
constinit byte MLOG_REC_INSERT = ((byte)9);
constinit byte MLOG_REC_CLUST_DELETE_MARK = ((byte)10);
constinit byte MLOG_REC_SEC_DELETE_MARK = ((byte)11);
constinit byte MLOG_REC_UPDATE_IN_PLACE = ((byte)13);
constinit byte MLOG_REC_DELETE = ((byte)14);
constinit byte MLOG_LIST_END_DELETE = ((byte)15);
constinit byte MLOG_LIST_START_DELETE = ((byte)16);
constinit byte MLOG_LIST_END_COPY_CREATED = ((byte)17);
constinit byte MLOG_PAGE_REORGANIZE = ((byte)18);
constinit byte MLOG_PAGE_CREATE = ((byte)19);
constinit byte MLOG_UNDO_INSERT = ((byte)20);
constinit byte MLOG_UNDO_ERASE_END = ((byte)21);
constinit byte MLOG_UNDO_INIT = ((byte)22);
constinit byte MLOG_UNDO_HDR_DISCARD = ((byte)23);
constinit byte MLOG_UNDO_HDR_REUSE = ((byte)24);
constinit byte MLOG_UNDO_HDR_CREATE = ((byte)25);
constinit byte MLOG_REC_MIN_MARK = ((byte)26);
constinit byte MLOG_IBUF_BITMAP_INIT = ((byte)27);
/*#define	MLOG_FULL_PAGE	((byte)28)	full contents of a page */
#ifdef IB_LOG_LSN_DEBUG
constinit byte MLOG_LSN = ((byte)28);
#endif
constinit byte MLOG_INIT_FILE_PAGE = ((byte)29);
constinit byte MLOG_WRITE_STRING = ((byte)30);
constinit byte MLOG_MULTI_REC_END = ((byte)31);
constinit byte MLOG_DUMMY_RECORD = ((byte)32);
constinit byte MLOG_FILE_CREATE = ((byte)33);
constinit byte MLOG_FILE_RENAME = ((byte)34);
constinit byte MLOG_FILE_DELETE = ((byte)35);
constinit byte MLOG_COMP_REC_MIN_MARK = ((byte)36);
constinit byte MLOG_COMP_PAGE_CREATE = ((byte)37);
constinit byte MLOG_COMP_REC_INSERT = ((byte)38);
constinit byte MLOG_COMP_REC_CLUST_DELETE_MARK = ((byte)39);
constinit byte MLOG_COMP_REC_SEC_DELETE_MARK = ((byte)40);
constinit byte MLOG_COMP_REC_UPDATE_IN_PLACE = ((byte)41);
constinit byte MLOG_COMP_REC_DELETE = ((byte)42);
constinit byte MLOG_COMP_LIST_END_DELETE = ((byte)43);
constinit byte MLOG_COMP_LIST_START_DELETE = ((byte)44);
constinit byte MLOG_COMP_LIST_END_COPY_CREATED = ((byte)45);
constinit byte MLOG_COMP_PAGE_REORGANIZE = ((byte)46);
constinit byte MLOG_FILE_CREATE2 = ((byte)47);
constinit byte MLOG_ZIP_WRITE_NODE_PTR = ((byte)48);
constinit byte MLOG_ZIP_WRITE_BLOB_PTR = ((byte)49);
constinit byte MLOG_ZIP_WRITE_HEADER = ((byte)50);
constinit byte MLOG_ZIP_PAGE_COMPRESS = ((byte)51);
constinit byte MLOG_BIGGEST_TYPE = ((byte)51);
/* @} */

constinit ulint MLOG_FILE_FLAG_TEMP = 1;

#ifdef IB_DEBUG
constinit ulint MTR_MAGIC_N = 54551;
#endif /* IB_DEBUG */

constinit ulint MTR_ACTIVE = 12231;
constinit ulint MTR_COMMITTING = 56456;
constinit ulint MTR_COMMITTED = 34676;

constinit ulint MTR_BUF_MEMO_SIZE = 200;

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Starts a mini-transaction and creates a mini-transaction handle
/// and buffer in the memory buffer given by the caller.
/// \param mtr Memory buffer for the mtr buffer.
/// \return Mtr buffer which also acts as the mtr handle.
IB_INLINE
mtr_t*
mtr_start(mtr_t* mtr);
/// \brief Commits a mini-transaction.
/// \param mtr Mini-transaction.
IB_INTERN
void
mtr_commit(mtr_t* mtr);
/// \brief Sets and returns a savepoint in mtr.
/// \param mtr Mtr.
/// \return Savepoint.
IB_INLINE
ulint
mtr_set_savepoint(mtr_t* mtr);
/// \brief Releases the latches stored in an mtr memo down to a savepoint.
/// \details NOTE! The mtr must not have made changes to buffer pages after the
/// savepoint, as these can be handled only by mtr_commit.
/// \param mtr Mtr.
/// \param savepoint Savepoint.
IB_INTERN
void
mtr_rollback_to_savepoint(mtr_t* mtr, ulint savepoint);
#ifndef IB_HOTBACKUP
/// \brief Releases the (index tree) s-latch stored in an mtr memo after a savepoint.
/// \param mtr Mtr.
/// \param savepoint Savepoint.
/// \param lock Latch to release.
IB_INLINE
void
mtr_release_s_latch_at_savepoint(mtr_t* mtr, ulint savepoint, rw_lock_t* lock);
#else /* !IB_HOTBACKUP */
# define mtr_release_s_latch_at_savepoint(mtr,savepoint,lock) ((void) 0)
#endif /* !IB_HOTBACKUP */
/// \brief Gets the logging mode of a mini-transaction.
/// \param mtr Mtr.
/// \return Logging mode: MTR_LOG_NONE, ...
IB_INLINE
ulint
mtr_get_log_mode(mtr_t* mtr);
/// \brief Changes the logging mode of a mini-transaction.
/// \param mtr Mtr.
/// \param mode Logging mode: MTR_LOG_NONE, ...
/// \return Old mode.
IB_INLINE
ulint
mtr_set_log_mode(mtr_t* mtr, ulint mode);
/// \brief Reads 1 - 4 bytes from a file page buffered in the buffer pool.
/// \param ptr Pointer from where to read.
/// \param type MLOG_1BYTE, MLOG_2BYTES, MLOG_4BYTES.
/// \param mtr Mini-transaction handle.
/// \return Value read.
IB_INTERN
ulint
mtr_read_ulint(const byte* ptr, ulint type, mtr_t* mtr);
/// \brief Reads 8 bytes from a file page buffered in the buffer pool.
/// \param ptr Pointer from where to read.
/// \param mtr Mini-transaction handle.
/// \return Value read.
IB_INTERN
dulint
mtr_read_dulint(const byte* ptr, mtr_t* mtr);
#ifndef IB_HOTBACKUP
/**
This macro locks an rw-lock in s-mode. */
#define mtr_s_lock(B, MTR)	mtr_s_lock_func((B), __FILE__, __LINE__, (MTR))
/**
This macro locks an rw-lock in x-mode. */
#define mtr_x_lock(B, MTR)	mtr_x_lock_func((B), __FILE__, __LINE__, (MTR))
/**
NOTE! Use the macro above!
Locks a lock in s-mode. */
IB_INLINE void mtr_s_lock_func(

	rw_lock_t*	lock,	/*!< in: rw-lock */
	const char*	file,	/*!< in: file name */
	ulint		line,	/*!< in: line number */
	mtr_t*		mtr);	/*!< in: mtr */
/**
NOTE! Use the macro above!
Locks a lock in x-mode. */
IB_INLINE void mtr_x_lock_func(
	rw_lock_t*	lock,	/*!< in: rw-lock */
	const char*	file,	/*!< in: file name */
	ulint		line,	/*!< in: line number */
	mtr_t*		mtr);	/*!< in: mtr */
#endif // !IB_HOTBACKUP

/// \brief Releases an object in the memo stack.
/// \param [in] mtr mtr
/// \param [in] object object
/// \param [in] type object type: MTR_MEMO_S_LOCK, ...
IB_INTERN void mtr_memo_release(mtr_t* mtr, void* object, ulint type);
#ifdef IB_DEBUG
# ifndef IB_HOTBACKUP
/// \brief Checks if memo contains the given item.
/// \param [in] mtr mtr
/// \param [in] object object to search
/// \param [in] type type of object
/// \return TRUE if contains
IB_INLINE ibool mtr_memo_contains(mtr_t* mtr, const void* object, ulint type);

/// \brief Checks if memo contains the given page.
/// \param mtr Mtr.
/// \param ptr Pointer to buffer frame.
/// \param type Type of object.
/// \return TRUE if contains.
IB_INTERN
ibool
mtr_memo_contains_page(mtr_t* mtr, const byte* ptr, ulint type);
/// \brief Prints info of an mtr handle.
/// \param mtr Mtr.
IB_INTERN
void
mtr_print(mtr_t* mtr);
# else /* !IB_HOTBACKUP */
#  define mtr_memo_contains(mtr, object, type)		TRUE
#  define mtr_memo_contains_page(mtr, ptr, type)	TRUE
# endif /* !IB_HOTBACKUP */
#endif /* IB_DEBUG */

/// \brief Pushes an object to an mtr memo stack.
/// \param mtr Mtr.
/// \param object Object.
/// \param type Object type: MTR_MEMO_S_LOCK, ...
IB_INLINE
void
mtr_memo_push(mtr_t* mtr, void* object, ulint type);

#ifndef IB_DO_NOT_INLINE
#include "mtr0mtr.inl"
#endif

#endif
