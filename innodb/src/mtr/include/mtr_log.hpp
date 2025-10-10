// Copyright (c) 1995, 2025, Innobase Oy. All Rights Reserved.
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

/// @file mtr_log.hpp
/// \brief Mini-transaction logging routines
///
/// Created 12/7/1995 Heikki Tuuri
#pragma once

#include "dict_types.hpp"
#include "mtr_mtr.hpp"
#include "univ.i"

#ifndef UNIV_HOTBACKUP

/// \brief Writes 1 - 4 bytes to a file page buffered in the buffer pool.
/// \details Writes the corresponding log record to the mini-transaction log.
/// \param ptr Pointer where to write.
/// \param val Value to write.
/// \param type MLOG_1BYTE, MLOG_2BYTES, MLOG_4BYTES.
/// \param mtr Mini-transaction handle.
IB_INTERN void mlog_write_ulint(byte *ptr, ulint val, byte type, mtr_t *mtr);

/// \brief Writes 8 bytes to a file page buffered in the buffer pool.
/// \details Writes the corresponding log record to the mini-transaction log.
/// \param ptr Pointer where to write.
/// \param val Value to write.
/// \param mtr Mini-transaction handle.
IB_INTERN void mlog_write_dulint(byte *ptr, dulint val, mtr_t *mtr);

/// \brief Writes a string to a file page buffered in the buffer pool.
/// \details Writes the corresponding log record to the mini-transaction log.
/// \param ptr Pointer where to write.
/// \param str String to write.
/// \param len String length.
/// \param mtr Mini-transaction handle.
IB_INTERN void mlog_write_string(byte *ptr, const byte *str, ulint len, mtr_t *mtr);

/// \brief Logs a write of a string to a file page buffered in the buffer pool.
/// \details Writes the corresponding log record to the mini-transaction log.
/// \param ptr Pointer written to.
/// \param len String length.
/// \param mtr Mini-transaction handle.
IB_INTERN void mlog_log_string(byte *ptr, ulint len, mtr_t *mtr);

/// \brief Writes initial part of a log record consisting of one-byte item
/// type and four-byte space and page numbers.
/// \param ptr Pointer to (inside) a buffer frame holding the file page where
/// modification is made
/// \param type Log item type: MLOG_1BYTE, ...
/// \param mtr Mini-transaction handle.
IB_INTERN void mlog_write_initial_log_record(const byte *ptr, byte type, mtr_t *mtr);

/// \brief Writes a log record about an .ibd file create/delete/rename.
/// \param type MLOG_FILE_CREATE, MLOG_FILE_DELETE, or MLOG_FILE_RENAME.
/// \param space_id Space id, if applicable.
/// \param page_no Page number (not relevant currently).
/// \param log_ptr Pointer to mtr log which has been opened.
/// \param mtr Mtr.
/// \return New value of log_ptr.
IB_INLINE byte* mlog_write_initial_log_record_for_file_op(ulint type, ulint space_id, ulint page_no, byte *log_ptr, mtr_t *mtr);

/// \brief Catenates 1 - 4 bytes to the mtr log.
/// \param mtr Mtr.
/// \param val Value to write.
/// \param type MLOG_1BYTE, MLOG_2BYTES, MLOG_4BYTES.
IB_INLINE void mlog_catenate_ulint(mtr_t *mtr, ulint val, ulint type);

/// \brief Catenates n bytes to the mtr log.
/// \param mtr Mtr.
/// \param str String to write.
/// \param len String length.
IB_INTERN void mlog_catenate_string(mtr_t *mtr, const byte *str, ulint len);

/// \brief Catenates a compressed ulint to mlog.
/// \param mtr Mtr.
/// \param val Value to write.
IB_INLINE void mlog_catenate_ulint_compressed(mtr_t *mtr, ulint val);

/// \brief Catenates a compressed dulint to mlog.
/// \param mtr Mtr.
/// \param val Value to write.
IB_INLINE void mlog_catenate_dulint_compressed(mtr_t *mtr, dulint val);

/// \brief Opens a buffer to mlog. It must be closed with mlog_close.
/// \param mtr Mtr.
/// \param size Buffer size in bytes; MUST be smaller than DYN_ARRAY_DATA_SIZE!
/// \return Buffer, NULL if log mode MTR_LOG_NONE.
IB_INLINE byte * mlog_open(mtr_t *mtr, ulint size);

/// \brief Closes a buffer opened to mlog.
/// \param mtr Mtr.
/// \param ptr Buffer space from ptr up was not used.
IB_INLINE void mlog_close(mtr_t *mtr, byte *ptr);

/// \brief Writes the initial part of a log record (3..11 bytes).
/// \details If the implementation of this function is changed, all size parameters to mlog_open() should be adjusted accordingly!
/// \param ptr Pointer to (inside) a buffer frame holding the file page where modification is made.
/// \param type Log item type: MLOG_1BYTE, ...
/// \param log_ptr Pointer to mtr log which has been opened.
/// \param mtr Mtr.
/// \return New value of log_ptr.
IB_INLINE byte* mlog_write_initial_log_record_fast(const byte *ptr, byte type, byte *log_ptr, mtr_t *mtr);

#else /* !UNIV_HOTBACKUP */

#define mlog_write_initial_log_record(ptr, type, mtr) ((void)0)

#define mlog_write_initial_log_record_fast(ptr, type, log_ptr, mtr) ((byte *)0)

#endif													   /* !UNIV_HOTBACKUP */
/// \brief Parses an initial log record written by mlog_write_initial_log_record.
/// \param ptr Buffer.
/// \param end_ptr Buffer end.
/// \param type Log record type: MLOG_1BYTE, ...
/// \param space Space id.
/// \param page_no Page number.
/// \return Parsed record end, NULL if not a complete record.
IB_INTERN byte* mlog_parse_initial_log_record(byte *ptr, byte *end_ptr, byte *type, ulint *space, ulint *page_no);

/// \brief Parses a log record written by mlog_write_ulint or mlog_write_dulint.
/// \param type Log record type: MLOG_1BYTE, ...
/// \param ptr Buffer.
/// \param end_ptr Buffer end.
/// \param page Page where to apply the log record, or NULL.
/// \param page_zip Compressed page, or NULL.
/// \return Parsed record end, NULL if not a complete record.
IB_INTERN byte* mlog_parse_nbytes(ulint type, byte *ptr, byte *end_ptr, byte *page, void *page_zip);

/// \brief Parses a log record written by mlog_write_string.
/// \param ptr Buffer.
/// \param end_ptr Buffer end.
/// \param page Page where to apply the log record, or NULL.
/// \param page_zip Compressed page, or NULL.
/// \return Parsed record end, NULL if not a complete record.
IB_INTERN byte* mlog_parse_string(byte *ptr, byte *end_ptr, byte *page, void *page_zip);

#ifndef UNIV_HOTBACKUP

/// \brief Opens a buffer for mlog, writes the initial log record and,
/// if needed, the field lengths of an index. Reserves space
/// for further log entries. The log entry must be closed with mtr_close().
/// \param mtr Mtr.
/// \param rec Index record or page.
/// \param index Record descriptor.
/// \param type Log item type.
/// \param size Requested buffer size in bytes (if 0, calls mlog_close() and returns NULL).
/// \return Buffer, NULL if log mode MTR_LOG_NONE.
IB_INTERN byte* mlog_open_and_write_index(mtr_t *mtr, const byte *rec, dict_index_t *index, byte type, ulint size);

#endif // !UNIV_HOTBACKUP

/// \brief Parses a log record written by mlog_open_and_write_index.
/// \param ptr Buffer.
/// \param end_ptr Buffer end.
/// \param comp TRUE=compact record format.
/// \param index Dummy index.
/// \return Parsed record end, NULL if not a complete record.
IB_INTERN byte* mlog_parse_index(byte *ptr, const byte *end_ptr, ibool comp, dict_index_t **index);

#ifndef UNIV_HOTBACKUP
/* Insert, update, and maybe other functions may use this value to define an
extra mlog buffer size for variable size data */
#define MLOG_BUF_MARGIN 256
#endif /* !UNIV_HOTBACKUP */

#ifndef IB_DO_NOT_INLINE
#include "mtr_log.inl"
#endif

