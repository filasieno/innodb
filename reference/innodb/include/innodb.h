/// \file innodb.hpp
/// \brief InnoDB API header file

#include "innodb_types.h"

#ifndef INNODB_HPP
#define INNODB_HPP

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if defined(IB_BUILDING_INNODB)
# if defined(IB_HAVE_VISIBILITY)
#  define INNODB_API __attribute__ ((visibility("default")))
#  define INNODB_LOCAL  __attribute__ ((visibility("hidden")))
# elif defined (__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#  define INNODB_API __global
#  define INNODB_LOCAL __hidden
# elif defined(_MSC_VER)
#  define INNODB_API extern __declspec(dllexport)
#  define INNODB_LOCAL
# endif // defined(IB_HAVE_VISIBILITY)
#else // defined(BUILDING_LIBDRIZZLE)
# if defined(_MSC_VER)
#  define INNODB_API extern __declspec(dllimport)
#  define INNODB_LOCAL
# else
#  define INNODB_API
#  define INNODB_LOCAL
# endif // defined(_MSC_VER)
#endif // defined(IB_BUILDING_INNODB)

/// \brief Return the API version number, the version number format is: 16 bits future use | 16 bits current | 16 bits revision | 16 bits age
/// - If the library source code has changed at all since the last release, then revision will be incremented (`c:r:a' becomes `c:r+1:a').
/// - If any interfaces have been added, removed, or changed since the last update, current will be incremented, and revision will be set to 0.
/// - If any interfaces have been added (but not changed or removed) since the last release, then age will be incremented.
/// - If any interfaces have been changed or removed since the last release, then age will be set to 0.
/// \ingroup misc
/// \return API version number
INNODB_API ib_u64_t ib_api_version(void) IB_NO_IGNORE;

/// \brief Initialize the InnoDB engine. This must be called prior to calling
/// any other InnoDB API function. You can call only the ib_cfg_*() functions
/// between calls to ib_init() and ib_startup(). No other INNODB
/// functions should be called.
/// \ingroup init
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_init(void) IB_NO_IGNORE;

/// \brief Startup the InnoDB engine. If this function is called on a non-existent database then based on the default or user specified configuration settings it will create all the necessary files. If the database was shutdown cleanly but the user deleted the REDO log files then it will recreate the REDO log files.
/// \ingroup init
/// \param format is the max file format name that the engine supports. Currently this is either Antelope or Barracuda although more may be added in the future without API changes.
/// \return DB_SUCCESS or error code
/// \see DB_SUCCESS
INNODB_API ib_err_t ib_startup(const char* format) IB_NO_IGNORE;

/// \brief Shutdown the InnoDB engine.
/// \details Call this function when they are no active transactions. It will close all files and release all memory
/// on successful completion. All internal variables will be reset to their default values.
/// \ingroup init
/// \param flag is the shutdown flag
/// \return	DB_SUCCESS or error code
INNODB_API ib_err_t ib_shutdown(ib_shutdown_t flag) IB_NO_IGNORE;

/// \brief Start a transaction that's been rolled back.
/// \details This special function exists for the case when InnoDB's deadlock detector has rolledack a transaction.
/// While the transaction has been rolled back the handle is still valid and can be reused by calling this function.
//  If you don't want to reuse the transaction handle then you can free the handle by calling ib_trx_release().
/// \ingroup trx
/// \param trx is the transaction to restart
/// \param ib_trx_level is the transaction isolation level
/// \return	innobase txn handle
INNODB_API ib_err_t ib_trx_start(ib_trx_t trx, ib_trx_level_t ib_trx_level) IB_NO_IGNORE;

/// \brief Begin a transaction. This will allocate a new transaction handle and put the transaction in the active state.
/// \ingroup trx
/// \param ib_trx_level is the transaction isolation level
/// \return	innobase txn handle
INNODB_API ib_trx_t ib_trx_begin(ib_trx_level_t ib_trx_level) IB_NO_IGNORE;

/// \brief Set client data for a transaction. This is passed back to the client in the trx_is_interrupted callback.
/// INNODB will only ever pass this around, it will never dereference it.
/// \ingroup trx
/// \param trx is the transaction to set the client data for
/// \param client_data is client program's data about this transaction
INNODB_API void ib_trx_set_client_data(ib_trx_t trx, void* client_data);

/// \brief Query the transaction's state.
/// \details This function can be used to check for the state of the transaction in case it has been rolled back by the InnoDB deadlock detector.
//  Note that when a transaction is selected as a victim for rollback, InnoDB will always return an appropriate error
/// code indicating this. @see DB_DEADLOCK, @see DB_LOCK_TABLE_FULL and
/// \see DB_LOCK_WAIT_TIMEOUT
/// \ingroup trx
/// \param trx is the transaction handle
/// \return	transaction state
INNODB_API ib_trx_state_t ib_trx_state(ib_trx_t	trx) IB_NO_IGNORE;

/// \brief Release the resources of the transaction.
/// If the transaction was selected as a victim by InnoDB and rolled back then use this function to free the transaction handle.
/// \ingroup trx
/// \param trx is the transaction handle
/// return	DB_SUCCESS or err code
INNODB_API ib_err_t ib_trx_release(ib_trx_t	trx) IB_NO_IGNORE;

/// \brief Commit a transaction.
/// This function will release the schema latches too. It will also free the transaction handle.
/// \ingroup trx
/// \param trx is thr transaction handle
/// \return	DB_SUCCESS or err code
INNODB_API ib_err_t ib_trx_commit(ib_trx_t trx) IB_NO_IGNORE;

/// \brief Rollback a transaction.
/// \details This function will release the schema latches too.
/// It will also free the transaction handle.
/// \ingroup trx
/// \param trx is the transaction handle
/// \return	DB_SUCCESS or err code
INNODB_API ib_err_t ib_trx_rollback(ib_trx_t trx) IB_NO_IGNORE;

/// \brief Add columns to a table schema
/// Tables are created in InnoDB by first creating a table schema which is identified by a handle. Then you add the column definitions to the table schema.
/// \ingroup ddl
/// \param tbl_sch is the table schema instance
/// \param name is the name of the column to add
/// \param col_type is the type of the column
/// \param col_attr are the attributes of the column, including constraints
/// \param client_type is any 16 bit number relevant only to the client
/// \param len is the maximum length of the column
/// \return	DB_SUCCESS or err code
INNODB_API ib_err_t ib_table_schema_add_col(ib_tbl_sch_t tbl_sch, const char* name, ib_col_type_t col_type, ib_col_attr_t col_attr, ib_u16_t client_type, ib_ulint_t len) IB_NO_IGNORE;

/// \brief Create and add an index key definition to a table schema.
/// \details The index schema is owned by the table schema instance and will be freed when the table schema instance is freed.
/// \ingroup ddl
/// \param[in,out] tbl_sch is the schema instance
/// \param name name of the key definition to create
/// \param[out] idx_sch is the key definition schema instance
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_table_schema_add_index(ib_tbl_sch_t tbl_sch, const char* name, ib_idx_sch_t* idx_sch) IB_NO_IGNORE;

/// \brief Destroy a schema. The handle is freed by this function.
/// \ingroup ddl
/// \param tbl_sch is the table schema to delete
INNODB_API void ib_table_schema_delete(ib_tbl_sch_t tbl_sch);

/// \brief Create a table schema.
/// \ingroup ddl
/// \param name is the table name for which to create the schema
/// \param[out] tbl_sch is the schema instance that is created
/// \param tbl_fmt is the format of the table to be created
/// \param page_size is the page size for the table or 0 for default
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_table_schema_create(const char* name, ib_tbl_sch_t* tbl_sch, ib_tbl_fmt_t tbl_fmt, ib_ulint_t page_size) IB_NO_IGNORE;

/// \brief Add columns to an index schema definition.
/// \ingroup ddl
/// \param[in,out] idx_sch is the index schema instance
/// \param name is the name of the column to add to the index schema
/// \param prefix_len is the prefix length of the index or 0 if no prefix
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_index_schema_add_col(ib_idx_sch_t idx_sch, const char* name, ib_ulint_t prefix_len) IB_NO_IGNORE;

/// \brief Create an index schema instance.
/// \ingroup ddl
/// \param usr_trx is the current user transaction
/// \param name is the name of the index to create
/// \param table_name is the name of the table the index belongs to
/// \param[out] idx_sch is the newly created index schema instance
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_index_schema_create(ib_trx_t usr_trx, const char* name, const char* table_name, ib_idx_sch_t* idx_sch) IB_NO_IGNORE;

/// \brief Set index as clustered index. Implies UNIQUE.
/// \ingroup ddl
/// \param[in,out] idx_sch is the index schema to update
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_index_schema_set_clustered(ib_idx_sch_t idx_sch) IB_NO_IGNORE;

/// \brief Set to true if it's a simple select.
/// \ingroup sql
/// \param[in, out] crsr is the cursor to update
INNODB_API void ib_cursor_set_simple_select(ib_crsr_t crsr);

/// \brief Set index as a unique index.
/// \ingroup ddl
/// \param[in,out] idx_sch is the index schema to update
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_index_schema_set_unique(ib_idx_sch_t idx_sch) IB_NO_IGNORE;

/// \brief Destroy an index schema.
/// \ingroup ddl
/// \param idx_sch is the index schema to delete
INNODB_API void ib_index_schema_delete(ib_idx_sch_t idx_sch);

/// \brief Create a table in the InnoDB data dictionary using the schema definition.
/// \details If the table exists in the database then this function will return DB_TABLE_IS_BEING_USED and id will contain that table's id.
/// \ingroup ddl
/// \param[in,out] trx the current user transaction
/// \param         sch the the schema for the table to create
/// \param[out]    id table id that was created
/// \return	DB_SUCCESS or err code
INNODB_API ib_err_t ib_table_create(ib_trx_t trx, const ib_tbl_sch_t sch, ib_id_t* id) IB_NO_IGNORE;

/// \brief Rename a table.
/// \details Ensure that you have acquired the schema lock in exclusive mode.
/// \ingroup ddl
/// \param[in,out] trx is the current user transaction
/// \param old_name the current name of the table
/// \param new_name the new name for the table
/// \return	DB_SUCCESS or err code
INNODB_API ib_err_t ib_table_rename(ib_trx_t trx, const char* old_name, const char* new_name) IB_NO_IGNORE;

/// \brief Create a secondary index.
/// \details The index id encodes the table id in the high 4 bytes and the index id in the lower 4 bytes.
/// \ingroup ddl
/// \param[in,out] idx_sch the schema for the index
/// \param[out] index_id is the new index id that was created
/// \return	DB_SUCCESS or err code
INNODB_API ib_err_t ib_index_create(ib_idx_sch_t idx_sch, ib_id_t* index_id) IB_NO_IGNORE;

/// \brief Drop a table.
/// \details Ensure that you have acquired the schema lock in exclusive mode.
/// \ingroup ddl
/// \param trx is the covering transaction.
/// \param name is the name of the table to drop
/// \return	DB_SUCCESS or err code
INNODB_API ib_err_t ib_table_drop(ib_trx_t trx, const char*	name) IB_NO_IGNORE;

/// \brief Drop a secondary index.
/// \details Ensure that you have acquired the schema lock in exclusive mode.
/// \ingroup ddl
/// \param trx is the covering transaction.
/// \param index_id is the id of the index to drop
/// \return	DB_SUCCESS or err code

INNODB_API ib_err_t ib_index_drop(ib_trx_t trx, ib_id_t index_id) IB_NO_IGNORE;

/// \brief Open an InnoDB table and return a cursor handle to it.
/// \ingroup cursor
/// \param table_id is the id of the table to open
/// \param trx is the current transaction handle, can be NULL
/// \param[out] crsr is the new cursor
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_open_table_using_id(ib_id_t table_id, ib_trx_t trx, ib_crsr_t* crsr) IB_NO_IGNORE;

/// \brief Open an InnoDB index and return a cursor handle to it.
/// \ingroup cursor
/// \param index_id is the id of the index to open
/// \param trx is the current transaction handle, can be NULL
/// \param[out] crsr is the new cursor
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_open_index_using_id(ib_id_t index_id, ib_trx_t trx, ib_crsr_t* crsr) IB_NO_IGNORE;

/// \brief Open an InnoDB secondary index cursor and return a cursor handle to it.
/// \ingroup cursor
/// \param open_crsr is an open cursor
/// \param index_name is the name of the index
/// \param[out] crsr is the new cursor
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_open_index_using_name(ib_crsr_t open_crsr, const char* index_name, ib_crsr_t* crsr) IB_NO_IGNORE;

/// \brief Open an InnoDB table by name and return a cursor handle to it.
/// \ingroup cursor
/// \param name is the table name to open
/// \param trx is the current transaction, can be NULL
/// \param crsr is the new cursor
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_open_table(const char* name, ib_trx_t trx, ib_crsr_t* crsr) IB_NO_IGNORE;

/// \brief Reset the cursor.
/// \ingroup cursor
/// \param crsr is an open cursor
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_reset(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Close an InnoDB table and free the cursor.
/// \ingroup cursor
/// \param crsr is an open cursor
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_close(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Insert a row to a table.
/// \ingroup dml
/// \param crsr is an open cursor
/// \param tpl is the tuple to insert
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_insert_row(ib_crsr_t crsr, const ib_tpl_t tpl) IB_NO_IGNORE;

/// \brief Update a row in a table.
/// \ingroup dml
/// \param crsr is the cursor instance
/// \param ib_old_tpl is the old tuple in the table
/// \param ib_new_tpl is the new tuple with the updated values
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_update_row(ib_crsr_t crsr, const ib_tpl_t ib_old_tpl, const ib_tpl_t ib_new_tpl) IB_NO_IGNORE;

/// \brief Delete a row in a table.
/// \ingroup dml
/// \param crsr is the cursor instance
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_delete_row(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Read current row.
/// \ingroup dml
/// \param crsr is the cursor instance
/// \param[out] tpl is the tuple to read the column values
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_read_row(ib_crsr_t crsr, ib_tpl_t tpl) IB_NO_IGNORE;

/// \brief Move cursor to the prev user record in the table.
/// \ingroup cursor
/// \param crsr is the cursor instance
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_prev(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Move cursor to the next user record in the table.
/// \ingroup cursor
/// \param crsr is the cursor instance
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_next(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Move cursor to the first record in the table.
/// \ingroup cursor
/// \param crsr is the cursor instance
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_first(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Move cursor to the last record in the table.
/// \ingroup cursor
/// \param crsr is the cursor instance
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_last(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Search for key.
/// \ingroup cursor
/// \param crsr is an open cursor instance
/// \param tpl is a key to search for
/// \param ib_srch_mode is the search mode
/// \param[out] result is -1, 0 or 1 depending on tuple eq or gt than the current row
/// \return DB_SUCCESS or err code
INNODB_API ib_err_t ib_cursor_moveto(ib_crsr_t crsr, ib_tpl_t tpl, ib_srch_mode_t ib_srch_mode, int* result) IB_NO_IGNORE;

/// \brief Attach the cursor to the transaction. The cursor must not already be attached to another transaction.
/// \ingroup cursor
/// \param crsr is the cursor instance
/// \param trx is the transaction to attach to the cursor
INNODB_API void ib_cursor_attach_trx(ib_crsr_t crsr, ib_trx_t trx);

/// \brief Set the client comparison function for BLOBs and client types.
/// \ingroup misc
/// \param client_cmp_func is the index key compare callback function
INNODB_API void ib_set_client_compare(ib_client_cmp_t client_cmp_func);

/// \brief Set the match mode for ib_cursor_move().
/// \ingroup cursor
/// \param crsr is the cursor instance
/// \param match_mode is the match mode to set
INNODB_API void ib_cursor_set_match_mode(ib_crsr_t crsr, ib_match_mode_t match_mode);

/// \brief Set a column of the tuple. Make a copy using the tuple's heap.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param col_no is the column index in the tuple
/// \param src is the data value to set
/// \param len is the data value (src) length in bytes
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_col_set_value(ib_tpl_t tpl, ib_ulint_t col_no, const void* src, ib_ulint_t len) IB_NO_IGNORE;

/// \brief Get the size of the data available in the column the tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \return bytes avail or IB_SQL_NULL
INNODB_API ib_ulint_t ib_col_get_len(ib_tpl_t tpl, ib_ulint_t i) IB_NO_IGNORE;

/// \brief Copy a column value from the tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] dst is where the data will be copied
/// \param len is the maximum number of bytes that can be copied to dst
/// \return bytes copied or IB_SQL_NULL
INNODB_API ib_ulint_t ib_col_copy_value(ib_tpl_t tpl, ib_ulint_t i, void* dst, ib_ulint_t len);

/// \brief Read a signed int 8 bit column from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] ival is the integer value
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_i8(ib_tpl_t tpl, ib_ulint_t i, ib_i8_t* ival) IB_NO_IGNORE;

/// \brief Read an unsigned int 8 bit column from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] ival is the integer value
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_u8(ib_tpl_t tpl, ib_ulint_t i, ib_u8_t* ival) IB_NO_IGNORE;

/// \brief Read a signed int 16 bit column from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] ival is the integer value
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_i16(ib_tpl_t tpl, ib_ulint_t i, ib_i16_t* ival) IB_NO_IGNORE;

/// \brief Read an unsigned int 16 bit column from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] ival is the integer value
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_u16(ib_tpl_t tpl, ib_ulint_t i, ib_u16_t* ival) IB_NO_IGNORE;

/// \brief Read a signed int 32 bit column from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] ival is the integer value
/// \return	DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_i32(ib_tpl_t tpl, ib_ulint_t i, ib_i32_t* ival) IB_NO_IGNORE;

/// \brief Read an unsigned int 32 bit column from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] ival is the integer value
/// \return	DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_u32(ib_tpl_t tpl, ib_ulint_t i, ib_u32_t* ival) IB_NO_IGNORE;

/// \brief Read a signed int 64 bit column from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] ival is the integer value
/// \return	DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_i64(ib_tpl_t tpl, ib_ulint_t i, ib_i64_t* ival) IB_NO_IGNORE;

/// \brief Read an unsigned int 64 bit column from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] ival is the integer value
/// \return	DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_u64(ib_tpl_t tpl, ib_ulint_t i, ib_u64_t* ival) IB_NO_IGNORE;

/// \brief Get a column value pointer from the tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \return	NULL or pointer to buffer
INNODB_API const void* ib_col_get_value(ib_tpl_t tpl, ib_ulint_t i) IB_NO_IGNORE;

/// \brief Get a column type, length and attributes from the tuple.
/// \ingroup dml
/// \param tpl is the tuple instance
/// \param i is the index (ordinal position) of the column within the tuple
/// \param[out] col_meta the column meta data
/// \return	len of column data
INNODB_API ib_ulint_t ib_col_get_meta(ib_tpl_t tpl, ib_ulint_t i, ib_col_meta_t* col_meta);

/// \brief "Clear" or reset an InnoDB tuple.
/// \details We free the heap and recreate the tuple.
/// \ingroup tuple
/// \param tpl is the tuple to be freed
/// \return	new tuple, or NULL
INNODB_API ib_tpl_t ib_tuple_clear(ib_tpl_t	tpl) IB_NO_IGNORE;

/// \brief Create a new cluster key search tuple and copy the contents of the secondary index key tuple columns that refer to the cluster index record to the cluster key.
/// \details It does a deep copy of the column data.
/// \ingroup tuple
/// \param crsr is a cursor opened on a secondary index
/// \param[out] dst is the tuple where the key data will be copied
/// \param src is the source secondary index tuple to copy from
/// \return	DB_SUCCESS or error code
INNODB_API ib_err_t ib_tuple_get_cluster_key(ib_crsr_t crsr, ib_tpl_t* dst, const ib_tpl_t src) IB_NO_IGNORE;

/// \brief Copy the contents of  source tuple to destination tuple.
/// \details The tuples must be of the same type and belong to the same table/index.
/// \ingroup tuple
/// \param dst is the destination tuple
/// \param src is the source tuple
/// \return	DB_SUCCESS or error code
INNODB_API ib_err_t ib_tuple_copy(ib_tpl_t dst, const ib_tpl_t src) IB_NO_IGNORE;

/// \brief Create an InnoDB tuple used for index/table search.
/// \ingroup tuple
/// \param crsr is the cursor instance
/// \return tuple for current index
INNODB_API ib_tpl_t ib_sec_search_tuple_create(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Create an InnoDB tuple used for index/table search.
/// \ingroup tuple
/// \param crsr is the cursor instance
/// \return tuple for current index
INNODB_API ib_tpl_t ib_sec_read_tuple_create(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Create an InnoDB tuple used for table key operations.
/// \ingroup tuple
/// \param crsr is the cursor instance
/// \return tuple for current table
INNODB_API ib_tpl_t ib_clust_search_tuple_create(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Create an InnoDB tuple for table row operations.
/// \ingroup tuple
/// \param crsr is the cursor instance
/// \return Tuple for current table
INNODB_API ib_tpl_t ib_clust_read_tuple_create(ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Return the number of user columns in the tuple definition.
/// \ingroup tuple
/// \param tpl is a tuple
/// \return number of user columns
INNODB_API ib_ulint_t ib_tuple_get_n_user_cols(const ib_tpl_t tpl) IB_NO_IGNORE;

/// \brief Return the number of columns in the tuple definition.
/// \ingroup tuple
/// \param tpl is a tuple
/// \return number of columns
INNODB_API ib_ulint_t ib_tuple_get_n_cols(const ib_tpl_t tpl) IB_NO_IGNORE;

/// \brief Destroy an InnoDB tuple.
/// \ingroup tuple
/// \param tpl is the tuple instance to delete
INNODB_API void ib_tuple_delete(ib_tpl_t tpl);

/// \brief Truncate a table. The cursor handle will be closed and set to NULL on success.
/// \ingroup ddl
/// \param[out] crsr is the cursor for table to truncate
/// \param[out] table_id is the new table id
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_cursor_truncate(ib_crsr_t* crsr, ib_id_t* table_id) IB_NO_IGNORE;

/// \brief Truncate a table.
/// \ingroup ddl
/// \param table_name is the name of the table to truncate
/// \param[out] table_id is the new table id
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_table_truncate(const char* table_name, ib_id_t* table_id) IB_NO_IGNORE;

/// \brief Get a table id.
/// \ingroup ddl
/// \param table_name is the name of the table to lookup
/// \param[out] table_id is the new table id if found
/// \return DB_SUCCESS if found
INNODB_API ib_err_t ib_table_get_id(const char* table_name, ib_id_t* table_id) IB_NO_IGNORE;

/// \brief Get an index id.
/// \ingroup ddl
/// \param table_name is the name of the table that contains the index
/// \param index_name is the name of the index to lookup
/// \param[out] index_id contains the index id if found
/// \return DB_SUCCESS if found
INNODB_API ib_err_t ib_index_get_id(const char* table_name, const char* index_name, ib_id_t* index_id) IB_NO_IGNORE;

/// \brief Create a database if it doesn't exist.
/// \ingroup ddl
/// \param db_name is the name of the database to create
/// \return IB_TRUE on success
INNODB_API ib_bool_t ib_database_create(const char* db_name) IB_NO_IGNORE;

/// \brief Drop a database if it exists. This function will also drop all tables within the database.
/// \ingroup ddl
/// \param db_name is the name of the database to drop
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_database_drop(const char* db_name) IB_NO_IGNORE;

/// \brief Check if cursor is positioned.
/// \ingroup cursor
/// \param crsr is the cursor instance to check
/// \return IB_TRUE if positioned
INNODB_API ib_bool_t ib_cursor_is_positioned(const ib_crsr_t crsr) IB_NO_IGNORE;

/// \brief Latches the data dictionary in shared mode.
/// \ingroup ddl
/// \param trx is the transaction instance
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_schema_lock_shared(ib_trx_t trx) IB_NO_IGNORE;

/// \brief Latches the data dictionary in exclusive mode.
/// \ingroup ddl
/// \param trx is the transaction instance
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_schema_lock_exclusive(ib_trx_t trx) IB_NO_IGNORE;

/// \brief Checks if the data dictionary is latched in exclusive mode by a user transaction.
/// \ingroup ddl
/// \param trx is a transaction instance
/// \return TRUE if exclusive latch
INNODB_API ib_bool_t ib_schema_lock_is_exclusive(const ib_trx_t trx) IB_NO_IGNORE;

/// \brief Checks if the data dictionary is latched in shared mode.
/// \ingroup ddl
/// \param trx is a transaction instance
/// \return TRUE if shared latch
INNODB_API ib_bool_t ib_schema_lock_is_shared(const ib_trx_t trx) IB_NO_IGNORE;

/// \brief Unlocks the data dictionary.
/// \ingroup ddl
/// \param trx is a transaction instance
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_schema_unlock(ib_trx_t trx);

/// \brief Lock an InnoDB cursor/table.
/// \ingroup trx
/// \param crsr is the cursor instance
/// \param mode is the lock mode
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_cursor_lock(ib_crsr_t crsr, ib_lck_mode_t mode) IB_NO_IGNORE;

/// \brief Set the Lock an InnoDB table using the table id.
/// \ingroup trx
/// \param trx is a transaction instance
/// \param table_id is the table to lock
/// \param mode is the lock mode
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_table_lock(ib_trx_t trx, ib_id_t table_id, ib_lck_mode_t mode) IB_NO_IGNORE;

/// \brief Set the Lock mode of the cursor.
/// \ingroup trx
/// \param crsr is the cursor instance for which we want to set the lock mode
/// \param mode is the lock mode
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_cursor_set_lock_mode(ib_crsr_t crsr, ib_lck_mode_t mode) IB_NO_IGNORE;

/// \brief Set need to access clustered index record flag.
/// \ingroup dml
/// \param crsr is the cursor instance for which we want to set the flag
INNODB_API void ib_cursor_set_cluster_access(ib_crsr_t crsr);

/// \brief Read a table's schema using the visitor pattern.
/// \details It will make the following sequence of calls:
/// \verbatim
///      visitor->table()
///      visitor->table_col()
///      for each user column:
///         visitor->index()
///      for each user index
///         visitor->index_col()
///      for each column in user index It will stop if any of the above functions returns a non-zero value.
/// \endverbatim
/// The caller must have an exclusive lock on the InnoDB data dictionary.
/// \ingroup ddl
/// \param trx transaction that owns the schema lock
/// \param name is the table name to read
/// \param visitor visitor functions to invoke on each definition
/// \param arg is the argument passed to the visitor functions.
/// \return DB_SUCCESS or DB_ERROR
INNODB_API ib_err_t ib_table_schema_visit(ib_trx_t trx, const char* name, const ib_schema_visitor_t* visitor, void* arg) IB_NO_IGNORE;

/// \brief List all the tables in the InnoDB's data dictionary. It will abort if visitor returns a non-zero value. It will call the function: visitor.tables(arg, const char* name, int IB_NAME_LEN); The function will abort if visitor.tables() returns non-zero.
/// \ingroup ddl
/// \param trx is the transaction that owns the schema lock
/// \param visitor is the visitor function
/// \param arg argument passed to the visitor function
/// \return DB_SUCCESS if successful
INNODB_API ib_err_t ib_schema_tables_iterate(ib_trx_t trx, ib_schema_visitor_table_all_t visitor, void* arg) IB_NO_IGNORE;

/// \brief Get the type of a configuration variable. Returns DB_SUCCESS if the variable with name "name" was found and "type" was set.
/// \ingroup config
/// \param name is the variable name to look up
/// \param[out] type is the type of the variable name if found
/// \return DB_SUCCESS if successful
INNODB_API ib_err_t ib_cfg_var_get_type(const char* name, ib_cfg_type_t* type) IB_NO_IGNORE;

/// \brief Set a configuration variable. The second argument's type depends on the type of the variable with the given "name". Returns DB_SUCCESS if the variable with name "name" was found and if its value was set.
/// \ingroup config
/// \param name is the config variable name whose value is to be set
/// \return DB_SUCCESS if set
INNODB_API ib_err_t ib_cfg_set(const char* name, ...) IB_NO_IGNORE;

/// \brief Get the value of a configuration variable. The type of the returned value depends on the type of the configuration variable. DB_SUCCESS is returned if the variable with name "name" was found and "value" was set.
/// \ingroup config
/// \param name is the variable name whose value needs to be accessed
/// \param[out] value is the value of the variable if found
/// \return DB_SUCCESS if retrieved successfully
INNODB_API ib_err_t ib_cfg_get(const char* name, void* value) IB_NO_IGNORE;

/// \brief Get a list of the names of all configuration variables. The caller is responsible for free(3)ing the returned array of strings when it is not needed anymore and for not modifying the individual strings.
/// \ingroup config
/// \param[out] names pointer to array of strings
/// \param[out] names_num number of strings returned
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_cfg_get_all(const char*** names, ib_u32_t* names_num) IB_NO_IGNORE;

/// \brief Creates a named savepoint. The transaction must be started. If there is already a savepoint of the same name, this call erases that old savepoint and replaces it with a new. Savepoints are deleted in a transaction commit or rollback.
/// \ingroup trx
/// \param trx is the transaction instance
/// \param name is the savepoint name
/// \param IB_NAME_LEN is the length of name in bytes
INNODB_API void ib_savepoint_take(ib_trx_t trx, const void* name, ib_ulint_t IB_NAME_LEN);

/// \brief Releases only the named savepoint. Savepoints which were set after this savepoint are left as is.
/// \ingroup trx
/// \param trx is the active transaction
/// \param name is the savepoint name
/// \param IB_NAME_LEN is the length of name in bytes
/// \return if no savepoint of the name found then DB_NO_SAVEPOINT, otherwise DB_SUCCESS
INNODB_API ib_err_t ib_savepoint_release(ib_trx_t trx, const void* name, ib_ulint_t IB_NAME_LEN) IB_NO_IGNORE;

/// \brief Rolls back a transaction back to a named savepoint.
/// \details Modifications after the savepoint are undone but InnoDB does NOT release the corresponding locks which are stored in memory.
/// If a lock is 'implicit', that is, a new inserted row holds a lock where the lock information is carried by the trx id stored in the row,
/// these locks are naturally released in the rollback. Savepoints which were set after this savepoint are deleted.
/// If name equals NULL then all the savepoints are rolled back.
///
/// \ingroup trx
/// \param trx is the active transaction
/// \param name is the savepoint name can be NULL
/// \param IB_NAME_LEN is the length of name in bytes
/// \return if no savepoint of the name found then DB_NO_SAVEPOINT, otherwise DB_SUCCESS
INNODB_API ib_err_t ib_savepoint_rollback(ib_trx_t trx, const void* name, ib_ulint_t IB_NAME_LEN) IB_NO_IGNORE;

/// \brief Write an integer value to a column. Integers are stored in big-endian format and will need to be converted from the host format.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_i8(ib_tpl_t tpl, int col_no, ib_i8_t val) IB_NO_IGNORE;

/// \brief Write an integer value to a column. Integers are stored in big-endian format and will need to be converted from the host format.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_i16(ib_tpl_t tpl, int col_no, ib_i16_t val) IB_NO_IGNORE;

/// \brief Write an integer value to a column. Integers are stored in big-endian format and will need to be converted from the host format.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_i32(ib_tpl_t tpl, int col_no, ib_i32_t val) IB_NO_IGNORE;

/// \brief Write an integer value to a column. Integers are stored in big-endian format and will need to be converted from the host format.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_i64(ib_tpl_t tpl, int col_no, ib_i64_t val) IB_NO_IGNORE;

/// \brief Write an integer value to a column. Integers are stored in big-endian format and will need to be converted from the host format.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_u8(ib_tpl_t tpl, int col_no, ib_u8_t val) IB_NO_IGNORE;

/// \brief Write an integer value to a column. Integers are stored in big-endian format and will need to be converted from the host format.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_u16(ib_tpl_t tpl, int col_no, ib_u16_t val) IB_NO_IGNORE;

/// \brief Write an integer value to a column. Integers are stored in big-endian format and will need to be converted from the host format.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_u32(ib_tpl_t tpl, int col_no, ib_u32_t val) IB_NO_IGNORE;

/// \brief Write an integer value to a column. Integers are stored in big-endian format and will need to be converted from the host format.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_u64(ib_tpl_t tpl, int col_no, ib_u64_t val) IB_NO_IGNORE;

/// \brief Inform the cursor that it's the start of an SQL statement.
/// \ingroup cursor
/// \param crsr is the cursor instance
INNODB_API void ib_cursor_stmt_begin(ib_crsr_t crsr);

/// \brief Write a double value to a column.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_double(ib_tpl_t tpl, int col_no, double val) IB_NO_IGNORE;

/// \brief Read a double column value from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple to read from
/// \param col_no is the column number to read
/// \param[out] dval is where the value is copied
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_double(ib_tpl_t tpl, ib_ulint_t col_no, double* dval) IB_NO_IGNORE;

/// \brief Write a float value to a column.
/// \ingroup dml
/// \param[in,out] tpl is the tuple to write to
/// \param col_no is the column number to update
/// \param val is the value to write
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_write_float(ib_tpl_t tpl, int col_no, float val) IB_NO_IGNORE;

/// \brief Read a float value from an InnoDB tuple.
/// \ingroup dml
/// \param tpl is the tuple to read from
/// \param col_no is the column number to read
/// \param[out] fval is where the value is copied
/// \return DB_SUCCESS or error
INNODB_API ib_err_t ib_tuple_read_float(ib_tpl_t tpl, ib_ulint_t col_no, float* fval) IB_NO_IGNORE;

/// \brief Set the message logging function.
/// \ingroup misc
/// \param ib_msg_log is the message logging function
/// \param ib_msg_stream is the message stream, this is the first argument to the loggingfunction
INNODB_API void ib_logger_set(ib_msg_log_t ib_msg_log, ib_msg_stream_t ib_msg_stream);

/// \brief Convert an error number to a human readable text message. The returned string is static and should not be freed or modified.
/// \ingroup misc
/// \param db_errno is the error number
/// \return string, describing the error
INNODB_API const char* ib_strerror(ib_err_t db_errno) IB_NO_IGNORE;

/// \brief Get the value of an INT status variable.
/// \ingroup misc
/// \param name is the status variable name
/// \param[out] dst is where the output value is copied if name is found
/// \return DB_SUCCESS if found and type is INT, DB_DATA_MISMATCH if found but type is not INT, DB_NOT_FOUND otherwise.
INNODB_API ib_err_t ib_status_get_i64(const char* name, ib_i64_t* dst) IB_NO_IGNORE;

/// \brief Get a list of the names of all status variables. The caller is responsible for free(3)ing the returned array of strings when it is not needed anymore and for not modifying the individual strings.
/// \ingroup misc
/// \param[out] names pointer to array of strings
/// \param[out] names_num number of strings returned
/// \return DB_SUCCESS or error code
INNODB_API ib_err_t ib_status_get_all(const char*** names, ib_u32_t* names_num) IB_NO_IGNORE;

/// \brief Type of callback in the event of INNODB panicing. Your callback should
/// call exit() rather soon, as continuing after a panic will lead to errors
/// returned from every API function. We have also not fully tested
/// every possible outcome from not immediately calling exit().
typedef void (*ib_panic_handler_t)(void*, int, char*, ...);

/// \brief Set panic handler.
/// \ingroup config
/// \param handler your panic handler
/// INNODB will "panic" upon finding certain forms of corruption.
/// By setting a panic handler, you can implement your own notification
/// to the end user of this corruption (e.g. popping up a dialog box).
INNODB_API void ib_set_panic_handler(ib_panic_handler_t handler);

/// \brief Callback for checking if a transaction has been interrupted.
/// This callback lets you implement the MySQL KILL command kind of
/// functionality.
/// A transaction may block in the thread it's running in (for example, while
/// acquiring row locks or doing IO) but other threads may do something that
/// causes ib_trx_is_interrupted_handler_t to return true.
typedef int (*ib_trx_is_interrupted_handler_t)(void*);

/// \brief Set trx_is_interrupted_handler.
/// \ingroup config
/// \param handler the trx_is_interrupted callback
/// You may specify a callback that INNODB will check during certain wait
/// situations to see if it should abort the operation or not. This lets
/// you implement MySQL/Drizzle KILL command style functionality.
INNODB_API void ib_set_trx_is_interrupted_handler(ib_trx_is_interrupted_handler_t handler);

/// \brief Get which key caused a duplicate key error.
/// \ingroup trx
/// \param trx Transaction where error occurred
/// \param table_name pointer to be set to table_name. Valid until next ib_ function call. If you would like to keep it, make a copy.
/// \param index_name pointer to be set to the index name. Valid until next ib_ function call. If you would like to keep it, make a copy.
/// In the event of \ref DB_DUPLICATE_KEY error, you can call this function
/// immediately after to get the name of the table and index that caused
/// the error.
INNODB_API ib_err_t ib_get_duplicate_key(ib_trx_t trx, const char **table_name, const char **index_name);

/// \brief Get table statistics.
/// \ingroup misc
/// \param crsr A Cursor that is opened to a table
/// \param table_stats a \ref ib_table_stats_t to be filled out by INNODB
/// \param sizeof_ib_table_stats_t sizeof(ib_table_stats_t). This allows for ABI compatible changes to the size of ib_table_stats_t.
/// \return \ref DB_SUCCESS or error
/// This function will fill out the provided \ref ib_table_stats_t with
/// statistics about the table on the currently opened \ref ib_crsr_t
INNODB_API ib_err_t ib_get_table_statistics(ib_crsr_t crsr, ib_table_stats_t *table_stats, size_t sizeof_ib_table_stats_t);

/// \brief Get statistics on number of different key values per index part
/// \details This function returns the approximate different key values for this index. They are periodically recalculated.
/// \ingroup misc
/// \param crsr A Cursor that is opened to a table
/// \param index_name name of the index
/// \param ncols returns the number of elements in n_diff
/// \param n_diff An array allocated with malloc() (user needs to free()) containing the statistics
/// \return \ref DB_SUCCESS or error. \ref DB_NOT_FOUND if index is not found
INNODB_API ib_err_t ib_get_index_stat_n_diff_key_vals(ib_crsr_t crsr, const char* index_name, ib_u64_t *ncols, ib_i64_t **n_diff);

/// \brief Force an update of table and index statistics
/// \details This function forces an update to the table and index statistics for the table crsr is opened on.
/// \ingroup misc
/// \param crsr A Cursor that is opened to a table
/// \return \ref DB_SUCCESS or error.
INNODB_API ib_err_t ib_update_table_statistics(ib_crsr_t crsr);

/// \brief Inject an error into INNODB
/// \ingroup debug
/// \param err The error inject code to insert.
/// \details This function will simulate an error condition inside INNODB. You should not rely on this function.
/// It is for INNODB test suite use only, parts may only be compiled into debug libraries and this function
/// can quite legitimately just return DB_ERROR and cause Voldemort to pay you a visit.
INNODB_API ib_err_t ib_error_inject(int err);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // INNODB_H