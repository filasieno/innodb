#pragma once

#define INNODB_API

/// \defgroup api XInnoDB API
/// \brief XInnoDB API
/// \details XInnoDB API is the interface for the XInnoDB database storage engine.

/// \defgroup sdk XInnoDB SDK API
/// \brief XInnoDB SDK API
/// \details XInnoDB SDK API

/// \defgroup components Internal components
/// \brief XInnoDB Internal components
/// \details Describe the components that make up the XInnoDB database storage engine.

/// \addtogroup api
/// @{

/// \defgroup prim Primitive types
/// \ingroup api
/// \brief XInnoDB types
/// \details XInnoDB types are the types used in the XInnoDB API.
/// @{
using ib_u64 = unsigned long long;
using ib_i64 = signed long long;
using ib_u32 = unsigned int;
using ib_i32 = signed int;
using ib_u16 = unsigned short;
using ib_i16 = signed short;
using ib_i8 = signed char;
using ib_u8 = unsigned char;
using ib_size_t = ib_u64;
using ib_int = signed long int;
using ib_ulint = unsigned long int;
using ib_bool = bool;
/// @}


/// \addtogroup other
/// @{

struct ib_sga_state;
struct ib_state;

/// \brief InnoDB Shared Global Area state descriptor
struct ib_sga_state_desc {
    ib_u64   frame_count;
    ib_u64   worker_buffer_size;
    ib_u64   context_buffer_size;
    ib_u64   log_buffer_size;
    ib_u64   debug_buffer_size;
    ib_ulint file_table_capacity;
};

/// \brief InnoDB Worker state descriptor
struct ib_state_desc {

};

/// \brief InnoDB main task handle
struct ib_main_task_hdl
{

};

/// \brief InnoDB task handle
template <typename T>
struct ib_task_hdl {

};

/// \brief InnoDB aysnc operation awaitable
template <typename T>
struct ib_async {

};

/// \brief InnoDB API version
struct ib_api_version {
    int major;
    int minor; 
    int revision;
    const char* build;
};

/// \brief InnoDB error codes. 
/// \details Most of the error codes are internal to the engine and will not be seen by user applications. 
/// The partial error codes reflect the sub-state of an operation within InnoDB. 
/// Some of the error codes are deprecated and are no longer used.
/// \author Fabio N. Filasieno
/// \date 2025-11-01
enum class ib_err {
    /// \brief A successult result
	DB_SUCCESS = 10,

	/// \brief This is a generic error code. 
	/// \details It is used to classify error conditions that can't be represented by other codes
	DB_ERROR,

	/// \brief An operation was interrupted by a user.
	DB_INTERRUPTED,

	/// \brief Operation caused an out of memory error. 
	/// \details Within InnoDB core code this is normally a fatal error
	DB_OUT_OF_MEMORY,

	/// \brief The operating system returned an out of file space error when trying to do an IO operation
	DB_OUT_OF_FILE_SPACE,

	/// \brief A lock request by transaction resulted in a lock wait
	/// \details The thread is suspended internally by InnoDB and is put on a lock wait queue.
	DB_LOCK_WAIT,

	/// \brief A lock request by a transaction resulted in a deadlock. 
	/// \details The transaction was rolled back
	DB_DEADLOCK,

	/// \brief Not used
	DB_ROLLBACK,

	/// \brief A record insert or update violates a unique contraint.
	DB_DUPLICATE_KEY,

	/// \brief A query thread should be in state suspended but is trying to acquire a lock. 
	/// \details Currently this is treated as a hard error and a violation of an invariant.
	DB_QUE_THR_SUSPENDED,

	/// \brief Required history data has been deleted due to lack of space in rollback segment
	DB_MISSING_HISTORY,

	/// \brief This error is not used
	DB_CLUSTER_NOT_FOUND = 30,

	/// \brief The table could not be found
	DB_TABLE_NOT_FOUND,

	/// \brief The database has to be stopped and restarted with more file space
	DB_MUST_GET_MORE_FILE_SPACE,

	/// \brief The user is trying to create a table in the InnoDB data dictionary but a table with that name already exists
	DB_TABLE_IS_BEING_USED,

	/// \brief A record in an index would not fit on a compressed page, or it woul become bigger than 1/2 free space in an uncompressed page frame
	DB_TOO_BIG_RECORD,

	/// \brief Lock wait lasted too long
	DB_LOCK_WAIT_TIMEOUT,

	/// \brief Referenced key value not found for a foreign key in an insert or update of a row
	DB_NO_REFERENCED_ROW,

	/// \brief Cannot delete or update a row because it contains a key value which is referenced
	DB_ROW_IS_REFERENCED,

	/// \brief Adding a foreign key constraint to a table failed
	DB_CANNOT_ADD_CONSTRAINT,

	/// \brief Data structure corruption noticed
	DB_CORRUPTION,

	/// \brief InnoDB cannot handle an index where same column appears twice
	DB_COL_APPEARS_TWICE_IN_INDEX,

	/// \brief Dropping a foreign key constraint from a table failed
	DB_CANNOT_DROP_CONSTRAINT,

	/// \brief No savepoint exists with the given name
	DB_NO_SAVEPOINT,

	/// \brief We cannot create a new single-table tablespace because a file of the same name already exists
	DB_TABLESPACE_ALREADY_EXISTS,

	/// \brief Tablespace does not exist or is being dropped right now
	DB_TABLESPACE_DELETED,

	/// \brief Lock structs have exhausted the buffer pool (for big transactions, InnoDB stores the lock structs in the buffer pool)
	DB_LOCK_TABLE_FULL,

	/// \brief Foreign key constraints activated but the operation would lead to a duplicate key in some table
	DB_FOREIGN_DUPLICATE_KEY,

	/// \brief When InnoDB runs out of the preconfigured undo slots, this can only happen when there are too many concurrent transactions
	DB_TOO_MANY_CONCURRENT_TRXS,

	/// \brief When InnoDB sees any artefact or a feature that it can't recoginize or work with e.g., FT indexes created by a later version of the engine.
	DB_UNSUPPORTED,

	/// \brief A column in the PRIMARY KEY was found to be NULL
	DB_PRIMARY_KEY_IS_NULL,

	/// \brief The application should clean up and quite ASAP. 
	/// \details Fatal error, InnoDB cannot continue operation without risking database corruption.
	DB_FATAL,

	// The following are partial failure codes
	// -----------------------------------------------

	/// \brief Partial failure code.
	DB_FAIL = 1000,

	/// \brief If an update or insert of a record doesn't fit in a Btree page
	DB_OVERFLOW,

	/// \brief If an update or delete of a record causes a Btree page to be below a minimum threshold
	DB_UNDERFLOW,

	/// \brief Failure to insert a secondary index entry to the insert buffer
	DB_STRONG_FAIL,

	/// \brief Failure trying to compress a page
	DB_ZIP_OVERFLOW,
	
	// ------------------------------------------------------

	/// \brief Record not found
	DB_RECORD_NOT_FOUND = 1500,

	/// \brief A cursor operation or search operation scanned to the end of the index.
	DB_END_OF_INDEX,

    // api_only_error_codes API only error codes
    // Error codes that are only used by the API and not by the engine itself.
    
	/// \brief Generic schema error
	DB_SCHEMA_ERROR = 2000,

	/// \brief Column update or read failed because the types mismatch
	DB_DATA_MISMATCH,

	/// \brief If an API function expects the schema to be locked in exclusive mod and if it's not then that API function will return this error code
	DB_SCHEMA_NOT_LOCKED,

	/// \brief Generic error code for "Not found" type of errors
	DB_NOT_FOUND,

	/// \brief Generic error code for "Readonly" type of errors
	DB_READONLY,

	/// \brief Generic error code for "Invalid input" type of errors
	DB_INVALID_INPUT
};


extern int ib_version;


using ib_worker_main_fn = ib_main_task_hdl(ib_state* state);


struct ib_cfg_type;
struct ib_client_cmp;
struct ib_col_attr;
struct ib_col_meta;
struct ib_col_type;

struct ib_id;


struct ib_lck_mode;
struct ib_match_mode;
struct ib_msg_log;
struct ib_msg_stream;
struct ib_panic_handler;
struct ib_schema_visitor;
struct ib_schema_visitor_table_all;
struct ib_shutdown;
struct ib_srch_mode;
struct ib_table_stats;
struct ib_tbl_fmt;
struct ib_tbl_sch;
struct ib_crsr;
struct ib_trx;
struct ib_trx_is_interrupted_handler;
struct ib_trx_level;
struct ib_trx_state;


/// \brief Tuple
/// \ingroup tuple
struct ib_tpl {};

INNODB_API ib_api_version           ib_get_api_version() noexcept;

INNODB_API ib_u64                   ib_get_sga_state_required_size(ib_sga_state_desc* desc) noexcept;
INNODB_API ib_u64                   ib_get_state_required_size(ib_state_desc* desc) noexcept;

INNODB_API ib_sga_state*            ib_sga_init(void* buffer, ib_u64 buffer_size, ib_sga_state_desc* desc, ib_err* out_err) noexcept;
INNODB_API void                     ib_sga_fini(ib_sga_state* state) noexcept;

INNODB_API int                      ib_run_worker(ib_worker_main_fn* worker_fn, ib_sga_state* sga) noexcept;

INNODB_API ib_async<ib_bool>        ib_database_create(ib_state* db, const char* db_name) noexcept;
INNODB_API ib_async<ib_err>         ib_database_drop(ib_state* db, const char* db_name) noexcept;
INNODB_API ib_async<ib_err>         ib_shutdown(ib_state* db, ib_shutdown flag) noexcept;
INNODB_API ib_async<ib_err>         ib_startup(ib_state* db, const char* format) noexcept;

INNODB_API ib_async<ib_err>         ib_cfg_get(ib_state* db, const char* name, void* value) noexcept;
INNODB_API ib_async<ib_err>         ib_cfg_get_all(ib_state* db, const char*** names, ib_u32* names_num) noexcept;
INNODB_API ib_async<ib_err>         ib_cfg_set(ib_state* db, const char* name, ...) noexcept;
INNODB_API ib_async<ib_err>         ib_cfg_var_get_type(ib_state* db, const char* name, ib_cfg_type* type) noexcept;

INNODB_API ib_async<ib_tpl>         ib_clust_read_tuple_create(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_tpl>         ib_clust_search_tuple_create(ib_state* db, ib_crsr crsr) noexcept;

INNODB_API ib_async<ib_ulint>       ib_col_copy_value(ib_state* db, ib_tpl tpl, ib_ulint i, void* dst, ib_ulint len) noexcept;
INNODB_API ib_async<ib_ulint>       ib_col_get_len(ib_state* db, ib_tpl tpl, ib_ulint i) noexcept;
INNODB_API ib_async<ib_ulint>       ib_col_get_meta(ib_state* db, ib_tpl tpl, ib_ulint i, ib_col_meta* col_meta) noexcept;
INNODB_API ib_async<const void*>    ib_col_get_value(ib_state* db, ib_tpl tpl, ib_ulint i) noexcept;
INNODB_API ib_async<ib_err>         ib_col_set_value(ib_state* db, ib_tpl tpl, ib_ulint col_no, const void* src, ib_ulint len) noexcept;
/// @}

/// \defgroup cursor Cursor operations
/// \ingroup api
/// \brief Cursor operations
/// \details Cursor operations are used to navigate through the database.
/// @{
struct ib_crsr;

INNODB_API ib_async<ib_err>         ib_cursor_open_table(ib_state* db, const char* name, ib_trx trx, ib_crsr* out_cur) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_open_table_using_id(ib_state* db, ib_id table_id, ib_trx trx, ib_crsr* out_crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_open_index_using_name(ib_state* db, ib_crsr open_crsr, const char* index_name, ib_crsr* out_crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_open_index_using_id(ib_state* db, ib_id index_id, ib_trx trx, ib_crsr* out_crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_reset(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_close(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<void>           ib_cursor_attach_trx(ib_state* db, ib_crsr crsr, ib_trx trx) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_insert_row(ib_state* db, ib_crsr crsr, const ib_tpl tpl) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_delete_row(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_read_row(ib_state* db, ib_crsr crsr, ib_tpl tpl) noexcept;
INNODB_API ib_async<ib_bool>        ib_cursor_is_positioned(ib_state* db, const ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_first(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_last(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_next(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_prev(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_lock(ib_state* db, ib_crsr crsr, ib_lck_mode mode) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_moveto(ib_state* db, ib_crsr crsr, ib_tpl tpl, ib_srch_mode ib_srch_mode, int* result) noexcept;
INNODB_API ib_async<void>           ib_cursor_set_cluster_access(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<void>           ib_cursor_set_lock_mode(ib_state* db, ib_crsr crsr, ib_lck_mode mode) noexcept;
INNODB_API ib_async<void>           ib_cursor_set_match_mode(ib_state* db, ib_crsr crsr, ib_match_mode match_mode) noexcept;
INNODB_API ib_async<void>           ib_cursor_set_simple_select(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<void>           ib_cursor_stmt_begin(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_truncate(ib_state* db, ib_crsr* crsr, ib_id* table_id) noexcept;
INNODB_API ib_async<ib_err>         ib_cursor_update_row(ib_state* db, ib_crsr crsr, const ib_tpl ib_old_tpl, const ib_tpl ib_new_tpl) noexcept;
INNODB_API ib_async<ib_err>         ib_get_table_statistics(ib_state* db, ib_crsr crsr, ib_table_stats *table_stats, ib_size_t sizeof_ib_table_stats_t) noexcept;
INNODB_API ib_async<ib_err>         ib_get_index_stat_n_diff_key_vals(ib_state* db, ib_crsr crsr, const char* index_name, ib_u64 *ncols, ib_i64 **n_diff) noexcept;
/// @}



/// \defgroup index Index operations
/// \ingroup api
/// \brief Index operations
/// \details Index operations are used to create, drop, and manage indexes.
/// @{

/// \brief Index schema
/// \details Index schema is used to create, drop, and manage indexes.
/// \ingroup index
struct ib_idx_sch {};

INNODB_API ib_async<ib_err>         ib_index_create(ib_state* db, ib_idx_sch idx_sch, ib_id* index_id) noexcept;
INNODB_API ib_async<ib_err>         ib_index_drop(ib_state* db, ib_trx trx, ib_id index_id) noexcept;
INNODB_API ib_async<ib_err>         ib_index_get_id(ib_state* db, const char* table_name, const char* index_name, ib_id* index_id) noexcept;
INNODB_API ib_async<ib_err>         ib_index_schema_add_col(ib_state* db, ib_idx_sch idx_sch, const char* name, ib_ulint prefix_len) noexcept;
INNODB_API ib_async<ib_err>         ib_index_schema_create(ib_state* db, ib_trx usr_trx, const char* name, const char* table_name, ib_idx_sch* idx_sch) noexcept;
INNODB_API ib_async<void>           ib_index_schema_delete(ib_state* db, ib_idx_sch idx_sch) noexcept;
INNODB_API ib_async<ib_err>         ib_index_schema_set_clustered(ib_state* db, ib_idx_sch idx_sch) noexcept;
INNODB_API ib_async<ib_err>         ib_index_schema_set_unique(ib_state* db, ib_idx_sch idx_sch) noexcept;
/// @}



/// \defgroup table Table operations
/// \ingroup api
/// \brief Table operations
/// \details Table operations are used to create, rename, drop, and manage tables.
/// @{
INNODB_API ib_async<ib_err>         ib_table_schema_create(ib_state* db, const char* name, ib_tbl_sch* tbl_sch, ib_tbl_fmt tbl_fmt, ib_ulint page_size) noexcept;
INNODB_API ib_async<ib_err>         ib_table_schema_add_col(ib_state* db, ib_tbl_sch tbl_sch, const char* name, ib_col_type col_type, ib_col_attr col_attr, ib_u16 client_type, ib_ulint len) noexcept;
INNODB_API ib_async<ib_err>         ib_table_schema_add_index(ib_state* db, ib_tbl_sch tbl_sch, const char* name, ib_idx_sch* idx_sch) noexcept;
INNODB_API ib_async<void>           ib_table_schema_delete(ib_state* db, ib_tbl_sch tbl_sch) noexcept;
INNODB_API ib_async<ib_err>         ib_table_schema_visit(ib_state* db, ib_trx trx, const char* name, const ib_schema_visitor* visitor, void* arg) noexcept;

INNODB_API ib_async<ib_err>         ib_table_create(ib_state* db, ib_trx trx, const ib_tbl_sch sch, ib_id* id) noexcept;
INNODB_API ib_async<ib_err>         ib_table_rename(ib_state* db, ib_trx trx, const char* old_name, const char* new_name) noexcept;
INNODB_API ib_async<ib_err>         ib_table_drop(ib_state* db, ib_trx trx, const char* name) noexcept;
INNODB_API ib_async<ib_err>         ib_table_get_id(ib_state* db, const char* table_name, ib_id* table_id) noexcept;
INNODB_API ib_async<ib_err>         ib_table_lock(ib_state* db, ib_trx trx, ib_id table_id, ib_lck_mode mode) noexcept;
INNODB_API ib_async<ib_err>         ib_table_truncate(ib_state* db, const char* table_name, ib_id* table_id) noexcept;


/// @}


/// \defgroup transaction Transaction operations
/// \ingroup api
/// \brief Transaction operations
/// \details Transaction operations are used to create, commit, and rollback transactions.
/// @{
INNODB_API ib_async<ib_err>         ib_trx_begin(ib_state* db, ib_trx_level ib_trx_level) noexcept;
INNODB_API ib_async<ib_err>         ib_trx_commit(ib_state* db, ib_trx trx) noexcept;
INNODB_API ib_async<ib_err>         ib_trx_rollback(ib_state* db, ib_trx trx) noexcept;
INNODB_API ib_async<void>           ib_trx_set_client_data(ib_state* db, ib_trx trx, void* client_data) noexcept;
INNODB_API ib_async<ib_err>         ib_trx_release(ib_state* db, ib_trx trx) noexcept;
INNODB_API ib_async<ib_err>         ib_trx_start(ib_state* db, ib_trx trx, ib_trx_level ib_trx_level) noexcept;
INNODB_API ib_async<ib_trx_state>   ib_trx_state(ib_state* db, ib_trx trx) noexcept;
INNODB_API ib_async<ib_err>         ib_get_duplicate_key(ib_state* db, ib_trx trx, const char **table_name, const char **index_name) noexcept;
INNODB_API ib_async<ib_err>         ib_savepoint_release(ib_state* db, ib_trx trx, const void* name, ib_ulint len) noexcept;
INNODB_API ib_async<ib_err>         ib_savepoint_rollback(ib_state* db, ib_trx trx, const void* name, ib_ulint len) noexcept;
INNODB_API ib_async<void>           ib_savepoint_take(ib_state* db, ib_trx trx, const void* name, ib_ulint len) noexcept;
/// @}

/// \defgroup tuple Tuple operations
/// \ingroup api
/// \brief Tuple operations
/// \details Tuple operations are used to create, manipulate, and access tuples.
/// @{



INNODB_API ib_async<ib_err>         ib_tuple_copy(ib_state* db, ib_tpl dst, const ib_tpl src) noexcept;
INNODB_API ib_async<ib_tpl>         ib_tuple_clear(ib_state* db, ib_tpl tpl) noexcept;
INNODB_API ib_async<void>           ib_tuple_delete(ib_state* db, ib_tpl tpl) noexcept;
INNODB_API ib_async<ib_ulint>       ib_tuple_get_n_cols(ib_state* db, const ib_tpl tpl) noexcept;
INNODB_API ib_async<ib_ulint>       ib_tuple_get_n_user_cols(ib_state* db, const ib_tpl tpl) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_get_cluster_key(ib_state* db, ib_crsr crsr, ib_tpl* dst, const ib_tpl src) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_double(ib_state* db, ib_tpl tpl, ib_ulint col_no, double* dval) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_float(ib_state* db, ib_tpl tpl, ib_ulint col_no, float* fval) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_i8(ib_state* db, ib_tpl tpl, ib_ulint i, ib_i8* ival) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_i16(ib_state* db, ib_tpl tpl, ib_ulint i, ib_i16* ival) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_i32(ib_state* db, ib_tpl tpl, ib_ulint i, ib_i32* ival) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_i64(ib_state* db, ib_tpl tpl, ib_ulint i, ib_i64* ival) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_u8(ib_state* db, ib_tpl tpl, ib_ulint i, ib_u8* ival) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_u16(ib_state* db, ib_tpl tpl, ib_ulint i, ib_u16* ival) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_u32(ib_state* db, ib_tpl tpl, ib_ulint i, ib_u32* ival) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_read_u64(ib_state* db, ib_tpl tpl, ib_ulint i, ib_u64* ival) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_double(ib_state* db, ib_tpl tpl, int col_no, double val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_float(ib_state* db, ib_tpl tpl, int col_no, float val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_i8(ib_state* db, ib_tpl tpl, int col_no, ib_i8 val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_i16(ib_state* db, ib_tpl tpl, int col_no, ib_i16 val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_i32(ib_state* db, ib_tpl tpl, int col_no, ib_i32 val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_i64(ib_state* db, ib_tpl tpl, int col_no, ib_i64 val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_u8(ib_state* db, ib_tpl tpl, int col_no, ib_u8 val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_u16(ib_state* db, ib_tpl tpl, int col_no, ib_u16 val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_u32(ib_state* db, ib_tpl tpl, int col_no, ib_u32 val) noexcept;
INNODB_API ib_async<ib_err>         ib_tuple_write_u64(ib_state* db, ib_tpl tpl, int col_no, ib_u64 val) noexcept;
/// @}


/// \addtogroup other
/// @{
INNODB_API ib_async<ib_err>         ib_schema_lock_exclusive(ib_state* db, ib_trx trx) noexcept;
INNODB_API ib_async<ib_bool>        ib_schema_lock_is_exclusive(ib_state* db, const ib_trx trx) noexcept;
INNODB_API ib_async<ib_bool>        ib_schema_lock_is_shared(ib_state* db, const ib_trx trx) noexcept;
INNODB_API ib_async<ib_err>         ib_schema_lock_shared(ib_state* db, ib_trx trx) noexcept;
INNODB_API ib_async<ib_err>         ib_schema_tables_iterate(ib_state* db, ib_trx trx, ib_schema_visitor_table_all visitor, void* arg) noexcept;
INNODB_API ib_async<ib_err>         ib_schema_unlock(ib_state* db, ib_trx trx) noexcept;
	
INNODB_API ib_async<ib_tpl>         ib_sec_read_tuple_create(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_tpl>         ib_sec_search_tuple_create(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<ib_err>         ib_update_table_statistics(ib_state* db, ib_crsr crsr) noexcept;
INNODB_API ib_async<const char*>    ib_strerror(ib_state* db, ib_err db_errno) noexcept;
INNODB_API ib_async<void>           ib_set_client_compare(ib_state* db, ib_client_cmp client_cmp_func) noexcept;
INNODB_API ib_async<void>           ib_set_panic_handler(ib_state* db, ib_panic_handler handler) noexcept;
INNODB_API ib_async<void>           ib_set_trx_is_interrupted_handler(ib_state* db, ib_trx_is_interrupted_handler handler) noexcept;
INNODB_API ib_async<void>           ib_logger_set(ib_state* db, ib_msg_log ib_msg_log, ib_msg_stream ib_msg_stream) noexcept;
INNODB_API ib_async<ib_err>         ib_status_get_all(ib_state* db, const char*** names, ib_u32* names_num) noexcept;
INNODB_API ib_async<ib_err>         ib_status_get_i64(ib_state* db, const char* name, ib_i64* dst) noexcept;
INNODB_API ib_async<ib_err>         ib_error_inject(ib_state* db, int err) noexcept;

/// @}

/// @}


/// \defgroup other Other Operations
/// \ingroup api
/// \brief Other operations
/// \details Other operations are used to create, manipulate, and access other operations.