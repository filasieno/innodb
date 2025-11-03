// Copyright (c) 1995, 2010, Innobase Oy. All Rights Reserved.
// Copyright (c) 2009, Google Inc.
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

/// \file log_log.hpp
/// \brief Database log
/// \details Originally created by Heikki Tuuri on 12/9/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "ut_byte.hpp"
#include "ut_lst.hpp"
#ifndef IB_HOTBACKUP
#include "sync_sync.hpp"
#include "sync_rw.hpp"
#endif /* !IB_HOTBACKUP */
#include "srv_srv.hpp"
#include "api_api.hpp"

/** Redo log buffer */
typedef struct log_struct	log_t;
/** Redo log group */
typedef struct log_group_struct	log_group_t;

#ifdef IB_DEBUG
/** Flag: write to log file? */
extern	ibool	log_do_write;
/** Flag: enable debug output when writing to the log? */
extern	ibool	log_debug_writes;
#else /* IB_DEBUG */
/** Write to log */
# define log_do_write TRUE
#endif /* IB_DEBUG */

/** Wait modes for log_write_up_to @{ */
constinit ulint LOG_NO_WAIT = 91;
constinit ulint LOG_WAIT_ONE_GROUP = 92;
constinit ulint LOG_WAIT_ALL_GROUPS = 93;
/* @} */
/** Maximum number of log groups in log_group_struct::checkpoint_buf */
constinit ulint LOG_MAX_N_GROUPS = 32;

#ifndef IB_HOTBACKUP
/// \brief Sets the global variable log_fsp_current_free_limit.
/// \details Also makes a checkpoint, so that we know that the limit has been written to a log checkpoint field on disk.
/// \param [in] limit to set
IB_INTERN void log_fsp_current_free_limit_set_and_checkpoint(ulint limit);
#endif /* !IB_HOTBACKUP */
/// \brief Calculates where in log files we find a specified lsn.
/// \return log file number
/// \param [out] log_file_offset offset in that file (including the header)
/// \param [in] first_header_lsn first log file start lsn
/// \param [in] lsn lsn whose position to determine
/// \param [in] n_log_files total number of log files
/// \param [in] log_file_size log file size (including the header)
IB_INTERN ulint log_calc_where_lsn_is(ib_int64_t* log_file_offset, ib_uint64_t first_header_lsn, ib_uint64_t lsn, ulint n_log_files, ib_int64_t log_file_size);
#ifndef IB_HOTBACKUP
/// \brief Acquire the log mutex.
IB_INLINE void log_acquire(void);

/// \brief Writes to the log the string given.
/// \details The log must be released with log_release.
/// \return end lsn of the log record, zero if did not succeed
/// \param [in] str string
/// \param [in] len string length
/// \param [out] start_lsn start lsn of the log record
IB_INLINE ib_uint64_t log_reserve_and_write_fast(const void* str, ulint len, ib_uint64_t* start_lsn);

/// \brief Releases the log mutex.
IB_INLINE void log_release(void);

/// \brief Checks if there is need for a log buffer flush or a new checkpoint, and does this if yes.
/// \details Any database operation should call this when it has modified more than about 4 pages. NOTE that this function may only be called when the OS thread owns no synchronization objects except the dictionary mutex.
IB_INLINE void log_free_check(void);
/// \brief Opens the log for log_write_low.
/// \details The log must be closed with log_close and released with log_release.
/// \return start lsn of the log record
/// \param [in] len length of data to be catenated
IB_INTERN ib_uint64_t log_reserve_and_open(ulint len);

/// \brief Writes to the log the string given.
/// \details It is assumed that the caller holds the log mutex.
/// \param [in] str string
/// \param [in] str_len string length
IB_INTERN void log_write_low(byte* str, ulint str_len);

/// \brief Closes the log.
/// \return lsn
/// \param [in] recovery recovery flag
IB_INTERN ib_uint64_t log_close(ib_recovery_t recovery);

/// \brief Gets the current lsn.
/// \return current lsn
IB_INLINE ib_uint64_t log_get_lsn(void);

/// \brief Gets the log group capacity.
/// \details It is OK to read the value without holding log_sys->mutex because it is constant.
/// \return log group capacity
IB_INLINE ulint log_get_capacity(void);
/// \brief Initializes the log.
IB_INTERN void innobase_log_init(void);

/// \brief Inits a log group to the log system.
/// \param [in] id group id
/// \param [in] n_files number of log files
/// \param [in] file_size log file size in bytes
/// \param [in] space_id space id of the file space which contains the log files of this group
/// \param [in] archive_space_id space id of the file space which contains some archived log files for this group; currently, only for the first log group this is used
IB_INTERN void log_group_init(ulint id, ulint n_files, ulint file_size, ulint space_id, ulint archive_space_id);

/// \brief Completes an i/o to a log file.
/// \param [in] group log group
IB_INTERN void log_io_complete(log_group_t* group);
/// \brief This function is called, e.g., when a transaction wants to commit.
/// \details It checks that the log has been written to the log file up to the last log entry written by the transaction. If there is a flush running, it waits and checks if the flush flushed enough. If not, starts a new flush.
/// \param [in] lsn log sequence number up to which the log should be written, IB_UINT64_T_MAX if not specified
/// \param [in] wait LOG_NO_WAIT, LOG_WAIT_ONE_GROUP, or LOG_WAIT_ALL_GROUPS
/// \param [in] flush_to_disk TRUE if we want the written log also to be flushed to disk
IB_INTERN void log_write_up_to(ib_uint64_t lsn, ulint wait, ibool flush_to_disk);

/// \brief Does a syncronous flush of the log buffer to disk.
IB_INTERN void log_buffer_flush_to_disk(void);

/// \brief This functions writes the log buffer to the log file and if 'flush' is set it forces a flush of the log file as well.
/// \details This is meant to be called from background master thread only as it does not wait for the write (+ possible flush) to finish.
/// \param [in] flush flush the logs to disk
IB_INTERN void log_buffer_sync_in_background(ibool flush);
/// \brief Advances the smallest lsn for which there are unflushed dirty blocks in the buffer pool and also may make a new checkpoint.
/// \details NOTE: this function may only be called if the calling thread owns no synchronization objects!
/// \return FALSE if there was a flush batch of the same type running, which means that we could not start this flush batch
/// \param [in] new_oldest try to advance oldest_modified_lsn at least to this lsn
/// \param [in] sync TRUE if synchronous operation is desired
IB_INTERN ibool log_preflush_pool_modified_pages(ib_uint64_t new_oldest, ibool sync);

/// \brief Makes a checkpoint.
/// \details Note that this function does not flush dirty blocks from the buffer pool: it only checks what is lsn of the oldest modification in the pool, and writes information about the lsn in log files. Use log_make_checkpoint_at to flush also the pool.
/// \return TRUE if success, FALSE if a checkpoint write was already running
/// \param [in] sync TRUE if synchronous operation is desired
/// \param [in] write_always the function normally checks if the new checkpoint would have a greater lsn than the previous one: if not, then no physical write is done; by setting this parameter TRUE, a physical write will always be made to log files
IB_INTERN ibool log_checkpoint(ibool sync, ibool write_always);

/// \brief Makes a checkpoint at a given lsn or later.
/// \param [in] lsn make a checkpoint at this or a later lsn, if IB_UINT64_T_MAX, makes a checkpoint at the latest lsn
/// \param [in] write_always the function normally checks if the new checkpoint would have a greater lsn than the previous one: if not, then no physical write is done; by setting this parameter TRUE, a physical write will always be made to log files
IB_INTERN void log_make_checkpoint_at(ib_uint64_t lsn, ibool write_always);
/// \brief Makes a checkpoint at the latest lsn and writes it to first page of each data file in the database, so that we know that the file spaces contain all modifications up to that lsn.
/// \details This can only be called at database shutdown. This function also writes all log in log files to the log archive.
/// \param [in] recovery recovery flag
/// \param [in] shutdown shutdown flag
IB_INTERN void logs_empty_and_mark_files_at_shutdown(ib_recovery_t recovery, ib_shutdown_t shutdown);

/// \brief Reads a checkpoint info from a log group header to log_sys->checkpoint_buf.
/// \param [in] group log group
/// \param [in] field LOG_CHECKPOINT_1 or LOG_CHECKPOINT_2
IB_INTERN void log_group_read_checkpoint_info(log_group_t* group, ulint field);

/// \brief Gets info from a checkpoint about a log group.
/// \param [in] buf buffer containing checkpoint info
/// \param [in] n nth slot
/// \param [out] file_no archived file number
/// \param [out] offset archived file offset
IB_INTERN void log_checkpoint_get_nth_group_info(const byte* buf, ulint n, ulint* file_no, ulint* offset);

/// \brief Writes checkpoint info to groups.
IB_INTERN void log_groups_write_checkpoint_info(void);
/// \brief Starts an archiving operation.
/// \return TRUE if succeed, FALSE if an archiving operation was already running
/// \param [in] sync TRUE if synchronous operation is desired
/// \param [out] n_bytes archive log buffer size, 0 if nothing to archive
IB_INTERN ibool log_archive_do(ibool sync, ulint* n_bytes);

/// \brief Writes the log contents to the archive up to the lsn when this function was called, and stops the archiving.
/// \details When archiving is started again, the archived log file numbers start from a number one higher, so that the archiving will not write again to the archived log files which exist when this function returns.
/// \return DB_SUCCESS or DB_ERROR
IB_INTERN ulint log_archive_stop(void);

/// \brief Starts again archiving which has been stopped.
/// \return DB_SUCCESS or DB_ERROR
IB_INTERN ulint log_archive_start(void);

/// \brief Stop archiving the log so that a gap may occur in the archived log files.
/// \return DB_SUCCESS or DB_ERROR
IB_INTERN ulint log_archive_noarchivelog(void);

/// \brief Start archiving the log so that a gap may occur in the archived log files.
/// \return DB_SUCCESS or DB_ERROR
IB_INTERN ulint log_archive_archivelog(void);
/// \brief Generates an archived log file name.
/// \param [in] buf buffer where to write
/// \param [in] id group id
/// \param [in] file_no file number
IB_INTERN void log_archived_file_name_gen(char* buf, ulint id, ulint file_no);
#else /* !IB_HOTBACKUP */
/// \brief Writes info to a buffer of a log group when log files are created in backup restoration.
/// \param [in] hdr_buf buffer which will be written to the start of the first log file
/// \param [in] start lsn of the start of the first log file; we pretend that there is a checkpoint at start + LOG_BLOCK_HDR_SIZE
IB_INTERN void log_reset_first_header_and_checkpoint(byte* hdr_buf, ib_uint64_t start);
#endif /* !IB_HOTBACKUP */
/// \brief Checks that there is enough free space in the log to start a new query step.
/// \details Flushes the log buffer or makes a new checkpoint if necessary. NOTE: this function may only be called if the calling thread owns no synchronization objects!
IB_INTERN void log_check_margins(void);
#ifndef IB_HOTBACKUP
/// \brief Reads a specified log segment to a buffer.
/// \param [in] type LOG_ARCHIVE or LOG_RECOVER
/// \param [in] buf buffer where to read
/// \param [in] group log group
/// \param [in] start_lsn read area start
/// \param [in] end_lsn read area end
IB_INTERN void log_group_read_log_seg(ulint type, byte* buf, log_group_t* group, ib_uint64_t start_lsn, ib_uint64_t end_lsn);

/// \brief Writes a buffer to a log file group.
/// \param [in] group log group
/// \param [in] buf buffer
/// \param [in] len buffer len; must be divisible by OS_FILE_LOG_BLOCK_SIZE
/// \param [in] start_lsn start lsn of the buffer; must be divisible by OS_FILE_LOG_BLOCK_SIZE
/// \param [in] new_data_offset start offset of new data in buf: this parameter is used to decide if we have to write a new log file header
IB_INTERN void log_group_write_buf(log_group_t* group, byte* buf, ulint len, ib_uint64_t start_lsn, ulint new_data_offset);

/// \brief Sets the field values in group to correspond to a given lsn.
/// \details For this function to work, the values must already be correctly initialized to correspond to some lsn, for instance, a checkpoint lsn.
/// \param [in,out] group group
/// \param [in] lsn lsn for which the values should be set
IB_INTERN void log_group_set_fields(log_group_t* group, ib_uint64_t lsn);

/// \brief Calculates the data capacity of a log group, when the log file headers are not included.
/// \return capacity in bytes
/// \param [in] group log group
IB_INTERN ulint log_group_get_capacity(const log_group_t* group);
#endif /* !IB_HOTBACKUP */
/// \brief Gets a log block flush bit.
/// \return TRUE if this block was the first to be written in a log flush
/// \param [in] log_block log block
IB_INLINE ibool log_block_get_flush_bit(const byte* log_block);

/// \brief Gets a log block number stored in the header.
/// \return log block number stored in the block header
/// \param [in] log_block log block
IB_INLINE ulint log_block_get_hdr_no(const byte* log_block);

/// \brief Gets a log block data length.
/// \return log block data length measured as a byte offset from the block start
/// \param [in] log_block log block
IB_INLINE ulint log_block_get_data_len(const byte* log_block);

/// \brief Sets the log block data length.
/// \param [in,out] log_block log block
/// \param [in] len data length
IB_INLINE void log_block_set_data_len(byte* log_block, ulint len);

/// \brief Calculates the checksum for a log block.
/// \return checksum
/// \param [in] block log block
IB_INLINE ulint log_block_calc_checksum(const byte* block);

/// \brief Gets a log block checksum field value.
/// \return checksum
/// \param [in] log_block log block
IB_INLINE ulint log_block_get_checksum(const byte* log_block);

/// \brief Sets a log block checksum field value.
/// \param [in,out] log_block log block
/// \param [in] checksum checksum
IB_INLINE void log_block_set_checksum(byte* log_block, ulint checksum);

/// \brief Gets a log block first mtr log record group offset.
/// \return first mtr log record group byte offset from the block start, 0 if none
/// \param [in] log_block log block
IB_INLINE ulint log_block_get_first_rec_group(const byte* log_block);

/// \brief Sets the log block first mtr log record group offset.
/// \param [in,out] log_block log block
/// \param [in] offset offset, 0 if none
IB_INLINE void log_block_set_first_rec_group(byte* log_block, ulint offset);

/// \brief Gets a log block checkpoint number field (4 lowest bytes).
/// \return checkpoint no (4 lowest bytes)
/// \param [in] log_block log block
IB_INLINE ulint log_block_get_checkpoint_no(const byte* log_block);
/// \brief Initializes a log block in the log buffer.
/// \param [in] log_block pointer to the log buffer
/// \param [in] lsn lsn within the log block
IB_INLINE void log_block_init(byte* log_block, ib_uint64_t lsn);

/// \brief Initializes a log block in the log buffer in the old, < 3.23.52 format, where there was no checksum yet.
/// \param [in] log_block pointer to the log buffer
/// \param [in] lsn lsn within the log block
IB_INLINE void log_block_init_in_old_format(byte* log_block, ib_uint64_t lsn);

/// \brief Converts a lsn to a log block number.
/// \return log block number, it is > 0 and <= 1G
/// \param [in] lsn lsn of a byte within the block
IB_INLINE ulint log_block_convert_lsn_to_no(ib_uint64_t lsn);

/// \brief Prints info of the log.
/// \param [in] state->stream stream where to print
IB_INTERN void log_print(ib_stream_t state->stream);

/// \brief Peeks the current lsn.
/// \return TRUE if success, FALSE if could not get the log system mutex
/// \param [out] lsn if returns TRUE, current lsn is here
IB_INTERN ibool log_peek_lsn(ib_uint64_t* lsn);

/// \brief Refreshes the statistics used to print per-second averages.
IB_INTERN void log_refresh_stats(void);

/// \brief Shutdown log system but doesn't release all the memory.
IB_INTERN void log_shutdown(void);

/// \brief Reset the variables.
IB_INTERN void log_var_init(void);

/// \brief Free the log system data structures.
IB_INTERN void log_mem_free(void);

extern log_t*	log_sys;

/* Values used as flags */
constinit ulint LOG_FLUSH = 7652559;
constinit ulint LOG_CHECKPOINT = 78656949;
#ifdef IB_LOG_ARCHIVE
constinit ulint LOG_ARCHIVE = 11122331;
#endif /* IB_LOG_ARCHIVE */
constinit ulint LOG_RECOVER = 98887331;

/* The counting of lsn's starts from this value: this must be non-zero */
constinit ib_uint64_t LOG_START_LSN = ((ib_uint64_t) (16 * OS_FILE_LOG_BLOCK_SIZE));

constinit ulint LOG_BUFFER_SIZE = (srv_log_buffer_size * IB_PAGE_SIZE);
constinit ulint LOG_ARCHIVE_BUF_SIZE = (srv_log_buffer_size * IB_PAGE_SIZE / 4);

/* Offsets of a log block header */
constinit ulint LOG_BLOCK_HDR_NO = 0;	/* block number which must be > 0 and
					is allowed to wrap around at 2G; the
					highest bit is set to 1 if this is the
					first log block in a log flush write
					segment */
constinit ulint LOG_BLOCK_FLUSH_BIT_MASK = 0x80000000UL;
					/* mask used to get the highest bit in
					the preceding field */
constinit ulint LOG_BLOCK_HDR_DATA_LEN = 4;	/* number of bytes of log written to
					this block */
constinit ulint LOG_BLOCK_FIRST_REC_GROUP = 6;	/* offset of the first start of an
					mtr log record group in this log block,
					0 if none; if the value is the same
					as LOG_BLOCK_HDR_DATA_LEN, it means
					that the first rec group has not yet
					been catenated to this log block, but
					if it will, it will start at this
					offset; an archive recovery can
					start parsing the log records starting
					from this offset in this log block,
					if value not 0 */
constinit ulint LOG_BLOCK_CHECKPOINT_NO = 8;	/* 4 lower bytes of the value of
					log_sys->next_checkpoint_no when the
					log block was last written to: if the
					block has not yet been written full,
					this value is only updated before a
					log buffer flush */
constinit ulint LOG_BLOCK_HDR_SIZE = 12;	/* size of the log block header in
					bytes */

/* Offsets of a log block trailer from the end of the block */
constinit ulint LOG_BLOCK_CHECKSUM = 4;	/* 4 byte checksum of the log block
					contents; in InnoDB versions
					< 3.23.52 this did not contain the
					checksum but the same value as
					.._HDR_NO */
constinit ulint LOG_BLOCK_TRL_SIZE = 4;	/* trailer size in bytes */

/* Offsets for a checkpoint field */
constinit ulint LOG_CHECKPOINT_NO = 0;
constinit ulint LOG_CHECKPOINT_LSN = 8;
constinit ulint LOG_CHECKPOINT_OFFSET = 16;
constinit ulint LOG_CHECKPOINT_LOG_BUF_SIZE = 20;
constinit ulint LOG_CHECKPOINT_ARCHIVED_LSN = 24;
constinit ulint LOG_CHECKPOINT_GROUP_ARRAY = 32;

/* For each value smaller than LOG_MAX_N_GROUPS the following 8 bytes: */

constinit ulint LOG_CHECKPOINT_ARCHIVED_FILE_NO = 0;
constinit ulint LOG_CHECKPOINT_ARCHIVED_OFFSET = 4;

constinit ulint LOG_CHECKPOINT_ARRAY_END = (LOG_CHECKPOINT_GROUP_ARRAY + LOG_MAX_N_GROUPS * 8);
constinit ulint LOG_CHECKPOINT_CHECKSUM_1 = LOG_CHECKPOINT_ARRAY_END;
constinit ulint LOG_CHECKPOINT_CHECKSUM_2 = (4 + LOG_CHECKPOINT_ARRAY_END);
constinit ulint LOG_CHECKPOINT_FSP_FREE_LIMIT = (8 + LOG_CHECKPOINT_ARRAY_END);
					/* current fsp free limit in
					tablespace 0, in units of one
					megabyte; this information is only used
					by ibbackup to decide if it can
					truncate unused ends of
					non-auto-extending data files in space
					0 */
constinit ulint LOG_CHECKPOINT_FSP_MAGIC_N = (12 + LOG_CHECKPOINT_ARRAY_END);
					/* this magic number tells if the
					checkpoint contains the above field:
					the field was added to
					InnoDB-3.23.50 */
constinit ulint LOG_CHECKPOINT_SIZE = (16 + LOG_CHECKPOINT_ARRAY_END);

constinit ulint LOG_CHECKPOINT_FSP_MAGIC_N_VAL = 1441231243;

/* Offsets of a log file header */
constinit ulint LOG_GROUP_ID = 0;	/* log group number */
constinit ulint LOG_FILE_START_LSN = 4;	/* lsn of the start of data in this
					log file */
constinit ulint LOG_FILE_NO = 12;	/* 4-byte archived log file number;
					this field is only defined in an
					archived log file */
constinit ulint LOG_FILE_WAS_CREATED_BY_HOT_BACKUP = 16;
					/* a 32-byte field which contains
					the string 'ibbackup' and the
					creation time if the log file was
					created by ibbackup --restore;
					when the application is started for
					the first time on the restored
					database, it can print helpful
					info for the user */
constinit ulint LOG_FILE_ARCH_COMPLETED = OS_FILE_LOG_BLOCK_SIZE;
					/* this 4-byte field is TRUE when
					the writing of an archived log file
					has been completed; this field is
					only defined in an archived log file */
constinit ulint LOG_FILE_END_LSN = (OS_FILE_LOG_BLOCK_SIZE + 4);
					/* lsn where the archived log file
					at least extends: actually the
					archived log file may extend to a
					later lsn, as long as it is within the
					same log block as this lsn; this field
					is defined only when an archived log
					file has been completely written */
constinit ulint LOG_CHECKPOINT_1 = OS_FILE_LOG_BLOCK_SIZE;
					/* first checkpoint field in the log
					header; we write alternately to the
					checkpoint fields when we make new
					checkpoints; this field is only defined
					in the first log file of a log group */
constinit ulint LOG_CHECKPOINT_2 = (3 * OS_FILE_LOG_BLOCK_SIZE);
					/* second checkpoint field in the log
					header */
constinit ulint LOG_FILE_HDR_SIZE = (4 * OS_FILE_LOG_BLOCK_SIZE);

constinit ulint LOG_GROUP_OK = 301;
constinit ulint LOG_GROUP_CORRUPTED = 302;

/** Log group consists of a number of log files, each of the same size; a log
group is implemented as a space in the sense of the module fil0fil. */
struct log_group_struct{
	/* The following fields are protected by log_sys->mutex */
	ulint		id;		/*!< log group id */
	ulint		n_files;	/*!< number of files in the group */
	ulint		file_size;	/*!< individual log file size in bytes,
					including the log file header */
	ulint		space_id;	/*!< file space which implements the log
					group */
	ulint		state;		/*!< LOG_GROUP_OK or
					LOG_GROUP_CORRUPTED */
	ib_uint64_t	lsn;		/*!< lsn used to fix coordinates within
					the log group */
	ulint		lsn_offset;	/*!< the offset of the above lsn */
	ulint		n_pending_writes;/*!< number of currently pending flush
					writes for this log group */
	byte**		file_header_bufs_ptr;/*!< unaligned buffers */
	byte**		file_header_bufs;/*!< buffers for each file header
					 in the group */
#ifdef IB_LOG_ARCHIVE
	/*-----------------------------*/
#ifdef IB_LOG_ARCHIVE
	byte**		archive_file_header_bufs_ptr;/*!< unaligned buffers */
	byte**		archive_file_header_bufs;/*!< buffers for each file
					header in the group */
#endif
	ulint		archive_space_id;/*!< file space which implements
					the log group archive */
	ulint		archived_file_no;/*!< file number corresponding to
					log_sys->archived_lsn */
	ulint		archived_offset;/*!< file offset corresponding to
					log_sys->archived_lsn, 0 if we have
					not yet written to the archive file
					number archived_file_no */
	ulint		next_archived_file_no;/*!< during an archive write,
					until the write is completed, we
					store the next value for
					archived_file_no here: the write
					completion function then sets the new
					value to ..._file_no */
	ulint		next_archived_offset; /*!< like the preceding field */
#endif /* IB_LOG_ARCHIVE */
	/*-----------------------------*/
	ib_uint64_t	scanned_lsn;	/*!< used only in recovery: recovery scan
					succeeded up to this lsn in this log
					group */
	byte*		checkpoint_buf_ptr;/*!< unaligned checkpoint header */
	byte*		checkpoint_buf;	/*!< checkpoint header is written from
					this buffer to the group */
	UT_LIST_NODE_T(log_group_t)
			log_groups;	/*!< list of log groups */
};

/** Redo log buffer */
struct log_struct{
	byte		pad[64];	/*!< padding to prevent other memory
					update hotspots from residing on the
					same memory cache line */
	ib_uint64_t	lsn;		/*!< log sequence number */
	ulint		buf_free;	/*!< first free offset within the log
					buffer */
#ifndef IB_HOTBACKUP
	mutex_t		mutex;		/*!< mutex protecting the log */
#endif /* !IB_HOTBACKUP */
	byte*		buf_ptr;	/* unaligned log buffer */
	byte*		buf;		/*!< log buffer */
	ulint		buf_size;	/*!< log buffer size in bytes */
	ulint		max_buf_free;	/*!< recommended maximum value of
					buf_free, after which the buffer is
					flushed */
#ifdef IB_DEBUG
	ulint		old_buf_free;	/*!< value of buf free when log was
					last time opened; only in the debug
					version */
	ib_uint64_t	old_lsn;	/*!< value of lsn when log was
					last time opened; only in the
					debug version */
#endif
	ibool		check_flush_or_checkpoint;
					/*!< this is set to TRUE when there may
					be need to flush the log buffer, or
					preflush buffer pool pages, or make
					a checkpoint; this MUST be TRUE when
					lsn - last_checkpoint_lsn >
					max_checkpoint_age; this flag is
					peeked at by log_free_check(), which
					does not reserve the log mutex */
	UT_LIST_BASE_NODE_T(log_group_t)
			log_groups;	/*!< log groups */

#ifndef IB_HOTBACKUP
	/** The fields involved in the log buffer flush @{ */

	ulint		buf_next_to_write;/*!< first offset in the log buffer
					where the byte content may not exist
					written to file, e.g., the start
					offset of a log record catenated
					later; this is advanced when a flush
					operation is completed to all the log
					groups */
	ib_uint64_t	written_to_some_lsn;
					/*!< first log sequence number not yet
					written to any log group; for this to
					be advanced, it is enough that the
					write i/o has been completed for any
					one log group */
	ib_uint64_t	written_to_all_lsn;
					/*!< first log sequence number not yet
					written to some log group; for this to
					be advanced, it is enough that the
					write i/o has been completed for all
					log groups.
					Note that since InnoDB currently
					has only one log group therefore
					this value is redundant. Also it
					is possible that this value
					falls behind the
					flushed_to_disk_lsn transiently.
					It is appropriate to use either
					flushed_to_disk_lsn or
					write_lsn which are always
					up-to-date and accurate. */
	ib_uint64_t	write_lsn;	/*!< end lsn for the current running
					write */
	ulint		write_end_offset;/*!< the data in buffer has
					been written up to this offset
					when the current write ends:
					this field will then be copied
					to buf_next_to_write */
	ib_uint64_t	current_flush_lsn;/*!< end lsn for the current running
					write + flush operation */
	ib_uint64_t	flushed_to_disk_lsn;
					/*!< how far we have written the log
					AND flushed to disk */
	ulint		n_pending_writes;/*!< number of currently
					pending flushes or writes */
	/* NOTE on the 'flush' in names of the fields below: starting from
	4.0.14, we separate the write of the log file and the actual fsync()
	or other method to flush it to disk. The names below shhould really
	be 'flush_or_write'! */
	os_event_t	no_flush_event;	/*!< this event is in the reset state
					when a flush or a write is running;
					a thread should wait for this without
					owning the log mutex, but NOTE that
					to set or reset this event, the
					thread MUST own the log mutex! */
	ibool		one_flushed;	/*!< during a flush, this is
					first FALSE and becomes TRUE
					when one log group has been
					written or flushed */
	os_event_t	one_flushed_event;/*!< this event is reset when the
					flush or write has not yet completed
					for any log group; e.g., this means
					that a transaction has been committed
					when this is set; a thread should wait
					for this without owning the log mutex,
					but NOTE that to set or reset this
					event, the thread MUST own the log
					mutex! */
	ulint		n_log_ios;	/*!< number of log i/os initiated thus
					far */
	ulint		n_log_ios_old;	/*!< number of log i/o's at the
					previous printout */
	time_t		last_printout_time;/*!< when log_print was last time
					called */
	/* @} */

	/** Fields involved in checkpoints @{ */
	ulint		log_group_capacity; /*!< capacity of the log group; if
					the checkpoint age exceeds this, it is
					a serious error because it is possible
					we will then overwrite log and spoil
					crash recovery */
	ulint		max_modified_age_async;
					/*!< when this recommended
					value for lsn -
					buf_pool_get_oldest_modification()
					is exceeded, we start an
					asynchronous preflush of pool pages */
	ulint		max_modified_age_sync;
					/*!< when this recommended
					value for lsn -
					buf_pool_get_oldest_modification()
					is exceeded, we start a
					synchronous preflush of pool pages */
	ulint		adm_checkpoint_interval;
					/*!< administrator-specified checkpoint
					interval in terms of log growth in
					bytes; the interval actually used by
					the database can be smaller */
	ulint		max_checkpoint_age_async;
					/*!< when this checkpoint age
					is exceeded we start an
					asynchronous writing of a new
					checkpoint */
	ulint		max_checkpoint_age;
					/*!< this is the maximum allowed value
					for lsn - last_checkpoint_lsn when a
					new query step is started */
	ib_uint64_t	next_checkpoint_no;
					/*!< next checkpoint number */
	ib_uint64_t	last_checkpoint_lsn;
					/*!< latest checkpoint lsn */
	ib_uint64_t	next_checkpoint_lsn;
					/*!< next checkpoint lsn */
	ulint		n_pending_checkpoint_writes;
					/*!< number of currently pending
					checkpoint writes */
	rw_lock_t	checkpoint_lock;/*!< this latch is x-locked when a
					checkpoint write is running; a thread
					should wait for this without owning
					the log mutex */
#endif /* !IB_HOTBACKUP */
	byte*		checkpoint_buf_ptr;/* unaligned checkpoint header */
	byte*		checkpoint_buf;	/*!< checkpoint header is read to this
					buffer */
	/* @} */
#ifdef IB_LOG_ARCHIVE
	/** Fields involved in archiving @{ */
	ulint		archiving_state;/*!< LOG_ARCH_ON, LOG_ARCH_STOPPING
					LOG_ARCH_STOPPED, LOG_ARCH_OFF */
	ib_uint64_t	archived_lsn;	/*!< archiving has advanced to this
					lsn */
	ulint		max_archived_lsn_age_async;
					/*!< recommended maximum age of
					archived_lsn, before we start
					asynchronous copying to the archive */
	ulint		max_archived_lsn_age;
					/*!< maximum allowed age for
					archived_lsn */
	ib_uint64_t	next_archived_lsn;/*!< during an archive write,
					until the write is completed, we
					store the next value for
					archived_lsn here: the write
					completion function then sets the new
					value to archived_lsn */
	ulint		archiving_phase;/*!< LOG_ARCHIVE_READ or
					LOG_ARCHIVE_WRITE */
	ulint		n_pending_archive_ios;
					/*!< number of currently pending reads
					or writes in archiving */
	rw_lock_t	archive_lock;	/*!< this latch is x-locked when an
					archive write is running; a thread
					should wait for this without owning
					the log mutex */
	ulint		archive_buf_size;/*!< size of archive_buf */
	byte*		archive_buf;	/*!< log segment is written to the
					archive from this buffer */
	os_event_t	archiving_on;	/*!< if archiving has been stopped,
					a thread can wait for this event to
					become signaled */
	/* @} */
#endif /* IB_LOG_ARCHIVE */
};

#ifdef IB_LOG_ARCHIVE
/** Archiving state @{ */
#define LOG_ARCH_ON		71
#define LOG_ARCH_STOPPING	72
#define LOG_ARCH_STOPPING2	73
#define LOG_ARCH_STOPPED	74
#define LOG_ARCH_OFF		75
/* @} */
#endif /* IB_LOG_ARCHIVE */

#ifndef IB_DO_NOT_INLINE
#include "log0log.inl"
#endif

#endif
