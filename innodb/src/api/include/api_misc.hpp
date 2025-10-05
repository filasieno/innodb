/******************************************************
Interface between Innobase and client. This file contains
the functions that don't have a proper home yet.

Copyright (c) 2008, 2025, Innobase Oy. All Rights Reserved.
Copyright (c) 2008 Oracle. All Rights Reserved.
*******************************************************/

/// @file api_misc.hpp
/// \brief Interface between Innobase and client
/// \details This file contains the functions that don't have a proper home yet.

#include "univ.i"
#include "os_file.hpp"
#include "que_que.hpp"
#include "trx_trx.hpp"

/// \brief Determines if the currently running transaction has been interrupted.
/// \param trx Transaction.
/// \return TRUE if interrupted.
UNIV_INTERN ibool trx_is_interrupted(const trx_t* trx);

/// \brief Create a temporary file using the OS specific function.
/// \param filename Temp filename prefix.
/// \return File descriptor or -1 on error.
UNIV_INTERN int ib_create_tempfile(const char* filename);
	
/// \brief Handles user errors and lock waits detected by the database engine.
/// \return TRUE if it was a lock wait and we should continue running the query thread.
UNIV_INTERN ibool ib_handle_errors(enum db_err* new_err, trx_t* trx, que_thr_t* thr, trx_savept_t* savept);

/// \brief Sets a lock on a table.
/// \param trx Transaction.
/// \param table Table to lock.
/// \param mode Lock mode.
/// \return Error code or DB_SUCCESS.
UNIV_INTERN enum db_err ib_trx_lock_table_with_retry(trx_t* trx, dict_table_t* table, enum lock_mode mode);

/// \brief Updates the table modification counter and calculates new estimates
/// for table and index statistics if necessary.
/// \param table Table.
UNIV_INTERN void ib_update_statistics_if_needed(dict_table_t* table);
