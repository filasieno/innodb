// Copyright (c) 1995, 2010, Innobase Oy. All Rights Reserved.
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

/// \file log_log.inl
/// \brief Database log inline functions
/// \details Originally created by Heikki Tuuri on 12/9/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "os_file.hpp"
#include "mach_data.hpp"
#include "mtr_mtr.hpp"
#include "srv_srv.hpp"

/// \brief Acquire the log mutex.
IB_INLINE void log_acquire(void)
{
    ut_ad(!mutex_own(&log_sys->mutex));
    mutex_enter(&log_sys->mutex);
}

/// \brief Releases the log mutex.
IB_INLINE void log_release(void)
{
    ut_ad(mutex_own(&log_sys->mutex));
    mutex_exit(&log_sys->mutex);
}

#ifdef IB_LOG_DEBUG
/// \brief Checks by parsing that the catenated log segment for a single mtr is consistent.
/// \param [in] buf pointer to the start of the log segment in the log_sys->buf log buffer
/// \param [in] len segment length in bytes
/// \param [in] buf_start_lsn buffer start lsn
IB_INTERN ibool log_check_log_recs(const byte* buf, ulint len, ib_uint64_t buf_start_lsn);
#endif /* IB_LOG_DEBUG */

/// \brief Gets a log block flush bit.
/// \return TRUE if this block was the first to be written in a log flush
/// \param [in] log_block log block
IB_INLINE ibool log_block_get_flush_bit(const byte* log_block)
{
    ulint field_value = mach_read_from_4(log_block + LOG_BLOCK_HDR_NO);
    if (LOG_BLOCK_FLUSH_BIT_MASK & field_value) {
        return TRUE;
    }
    return FALSE;
}

/// \brief Sets the log block flush bit.
/// \param [in,out] log_block log block
/// \param [in] val value to set
IB_INLINE void log_block_set_flush_bit(byte* log_block, ibool val)
{
    ulint field = mach_read_from_4(log_block + LOG_BLOCK_HDR_NO);
    if (val) {
        field = field | LOG_BLOCK_FLUSH_BIT_MASK;
    } else {
        field = field & ~LOG_BLOCK_FLUSH_BIT_MASK;
    }
    mach_write_to_4(log_block + LOG_BLOCK_HDR_NO, field);
}

/// \brief Gets a log block number stored in the header.
/// \return log block number stored in the block header
/// \param [in] log_block log block
IB_INLINE ulint log_block_get_hdr_no(const byte* log_block)
{
    return (~LOG_BLOCK_FLUSH_BIT_MASK & mach_read_from_4(log_block + LOG_BLOCK_HDR_NO));
}

/************************************************************//**
Sets the log block number stored in the header; NOTE that this must be set
before the flush bit! */
IB_INLINE
void
log_block_set_hdr_no(
/*=================*/
	byte*	log_block,	/*!< in/out: log block */
	ulint	n)		/*!< in: log block number: must be > 0 and
				< LOG_BLOCK_FLUSH_BIT_MASK */
{
	ut_ad(n > 0);
	ut_ad(n < LOG_BLOCK_FLUSH_BIT_MASK);

	mach_write_to_4(log_block + LOG_BLOCK_HDR_NO, n);
}

/************************************************************//**
Gets a log block data length.
@return	log block data length measured as a byte offset from the block start */
IB_INLINE
ulint
log_block_get_data_len(
/*===================*/
	const byte*	log_block)	/*!< in: log block */
{
	return(mach_read_from_2(log_block + LOG_BLOCK_HDR_DATA_LEN));
}

/************************************************************//**
Sets the log block data length. */
IB_INLINE
void
log_block_set_data_len(
/*===================*/
	byte*	log_block,	/*!< in/out: log block */
	ulint	len)		/*!< in: data length */
{
	mach_write_to_2(log_block + LOG_BLOCK_HDR_DATA_LEN, len);
}

/************************************************************//**
Gets a log block first mtr log record group offset.
@return first mtr log record group byte offset from the block start, 0
if none */
IB_INLINE
ulint
log_block_get_first_rec_group(
/*==========================*/
	const byte*	log_block)	/*!< in: log block */
{
	return(mach_read_from_2(log_block + LOG_BLOCK_FIRST_REC_GROUP));
}

/************************************************************//**
Sets the log block first mtr log record group offset. */
IB_INLINE
void
log_block_set_first_rec_group(
/*==========================*/
	byte*	log_block,	/*!< in/out: log block */
	ulint	offset)		/*!< in: offset, 0 if none */
{
	mach_write_to_2(log_block + LOG_BLOCK_FIRST_REC_GROUP, offset);
}

/************************************************************//**
Gets a log block checkpoint number field (4 lowest bytes).
@return	checkpoint no (4 lowest bytes) */
IB_INLINE
ulint
log_block_get_checkpoint_no(
/*========================*/
	const byte*	log_block)	/*!< in: log block */
{
	return(mach_read_from_4(log_block + LOG_BLOCK_CHECKPOINT_NO));
}

/************************************************************//**
Sets a log block checkpoint number field (4 lowest bytes). */
IB_INLINE
void
log_block_set_checkpoint_no(
/*========================*/
	byte*		log_block,	/*!< in/out: log block */
	ib_uint64_t	no)		/*!< in: checkpoint no */
{
	mach_write_to_4(log_block + LOG_BLOCK_CHECKPOINT_NO, (ulint) no);
}

/************************************************************//**
Converts a lsn to a log block number.
@return	log block number, it is > 0 and <= 1G */
IB_INLINE
ulint
log_block_convert_lsn_to_no(
/*========================*/
	ib_uint64_t	lsn)	/*!< in: lsn of a byte within the block */
{
	return(((ulint) (lsn / OS_FILE_LOG_BLOCK_SIZE) & 0x3FFFFFFFUL) + 1);
}

/************************************************************//**
Calculates the checksum for a log block.
@return	checksum */
IB_INLINE
ulint
log_block_calc_checksum(
/*====================*/
	const byte*	block)	/*!< in: log block */
{
	ulint	sum;
	ulint	sh;
	ulint	i;

	sum = 1;
	sh = 0;

	for (i = 0; i < OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE; i++) {
		ulint	b = (ulint) block[i];
		sum &= 0x7FFFFFFFUL;
		sum += b;
		sum += b << sh;
		sh++;
		if (sh > 24) {
			sh = 0;
		}
	}

	return(sum);
}

/************************************************************//**
Gets a log block checksum field value.
@return	checksum */
IB_INLINE
ulint
log_block_get_checksum(
/*===================*/
	const byte*	log_block)	/*!< in: log block */
{
	return(mach_read_from_4(log_block + OS_FILE_LOG_BLOCK_SIZE
				- LOG_BLOCK_CHECKSUM));
}

/************************************************************//**
Sets a log block checksum field value. */
IB_INLINE
void
log_block_set_checksum(
/*===================*/
	byte*	log_block,	/*!< in/out: log block */
	ulint	checksum)	/*!< in: checksum */
{
	mach_write_to_4(log_block + OS_FILE_LOG_BLOCK_SIZE
			- LOG_BLOCK_CHECKSUM,
			checksum);
}

/************************************************************//**
Initializes a log block in the log buffer. */
IB_INLINE
void
log_block_init(
/*===========*/
	byte*		log_block,	/*!< in: pointer to the log buffer */
	ib_uint64_t	lsn)		/*!< in: lsn within the log block */
{
	ulint	no;

	ut_ad(mutex_own(&(log_sys->mutex)));

	no = log_block_convert_lsn_to_no(lsn);

	log_block_set_hdr_no(log_block, no);

	log_block_set_data_len(log_block, LOG_BLOCK_HDR_SIZE);
	log_block_set_first_rec_group(log_block, 0);
}

/************************************************************//**
Initializes a log block in the log buffer in the old format, where there
was no checksum yet. */
IB_INLINE
void
log_block_init_in_old_format(
/*=========================*/
	byte*		log_block,	/*!< in: pointer to the log buffer */
	ib_uint64_t	lsn)		/*!< in: lsn within the log block */
{
	ulint	no;

	ut_ad(mutex_own(&(log_sys->mutex)));

	no = log_block_convert_lsn_to_no(lsn);

	log_block_set_hdr_no(log_block, no);
	mach_write_to_4(log_block + OS_FILE_LOG_BLOCK_SIZE
			- LOG_BLOCK_CHECKSUM, no);
	log_block_set_data_len(log_block, LOG_BLOCK_HDR_SIZE);
	log_block_set_first_rec_group(log_block, 0);
}

#ifndef IB_HOTBACKUP
/************************************************************//**
Writes to the log the string given. The log must be released with
log_release().
@return	end lsn of the log record, zero if did not succeed */
IB_INLINE
ib_uint64_t
log_reserve_and_write_fast(
/*=======================*/
	const void*	str,	/*!< in: string */
	ulint		len,	/*!< in: string length */
	ib_uint64_t*	start_lsn)/*!< out: start lsn of the log record */
{
	ulint		data_len;
#ifdef IB_LOG_LSN_DEBUG
	/* length of the LSN pseudo-record */
	ulint		lsn_len;
#endif /* IB_LOG_LSN_DEBUG */

#ifdef IB_LOG_LSN_DEBUG
	lsn_len = 1
		+ mach_get_compressed_size(log_sys->lsn >> 32)
		+ mach_get_compressed_size(log_sys->lsn & 0xFFFFFFFFUL);
#endif /* IB_LOG_LSN_DEBUG */

	data_len = len
#ifdef IB_LOG_LSN_DEBUG
		+ lsn_len
#endif /* IB_LOG_LSN_DEBUG */
		+ log_sys->buf_free % OS_FILE_LOG_BLOCK_SIZE;

	if (data_len >= OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE) {

		/* The string does not fit within the current log block
		or the log block would become full */

		return(0);
	}

	*start_lsn = log_sys->lsn;

#ifdef IB_LOG_LSN_DEBUG
	{
		/* Write the LSN pseudo-record. */
		byte* b = &log_sys->buf[log_sys->buf_free];
		*b++ = MLOG_LSN | (MLOG_SINGLE_REC_FLAG & *(const byte*) str);
		/* Write the LSN in two parts,
		as a pseudo page number and space id. */
		b += mach_write_compressed(b, log_sys->lsn >> 32);
		b += mach_write_compressed(b, log_sys->lsn & 0xFFFFFFFFUL);
		ut_a(b - lsn_len == &log_sys->buf[log_sys->buf_free]);

		memcpy(b, str, len);
		len += lsn_len;
	}
#else /* IB_LOG_LSN_DEBUG */
	memcpy(log_sys->buf + log_sys->buf_free, str, len);
#endif /* IB_LOG_LSN_DEBUG */

	log_block_set_data_len((byte*) ut_align_down(log_sys->buf
						     + log_sys->buf_free,
						     OS_FILE_LOG_BLOCK_SIZE),
			       data_len);
#ifdef IB_LOG_DEBUG
	log_sys->old_buf_free = log_sys->buf_free;
	log_sys->old_lsn = log_sys->lsn;
#endif
	log_sys->buf_free += len;

	ut_ad(log_sys->buf_free <= log_sys->buf_size);

	log_sys->lsn += len;

#ifdef IB_LOG_DEBUG
	log_check_log_recs(log_sys->buf + log_sys->old_buf_free,
			   log_sys->buf_free - log_sys->old_buf_free,
			   log_sys->old_lsn);
#endif

	return(log_sys->lsn);
}

/************************************************************//**
Gets the current lsn.
@return	current lsn */
IB_INLINE
ib_uint64_t
log_get_lsn(void)
/*=============*/
{
	ib_uint64_t	lsn;

	log_acquire();

	lsn = log_sys->lsn;

	log_release();

	return(lsn);
}

/****************************************************************
Gets the log group capacity. It is OK to read the value without
holding log_sys->mutex because it is constant.
@return	log group capacity */
IB_INLINE
ulint
log_get_capacity(void)
/*==================*/
{
	return(log_sys->log_group_capacity);
}

/***********************************************************************//**
Checks if there is need for a log buffer flush or a new checkpoint, and does
this if yes. Any database operation should call this when it has modified
more than about 4 pages. NOTE that this function may only be called when the
OS thread owns no synchronization objects except the dictionary mutex. */
IB_INLINE
void
log_free_check(void)
/*================*/
{
	/* ut_ad(sync_thread_levels_empty()); */

	if (log_sys->check_flush_or_checkpoint) {

		log_check_margins();
	}
}
#endif /* !IB_HOTBACKUP */
