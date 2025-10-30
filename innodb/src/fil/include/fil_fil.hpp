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

/// \file fil_fil.hpp
/// \brief The low-level file system
/// \details Originally created by Heikki Tuuri on 10/25/1995
/// \author Fabio N. Filasieno
/// \date 18/10/2025

#pragma once

#include "univ.i"
#ifndef IB_HOTBACKUP
#include "sync_rw.hpp"
#endif /* !IB_HOTBACKUP */
#include "dict_types.hpp"
#include "ut_byte.hpp"
#include "os_file.hpp"
#include "srv_srv.hpp"

/* When program is run, the default directory "." is the current datadir,
but in ibbackup we must set it explicitly; the path must NOT contain the
trailing '/' or '\' */
extern const char* fil_path_to_client_datadir;

/** Initial size of a single-table tablespace in pages */
constinit ulint FIL_IBD_FILE_INITIAL_SIZE = 4;

/** 'null' (undefined) page offset in the context of file spaces */
#define FIL_NULL ULINT32_UNDEFINED

/* Space address data type; this is intended to be used when
addresses accurate to a byte are stored in file pages. If the page part
of the address is FIL_NULL, the address is considered undefined. */

typedef byte fil_faddr_t; /*!< 'type' definition in C: an address stored in a file page is a string of bytes */
constinit ulint FIL_ADDR_PAGE = 0; /* first in address is the page offset */
constinit ulint FIL_ADDR_BYTE = 4; /* then comes 2-byte byte offset within page */

constinit ulint FIL_ADDR_SIZE = 6; /* address size is 6 bytes */

/** A struct for storing a space address FIL_ADDR, when it is used
in C program data structures. */

typedef struct fil_addr_struct fil_addr_t;
/** File space address */
struct fil_addr_struct{
    ulint page;        /*!< page number within a space */
    ulint boffset;     /*!< byte offset within the page */
};

/** The null file address */
extern const fil_addr_t fil_addr_null;

/** The byte offsets on a file page for various variables @{ */
constinit ulint FIL_PAGE_SPACE_OR_CHKSUM = 0; /*!< in < MySQL-4.0.14 space id the page belongs to (== 0) but in later versions the 'new' checksum of the page */
constinit ulint FIL_PAGE_OFFSET = 4;          /*!< page offset inside space */
constinit ulint FIL_PAGE_PREV = 8;            /*!< if there is a 'natural' predecessor of the page, its offset. Otherwise FIL_NULL. This field is not set on BLOB pages, which are stored as a singly-linked list. See also FIL_PAGE_NEXT. */
constinit ulint FIL_PAGE_NEXT = 12;           /*!< if there is a 'natural' successor of the page, its offset. Otherwise FIL_NULL. B-tree index pages (FIL_PAGE_TYPE contains FIL_PAGE_INDEX) on the same PAGE_LEVEL are maintained as a doubly linked list via FIL_PAGE_PREV and FIL_PAGE_NEXT in the collation order of the smallest user record on each page. */
constinit ulint FIL_PAGE_LSN = 16;               /*!< lsn of the end of the newest modification log record to the page */
constinit ulint FIL_PAGE_TYPE = 24;              /*!< file page type: FIL_PAGE_INDEX,..., 2 bytes. The contents of this field can only be trusted in the following case: if the page is an uncompressed B-tree index page, then it is guaranteed that the value is FIL_PAGE_INDEX. The opposite does not hold. In tablespaces created by InnoDB 5.1.7 or later, the contents of this field is valid for all uncompressed pages. */
constinit ulint FIL_PAGE_FILE_FLUSH_LSN = 26;    /*!< this is only defined for the first page in a system tablespace data file (ibdata*, not *.ibd): the file has been flushed to disk at least up to this lsn */
constinit ulint FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID = 34; /*!< starting from 4.1.x this contains the space id of the page */
constinit ulint FIL_PAGE_DATA = 38;             /*!< start of the data on the page */
/* @} */
/** File page trailer @{ */
constinit ulint FIL_PAGE_END_LSN_OLD_CHKSUM = 8; /*!< the low 4 bytes of this are used to store the page checksum, the last 4 bytes should be identical to the last 4 bytes of FIL_PAGE_LSN */
constinit ulint FIL_PAGE_DATA_END = 8;            /*!< size of the page trailer */
/* @} */

/** File page types (values of FIL_PAGE_TYPE) @{ */
constinit ulint FIL_PAGE_INDEX = 17855;          /*!< B-tree node */
constinit ulint FIL_PAGE_UNDO_LOG = 2;           /*!< Undo log page */
constinit ulint FIL_PAGE_INODE = 3;              /*!< Index node */
constinit ulint FIL_PAGE_IBUF_FREE_LIST = 4;     /*!< Insert buffer free list */
/* File page types introduced in InnoDB 5.1.7 */
constinit ulint FIL_PAGE_TYPE_ALLOCATED = 0;     /*!< Freshly allocated page */
constinit ulint FIL_PAGE_IBUF_BITMAP = 5;        /*!< Insert buffer bitmap */
constinit ulint FIL_PAGE_TYPE_SYS = 6;           /*!< System page */
constinit ulint FIL_PAGE_TYPE_TRX_SYS = 7;       /*!< Transaction system data */
constinit ulint FIL_PAGE_TYPE_FSP_HDR = 8;       /*!< File space header */
constinit ulint FIL_PAGE_TYPE_XDES = 9;          /*!< Extent descriptor page */
constinit ulint FIL_PAGE_TYPE_BLOB = 10;         /*!< Uncompressed BLOB page */
constinit ulint FIL_PAGE_TYPE_ZBLOB = 11;        /*!< First compressed BLOB page */
constinit ulint FIL_PAGE_TYPE_ZBLOB2 = 12;       /*!< Subsequent compressed BLOB page */
/* @} */

/** Space types @{ */
constinit ulint FIL_TABLESPACE = 501;            /*!< tablespace */
constinit ulint FIL_LOG = 502;                   /*!< redo log */
/* @} */

/** The number of fsyncs done to the log */
extern ulint fil_n_log_flushes;

/** Number of pending redo log flushes */
extern ulint fil_n_pending_log_flushes;
/** Number of pending tablespace flushes */
extern ulint fil_n_pending_tablespace_flushes;


#ifndef IB_HOTBACKUP

/// \brief Returns the version number of a tablespace, -1 if not found.
/// \param [in] id space id
/// \return version number, -1 if the tablespace does not exist in the memory cache
IB_INTERN ib_int64_t fil_space_get_version(ulint id);

/// \brief Returns the latch of a file space.
/// \param [in] id space id
/// \param [out] zip_size compressed page size, or 0 for uncompressed tablespaces
/// \return latch protecting storage allocation
IB_INTERN rw_lock_t* fil_space_get_latch(ulint id, ulint* zip_size);

/// \brief Returns the type of a file space.
/// \param [in] id space id
/// \return FIL_TABLESPACE or FIL_LOG
IB_INTERN ulint fil_space_get_type(ulint id);
#endif /* !IB_HOTBACKUP */

/// \brief Appends a new file to the chain of files of a space. File must be closed.
/// \param [in] name file name (file must be closed)
/// \param [in] size file size in database blocks, rounded downwards to an integer
/// \param [in] id space id where to append
/// \param [in] is_raw TRUE if a raw device or a raw disk partition
IB_INTERN void fil_node_create(const char* name, ulint size, ulint id, ibool is_raw);
#ifdef IB_LOG_ARCHIVE

/// \brief Drops files from the start of a file space, so that its size is cut by the amount given.
/// \param [in] id space id
/// \param [in] trunc_len truncate by this much; it is an error if this does not equal to the combined size of some initial files in the space
IB_INTERN void fil_space_truncate_start(ulint id, ulint trunc_len);
#endif /* IB_LOG_ARCHIVE */

/// \brief Creates a space memory object and puts it to the 'fil system' hash table.
/// \param [in] name space name
/// \param [in] id space id
/// \param [in] zip_size compressed page size, or 0 for uncompressed tablespaces
/// \param [in] purpose FIL_TABLESPACE, or FIL_LOG if log
/// \return TRUE if success
/// \details If there is an error, prints an error message to the .err log.
IB_INTERN ibool fil_space_create(const char* name, ulint id, ulint zip_size, ulint purpose);

/// \brief Returns the size of the space in pages.
/// \param [in] id space id
/// \return space size, 0 if space not found
/// \details The tablespace must be cached in the memory cache.
IB_INTERN ulint fil_space_get_size(ulint id);

/// \brief Returns the flags of the space.
/// \param [in] id space id
/// \return flags, ULINT_UNDEFINED if space not found
/// \details The tablespace must be cached in the memory cache.
IB_INTERN ulint fil_space_get_flags(ulint id);

/// \brief Returns the compressed page size of the space, or 0 if the space is not compressed.
/// \param [in] id space id
/// \return compressed page size, ULINT_UNDEFINED if space not found
/// \details The tablespace must be cached in the memory cache.
IB_INTERN ulint fil_space_get_zip_size(ulint id);

/// \brief Checks if the pair space, page_no refers to an existing page in a tablespace file space.
/// \param [in] id space id
/// \param [in] page_no page number
/// \return TRUE if the address is meaningful
/// \details The tablespace must be cached in the memory cache.
IB_INTERN ibool fil_check_adress_in_tablespace(ulint id, ulint page_no);

/// \brief Initializes the tablespace memory cache.
/// \param [in] hash_size hash table size
/// \param [in] max_n_open max number of open files
IB_INTERN void fil_init(ulint hash_size, ulint max_n_open);

/// \brief Deinitializes the tablespace memory cache.
IB_INTERN void fil_close(void);

/// \brief Opens all log files and system tablespace data files.
/// \details They stay open until the database server shutdown. This should be called at a server startup after the space objects for the log and the system tablespace have been created. The purpose of this operation is to make sure we never run out of file descriptors if we need to read from the insert buffer or to write to the log.
IB_INTERN void fil_open_log_and_system_tablespace_files(void);

/// \brief Closes all open files.
/// \details There must not be any pending i/o's or not flushed modifications in the files.
IB_INTERN void fil_close_all_files(void);

/// \brief Sets the max tablespace id counter if the given number is bigger than the previous value.
/// \param [in] max_id maximum known id
IB_INTERN void fil_set_max_space_id_if_bigger(ulint max_id);
#ifndef IB_HOTBACKUP

/// \brief Writes the flushed lsn and the latest archived log number to the page header of the first page of each data file in the system tablespace.
/// \param [in] lsn lsn to write
/// \param [in] arch_log_no latest archived log file number
/// \return DB_SUCCESS or error number
IB_INTERN ulint fil_write_flushed_lsn_to_data_files(ib_uint64_t lsn, ulint arch_log_no);

/// \brief Reads the flushed lsn and arch no fields from a data file at database startup.
/// \param [in] data_file open data file
/// \param [in] one_read_already TRUE if min and max parameters below already contain sensible data
#ifdef IB_LOG_ARCHIVE
/// \param [in,out] min_arch_log_no
/// \param [in,out] max_arch_log_no
#endif /* IB_LOG_ARCHIVE */
/// \param [in,out] min_flushed_lsn
/// \param [in,out] max_flushed_lsn
IB_INTERN void fil_read_flushed_lsn_and_arch_log_no(os_file_t data_file, ibool one_read_already,
#ifdef IB_LOG_ARCHIVE
    ulint* min_arch_log_no, ulint* max_arch_log_no,
#endif /* IB_LOG_ARCHIVE */
    ib_uint64_t* min_flushed_lsn, ib_uint64_t* max_flushed_lsn);

    /// \brief Increments the count of pending insert buffer page merges, if space is not being deleted.
/// \param [in] id space id
/// \return TRUE if being deleted, and ibuf merges should be skipped
/// \details The space is not being deleted.
IB_INTERN ibool fil_inc_pending_ibuf_merges(ulint id);

/// \brief Decrements the count of pending insert buffer page merges.
/// \param [in] id space id
IB_INTERN void fil_decr_pending_ibuf_merges(ulint id);
#endif /* !IB_HOTBACKUP */

/// \brief Parses the body of a log record written about an .ibd file operation.
/// \details That is, the log record part after the standard (type, space id, page no) header of the log record. If desired, also replays the delete or rename operation if the .ibd file exists and the space id in it matches. Replays the create operation if a file at that path does not exist yet. If the database directory for the file to be created does not exist, then we create the directory, too. Note that ibbackup --apply-log sets fil_path_to_client_datadir to point to the datadir that we should use in replaying the file operations.
/// \param [in] ptr buffer containing the log record body, or an initial segment of it, if the record does not fir completely between ptr and end_ptr
/// \param [in] end_ptr buffer end
/// \param [in] type the type of this log record
/// \param [in] space_id the space id of the tablespace in question, or 0 if the log record should only be parsed but not replayed
/// \param [in] log_flags redo log flags (stored in the page number parameter)
/// \return end of log record, or NULL if the record was not completely contained between ptr and end_ptr
IB_INTERN byte* fil_op_log_parse_or_replay(byte* ptr, byte* end_ptr, ulint type, ulint space_id, ulint log_flags);

/// \brief Deletes a single-table tablespace.
/// \param [in] id space id
/// \return TRUE if success
/// \details The tablespace must be cached in the memory cache.
IB_INTERN ibool fil_delete_tablespace(ulint id);
#ifndef IB_HOTBACKUP

/// \brief Discards a single-table tablespace.
/// \param [in] id space id
/// \return TRUE if success
/// \details The tablespace must be cached in the memory cache. Discarding is like deleting a tablespace, but 1) we do not drop the table from the data dictionary; 2) we remove all insert buffer entries for the tablespace immediately; in DROP TABLE they are only removed gradually in the background; 3) when the user does IMPORT TABLESPACE, the tablespace will have the same id as it originally had.
IB_INTERN ibool fil_discard_tablespace(ulint id);
#endif /* !IB_HOTBACKUP */

/// \brief Renames a single-table tablespace.
/// \param [in] old_name old table name in the standard databasename/tablename format of InnoDB, or NULL if we do the rename based on the space id only
/// \param [in] id space id
/// \param [in] new_name new table name in the standard databasename/tablename format of InnoDB
/// \return TRUE if success
/// \details The tablespace must be cached in the tablespace memory cache.
IB_INTERN ibool fil_rename_tablespace(const char* old_name, ulint id, const char* new_name);


/// \brief Creates a new single-table tablespace in a database directory.
/// \param [in,out] space_id space id; if this is != 0, then this is an input parameter, otherwise output
/// \param [in] tablename the table name in the usual databasename/tablename format of InnoDB, or a dir path to a temp table
/// \param [in] is_temp TRUE if a table created with CREATE TEMPORARY TABLE
/// \param [in] flags tablespace flags
/// \param [in] size the initial size of the tablespace file in pages, must be >= FIL_IBD_FILE_INITIAL_SIZE
/// \return DB_SUCCESS or error code
/// \details The datadir is the current directory of a running program. We can refer to it by simply the path '.'. Tables created with CREATE TEMPORARY TABLE we place in the configured TEMP dir of the application.
IB_INTERN ulint fil_create_new_single_table_tablespace(ulint* space_id, const char* tablename, ibool is_temp, ulint flags, ulint size);
#ifndef IB_HOTBACKUP

/// \brief Tries to open a single-table tablespace and optionally checks the space id is right in it.
/// \param [in] check_space_id should we check that the space id in the file is right; we assume that this function runs much faster if no check is made, since accessing the file inode probably is much faster (the OS caches them) than accessing the first page of the file
/// \param [in] id space id
/// \param [in] flags tablespace flags
/// \param [in] name table name in the databasename/tablename format
/// \return TRUE if success
/// \details If does not succeed, prints an error message to the .err log. This function is used to open a tablespace when we start up the application, and also in IMPORT TABLESPACE. NOTE that we assume this operation is used either at the database startup or under the protection of the dictionary mutex, so that two users cannot race here. This operation does not leave the file associated with the tablespace open, but closes it after we have looked at the space id in it.
IB_INTERN ibool fil_open_single_table_tablespace(ibool check_space_id, ulint id, ulint flags, const char* name);

/// \brief Resets page lsn's in the file if they have risen above the current system lsn.
/// \param [in] name table name in the databasename/tablename format
/// \param [in] current_lsn reset lsn's if the lsn stamped to FIL_PAGE_FILE_FLUSH_LSN in the first page is too high
/// \return TRUE if success
/// \details It is possible, though very improbable, that the lsn's in the tablespace to be imported have risen above the current system lsn, if a lengthy purge, ibuf merge, or rollback was performed on a backup taken with ibbackup. If that is the case, reset page lsn's in the file. We assume that the engine was shutdown after it performed these cleanup operations on the .ibd file, so that it at the shutdown stamped the latest lsn to the FIL_PAGE_FILE_FLUSH_LSN in the first page of the .ibd file, and we can determine whether we need to reset the lsn's just by looking at that flush lsn.
IB_INTERN ibool fil_reset_too_high_lsns(const char* name, ib_uint64_t current_lsn);
#endif /* !IB_HOTBACKUP */

/// \brief Scans the database directories for .ibd files at server startup if crash recovery is needed.
/// \param [in] recovery recovery flag
/// \return DB_SUCCESS or error number
/// \details At the server startup, if we need crash recovery, scans the database directories under the current dir, looking for .ibd files. Those files are single-table tablespaces. We need to know the space id in each of them so that we know into which file we should look to check the contents of a page stored in the doublewrite buffer, also to know where to apply log records where the space id is != 0.
IB_INTERN ulint fil_load_single_table_tablespaces(ib_recovery_t recovery);

/// \brief Prints an error message of orphaned .ibd files if crash recovery is needed.
/// \details If we need crash recovery, and we have called fil_load_single_table_tablespaces() and dict_load_single_table_tablespaces(), we can call this function to print an error message of orphaned .ibd files for which there is not a data dictionary entry with a matching table name and space id.
IB_INTERN void fil_print_orphaned_tablespaces(void);

/// \brief Returns TRUE if a single-table tablespace does not exist in the memory cache, or is being deleted there.
/// \param [in] id space id
/// \param [in] version tablespace_version should be this; if you pass -1 as the value of this, then this parameter is ignored
/// \return TRUE if does not exist or is being deleted
IB_INTERN ibool fil_tablespace_deleted_or_being_deleted_in_mem(ulint id, ib_int64_t version);

/// \brief Returns TRUE if a single-table tablespace exists in the memory cache.
/// \param [in] id space id
/// \return TRUE if exists
IB_INTERN ibool fil_tablespace_exists_in_mem(ulint id);
#ifndef IB_HOTBACKUP

/// \brief Returns TRUE if a matching tablespace exists in the InnoDB tablespace memory cache.
/// \param [in] id space id
/// \param [in] name table name in the standard 'databasename/tablename' format or the dir path to a temp table
/// \param [in] is_temp TRUE if created with CREATE TEMPORARY TABLE
/// \param [in] mark_space in crash recovery, at database startup we mark all spaces which have an associated table in the InnoDB data dictionary, so that we can print a warning about orphaned tablespaces
/// \param [in] print_error_if_does_not_exist print detailed error information to the .err log if a matching tablespace is not found from memory
/// \return TRUE if a matching tablespace exists in the memory cache
/// \details Note that if we have not done a crash recovery at the database startup, there may be many tablespaces which are not yet in the memory cache.
IB_INTERN ibool fil_space_for_table_exists_in_mem(ulint id, const char* name, ibool is_temp, ibool mark_space, ibool print_error_if_does_not_exist);
#else /* !IB_HOTBACKUP */

/// \brief Extends all tablespaces to the size stored in the space header.
/// \details During the ibbackup --apply-log phase we extended the spaces on-demand so that log records could be appllied, but that may have left spaces still too small compared to the size stored in the space header.
IB_INTERN void fil_extend_tablespaces_to_stored_len(void);
#endif /* !IB_HOTBACKUP */

/// \brief Tries to extend a data file so that it would accommodate the number of pages given.
/// \param [out] actual_size size of the space after extension; if we ran out of disk space this may be lower than the desired size
/// \param [in] space_id space id
/// \param [in] size_after_extend desired size in pages after the extension; if the current space size is bigger than this already, the function does nothing
/// \return TRUE if success
/// \details The tablespace must be cached in the memory cache. If the space is big enough already, does nothing.
IB_INTERN ibool fil_extend_space_to_desired_size(ulint* actual_size, ulint space_id, ulint size_after_extend);

/// \brief Tries to reserve free extents in a file space.
/// \param [in] id space id
/// \param [in] n_free_now number of free extents now
/// \param [in] n_to_reserve how many one wants to reserve
/// \return TRUE if succeed
IB_INTERN ibool fil_space_reserve_free_extents(ulint id, ulint n_free_now, ulint n_to_reserve);

/// \brief Releases free extents in a file space.
/// \param [in] id space id
/// \param [in] n_reserved how many one reserved
IB_INTERN void fil_space_release_free_extents(ulint id, ulint n_reserved);

/// \brief Gets the number of reserved extents.
/// \param [in] id space id
/// \details If the database is silent, this number should be zero.
IB_INTERN ulint fil_space_get_n_reserved_extents(ulint id);

/// \brief Reads or writes data. This operation is asynchronous (aio).
/// \param [in] type OS_FILE_READ or OS_FILE_WRITE, ORed to OS_FILE_LOG, if a log i/o and ORed to OS_AIO_SIMULATED_WAKE_LATER if simulated aio and we want to post a batch of i/os; NOTE that a simulated batch may introduce hidden chances of deadlocks, because i/os are not actually handled until all have been posted: use with great caution!
/// \param [in] sync TRUE if synchronous aio is desired
/// \param [in] space_id space id
/// \param [in] zip_size compressed page size in bytes; 0 for uncompressed pages
/// \param [in] block_offset offset in number of blocks
/// \param [in] byte_offset remainder of offset in bytes; in aio this must be divisible by the OS block size
/// \param [in] len how many bytes to read or write; this must not cross a file boundary; in aio this must be a block size multiple
/// \param [in,out] buf buffer where to store read data or from where to write; in aio this must be appropriately aligned
/// \param [in] message message for aio handler if non-sync aio used, else ignored
/// \return DB_SUCCESS, or DB_TABLESPACE_DELETED if we are trying to do i/o on a tablespace which does not exist
IB_INTERN ulint fil_io(ulint type, ibool sync, ulint space_id, ulint zip_size, ulint block_offset, ulint byte_offset, ulint len, void* buf, void* message);

/// \brief Waits for an aio operation to complete.
/// \param [in] segment the number of the segment in the aio array to wait for
/// \details This function is used to write the handler for completed requests. The aio array of pending requests is divided into segments (see os0file.c for more info). The thread specifies which segment it wants to wait for.
IB_INTERN void fil_aio_wait(ulint segment);

/// \brief Flushes to disk possible writes cached by the OS.
/// \param [in] space_id file space id (this can be a group of log files or a tablespace of the database)
/// \details If the space does not exist or is being dropped, does not do anything.
IB_INTERN void fil_flush(ulint space_id);

/// \brief Flushes to disk writes in file spaces of the given type possibly cached by the OS.
/// \param [in] purpose FIL_TABLESPACE, FIL_LOG
IB_INTERN void fil_flush_file_spaces(ulint purpose);

/// \brief Checks the consistency of the tablespace cache.
/// \return TRUE if ok
IB_INTERN ibool fil_validate(void);

/// \brief Returns TRUE if file address is undefined.
/// \param [in] addr address
/// \return TRUE if undefined
IB_INTERN ibool fil_addr_is_null(fil_addr_t addr);

/// \brief Get the predecessor of a file page.
/// \param [in] page file page
/// \return FIL_PAGE_PREV
IB_INTERN ulint fil_page_get_prev(const byte* page);

/// \brief Get the successor of a file page.
/// \param [in] page file page
/// \return FIL_PAGE_NEXT
IB_INTERN ulint fil_page_get_next(const byte* page);

/// \brief Sets the file page type.
/// \param [in,out] page file page
/// \param [in] type type
IB_INTERN void fil_page_set_type(byte* page, ulint type);

/// \brief Gets the file page type.
/// \param [in] page file page
/// \return type; NOTE that if the type has not been written to page, the return value not defined
IB_INTERN ulint fil_page_get_type(const byte* page);

/// \brief Reset variables.
IB_INTERN void fil_var_init(void);

/// \brief Remove the underlying directory where the database .ibd files are stored.
/// \param [in] dbname database name
/// \return TRUE on success
IB_INTERN ibool fil_rmdir(const char* dbname);

/// \brief Create the underlying directory where the database .ibd files are stored.
/// \param [in] dbname database name
/// \return TRUE on success
IB_INTERN ibool fil_mkdir(const char* dbname);

typedef struct fil_space_struct fil_space_t;
