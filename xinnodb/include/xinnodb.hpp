#pragma once

#define INNODB_API

// ========================================================================================================================
// Top Level Groups
// ========================================================================================================================

/// \defgroup api XInnoDB API
/// \brief XInnoDB API
/// \details XInnoDB API is the interface for the XInnoDB database storage engine.

/// \defgroup sdk XInnoDB SDK API
/// \brief XInnoDB SDK API
/// \details XInnoDB SDK API

/// \defgroup components Internal components
/// \brief XInnoDB Internal components
/// \details Describe the components that make up the XInnoDB database storage engine.


// ========================================================================================================================
// API Groups
// ========================================================================================================================

/// \defgroup cursor Cursor operations
/// \ingroup api
/// \brief Cursor operations
/// \details Cursor operations are used to navigate through the database.

/// \defgroup index Index operations
/// \ingroup api
/// \brief Index operations
/// \details Index operations are used to create, drop, and manage indexes.

/// \defgroup table Table operations
/// \ingroup api
/// \brief Table operations
/// \details Table operations are used to create, rename, drop, and manage tables.

/// \defgroup transaction Transaction operations
/// \ingroup api
/// \brief Transaction operations
/// \details Transaction operations are used to create, commit, and rollback transactions.

/// \defgroup tuple Tuple operations
/// \ingroup api
/// \brief Tuple operations
/// \details Tuple operations are used to create, manipulate, and access tuples.

/// \defgroup other Other Operations
/// \ingroup api
/// \brief Other operations
/// \details Other operations are used to create, manipulate, and access other operations.

// ========================================================================================================================
// SDK Groups
// ========================================================================================================================


// ========================================================================================================================
// Component Groups
// ========================================================================================================================

/// \defgroup prim Primitive types
/// \brief XInnoDB types
/// \details XInnoDB types are the types used in the XInnoDB API.
/// \ingroup api



// ========================================================================================================================
// Definitions
// ========================================================================================================================

/// \brief an unisgned 64 bit integer
/// \ingroup prim
using ib_u64 = unsigned long long;

/// \brief a signed 64 bit integer
/// \ingroup prim
using ib_i64 = signed long long;

/// \brief an unsigned 32 bit integer
/// \ingroup prim
using ib_u32 = unsigned int;

/// \brief a signed 32 bit integer
/// \ingroup prim
using ib_i32 = signed int;

/// \brief an unsigned 16 bit integer
/// \ingroup prim
using ib_u16 = unsigned short;

/// \brief a signed 16 bit integer
/// \ingroup prim
using ib_i16 = signed short;

/// \brief a signed 8 bit integer
/// \ingroup prim
using ib_i8 = signed char;

/// \brief an unsigned 8 bit integer
/// \ingroup prim
using ib_u8 = unsigned char;

/// \brief an unsigned 64 bit integer
/// \ingroup prim
using ib_size = ib_u64;

/// \brief a signed long integer
/// \ingroup prim
using ib_int = signed long int;

/// \brief an unsigned long integer
/// \ingroup prim
using ib_ulint = unsigned long int;

/// \brief a boolean
/// \ingroup prim
using ib_bool = bool;

/// \brief a byte
/// \ingroup prim
using ib_byte = ib_u8;

/// \ingroup prim
/// \brief an unsigned integer that has the same size as a pointer
using ib_uintptr = unsigned long long int;

/// \brief The integral type that represents internal table and index ids.
/// \ingroup prim
using ib_id = ib_u64;

/// \brief InnoDB API version
/// \ingroup prim
struct ib_api_version {
	/// \brief Defines the major version number
	int major;

	/// \brief Defines the minor version number
	int minor;

	/// \brief Defines the revision number
	int revision;

	/// \brief Defines the build commit tag
	const char *build;
};

/// \brief InnoDB Shared Global Area handle
struct ib_sga_hdl {
	/// \brief the data ptr
	ib_uintptr hdl;
};

/// \brief InnoDB state handle
struct ib_state_hdl {
	/// \brief the data ptr
	ib_uintptr hdl;
};

/// \brief InnoDB transaction handle, all database operations need to be covered by transactions.
/// \details The handle can be created with ib_trx_begin(), you commit your changes with ib_trx_commit() and undo your changes using ib_trx_rollback().
/// If the InnoDB deadlock monitor rolls back the transaction then you need to free the transaction using the function ib_trx_release().
/// You can query the state of an InnoDB transaction by calling ib_trx_state().
/// \ingroup transaction
struct ib_trx_hdl {
	/// \brief the data ptr
	ib_uintptr hdl;
};

/// \brief InnoDB cursor handle
/// \ingroup cursor
struct ib_crsr_hdl {
	/// \brief the data ptr
	ib_uintptr hdl;
};

/// \brief InnoDB tuple handle.
/// \details This handle can refer to either a cluster index tuple or a secondary index tuple.
/// There are two types of tuples for each type of index, making a total of four tuple handles.
/// There is a tuple for reading the entire row contents and another for searching on the index key.
/// \ingroup tuple
struct ib_tpl_hdl {
	/// \brief the data ptr
	ib_uintptr hdl;
};

/// \brief InnoDB message stream handle
/// \ingroup other
struct ib_msg_stream_hdl {
	/// \brief the data ptr
	ib_uintptr hdl;
};

/// \brief InnoDB main task handle
struct ib_main_task_hdl {
	/// \brief the data ptr
	ib_uintptr hdl;
};

/// \brief InnoDB table schema handle
/// \ingroup schema
struct ib_tbl_sch_hdl {
	ib_uintptr hdl;
};

/// \brief InnoDB index schema handle
/// \ingroup schema
struct ib_idx_sch_hdl {
	ib_uintptr hdl;
};

/// \brief InnoDB task handle
template <typename T>
struct ib_task_hdl {
	ib_uintptr hdl;
};

/// \addtogroup other
/// @{

/// \brief InnoDB Shared Global Area state descriptor
struct ib_sga_state_desc {
	ib_u64 frame_count;
	ib_u64 worker_buffer_size;
	ib_u64 context_buffer_size;
	ib_u64 log_buffer_size;
	ib_u64 debug_buffer_size;
	ib_ulint file_table_capacity;
};

/// \brief InnoDB Worker state descriptor
struct ib_state_desc {
};

/// \brief InnoDB aysnc operation awaitable
template <typename T>
struct ib_async {
};

/// \brief InnoDB error codes.
/// \details Most of the error codes are internal to the engine and will not be seen by user applications.
/// The partial error codes reflect the sub-state of an operation within InnoDB.
/// Some of the error codes are deprecated and are no longer used.
enum ib_err {
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

using ib_worker_main_fn = ib_main_task_hdl(ib_state_hdl state);

struct ib_client_cmp;
struct ib_col_meta;
struct ib_msg_log;

struct ib_schema_visitor;
struct ib_schema_visitor_table_all;
struct ib_table_stats;

/// \brief Possible types for a configuration variable.
/// \details XXX Can we avoid having different types for ulint and ulong?
/// - On Win64 "unsigned long" is 32 bits
/// - ulong is always defined as "unsigned long"
/// - On Win64 ulint is defined as 64 bit integer
/// => On Win64 ulint != ulong.
/// If we typecast all ulong and ulint variables to the smaller type
/// ulong, then we will cut the range of the ulint variables.
/// This is not a problem for most ulint variables because their max
/// allowed values do not exceed 2^32-1 (e.g. log_groups is ulint
/// but its max allowed value is 10). BUT buffer_pool_size and
/// log_file_size allow up to 2^64-1.
enum ib_cfg_type {

	/// \brief The configuration parameter is of type ibool.
	IB_CFG_IBOOL,

	/// \brief The configuration parameter is of type ulint.
	IB_CFG_ULINT,

	/// \brief The configuration parameter is of type ulong.
	IB_CFG_ULONG,

	/// \brief The configuration parameter is of type char*.
	IB_CFG_TEXT,

	/// \brief The configuration parameter a callback parameter.
	IB_CFG_CB
};

/// \brief Column types that are supported.
enum ib_col_type {
	/// \brief Character varying length. The column is not padded.
	IB_VARCHAR = 1,

	/// \brief Fixed length character string. The column is padded to the right.
	IB_CHAR = 2,

	/// \brief Fixed length binary, similar to IB_CHAR but the column is not padded to the right.
	IB_BINARY = 3,

	/// \brief Variable length binary
	IB_VARBINARY = 4,

	/// \brief Binary large object, or a TEXT type
	IB_BLOB = 5,

	/// \brief Integer: can be any size from 1 - 8 bytes. If the size is 1, 2, 4 and 8 bytes then you can use the typed read and write functions. For other sizes you will need to use the ib_col_get_value() function and do the conversion yourself.
	IB_INT = 6,

	/// \brief System column, this column can be one of DATA_TRX_ID, DATA_ROLL_PTR or DATA_ROW_ID.
	IB_SYS = 8,

	/// \brief C (float) floating point value.
	IB_FLOAT = 9,

	/// \brief C (double) floating point value.
	IB_DOUBLE = 10,

	/// \brief Decimal stored as an ASCII string.
	IB_DECIMAL = 11,

	/// \brief Any charset, varying length
	IB_VARCHAR_ANYCHARSET = 12,

	/// \brief Any charset, fixed length
	IB_CHAR_ANYCHARSET = 13
};

/// \brief InnoDB table format types
enum ib_tbl_fmt {
	/// \brief Redundant row format, the column type and length is stored in the row.
	IB_TBL_REDUNDANT,

	/// \brief Compact row format, the column type is not stored in the row. The length is stored in the row but the storage format uses a compact format to store the length of the column data and record data storage format also uses less storage.
	IB_TBL_COMPACT,

	/// \brief Compact row format. BLOB prefixes are not stored in the clustered index
	IB_TBL_DYNAMIC,

	/// \brief Similar to dynamic format but with pages compressed
	IB_TBL_COMPRESSED
};

/// \brief InnoDB column attributes
enum ib_col_attr {
	/// \brief No special attributes
	IB_COL_NONE = 0,

	/// \brief Column data can't be NULL
	IB_COL_NOT_NULL = 1,

	/// \brief Column is IB_INT and unsigned
	IB_COL_UNSIGNED = 2,

	/// \brief Future use, reserved
	IB_COL_NOT_USED = 4,

	IB_COL_CUSTOM1 = 8,
	IB_COL_CUSTOM2 = 16,
	IB_COL_CUSTOM3 = 32
};

/// \brief InnoDB lock modes.
enum ib_lck_mode {
	IB_LOCK_IS = 0,				  //!< Intention shared, an intention lock should be used to lock tables
	IB_LOCK_IX,					  //!< Intention exclusive, an intention lock should be used to lock tables
	IB_LOCK_S,					  //!< Shared locks should be used to lock rows
	IB_LOCK_X,					  //!< Exclusive locks should be used to lock rows
	IB_LOCK_NOT_USED,			  //!< Future use, reserved
	IB_LOCK_NONE,				  //!< This is used internally to note consistent read
	IB_LOCK_NUM = IB_LOCK_NONE	  //!< number of lock modes
};

/// \brief ib_srch_mode_t InnoDB cursor search modes for ib_cursor_moveto().
/// \details Values must match those found in page_cur.h
enum ib_srch_mode {
	IB_CUR_G = 1,	  //!< If search key is not found then position the cursor on the row that is greater than the search key
	IB_CUR_GE = 2,	  //!< If the search key not found then position the cursor on the row that is greater than or equal to the search key
	IB_CUR_L = 3,	  //!< If search key is not found then position the cursor on the row that is less than the search key
	IB_CUR_LE = 4	  //!< If search key is not found then position the cursor on the row that is less than or equal to the search key
};

/// \brief ib_match_mode_t Various match modes used by ib_cursor_moveto()
enum ib_match_mode {
	IB_CLOSEST_MATCH,	 //!< Closest match possible
	IB_EXACT_MATCH,		 //!< Search using a complete key value
	IB_EXACT_PREFIX		 //!<  Search using a key prefix which must match to rows: the prefix may contain an incomplete field (the last field in prefix may be just a prefix of a fixed length column)
};

/// \brief ib_trx_state_t The transaction state can be queried using th ib_trx_state() function.
/// \details The InnoDB deadlock monitor can roll back a transaction and users should be prepared for this, especially where there is high contention.
/// The way to determine the state of the transaction is to query it's state and check.
enum ib_trx_state {
	/// \brief Has not started yet, the transaction has not ben started yet.
	IB_TRX_NOT_STARTED,

	/// \brief The transaction is currently active and needs to be either committed or rolled back.
	IB_TRX_ACTIVE,

	/// \brief Not committed to disk yet
	IB_TRX_COMMITTED_IN_MEMORY,

	/// \brief Support for 2PC/XA
	IB_TRX_PREPARED
};

/// \brief ib_trx_level_t Transaction isolation levels
/// \details Note: Must be in sync with trx0trx.h
enum ib_trx_level {
	/// \brief Dirty read: non-locking SELECTs are performed so that we do not look at a possible earlier version of a record; thus they are not 'consistent' reads under this isolation level; otherwise like level 2
	IB_TRX_READ_UNCOMMITTED = 0,

	/// \brief Somewhat Oracle-like isolation, except that in range UPDATE and DELETE we must block phantom rows with next-key locks; SELECT ... FOR UPDATE and ... LOCK IN SHARE MODE only lock the index records, NOT the gaps before them, and thus allow free inserting; each consistent read reads its own snapshot
	IB_TRX_READ_COMMITTED = 1,

	/// \brief All consistent reads in the same trx read the same snapshot; full next-key locking used in locking reads to block insertions into gaps
	IB_TRX_REPEATABLE_READ = 2,

	/// \brief All plain SELECTs are converted to LOCK IN SHARE MODE reads
	IB_TRX_SERIALIZABLE = 3
};

/// \brief ib_shutdown_t When ib_shutdown() is called InnoDB may take a long time to shutdown because of background tasks e.g., purging deleted records. The following flags allow the user to control the shutdown behavior.
enum ib_shutdown {
	/// \brief Normal shutdown, do insert buffer merge and purge before complete shutdown.
	IB_SHUTDOWN_NORMAL,

	/// \brief Do not do a purge and index buffer merge at shutdown.
	IB_SHUTDOWN_NO_IBUFMERGE_PURGE,

	/// \brief Same as NO_IBUFMERGE_PURGE and in addition do not even flush the buffer pool to data files. No committed transactions are lost
	IB_SHUTDOWN_NO_BUFPOOL_FLUSH
};

/// \brief Generical InnoDB callback prototype.
using ib_cb_fn = void(void);

/// \brief All log messages are written to this function.It should have the same
/// behavior as fprintf(3).
template <typename... Args>
using ib_msg_log_fn = int(ib_msg_stream_hdl msg_stream, const char *format, Args... args);

// Note: This is to make it easy for API users to have type
// checking for arguments to our functions. Making it ib_opaque_t
// by itself will result in pointer decay resulting in subverting
// of the compiler's type checking.

/// \brief Currently, this is also the number of callback functions in the struct.
enum ib_schema_visitor_version {
	IB_SCHEMA_VISITOR_TABLE = 1,
	IB_SCHEMA_VISITOR_TABLE_COL = 2,
	IB_SCHEMA_VISITOR_TABLE_AND_INDEX = 3,
	IB_SCHEMA_VISITOR_TABLE_AND_INDEX_COL = 4
};

/// \brief Visit all tables in the InnoDB schem.
/// \param arg User callback arg
/// \param name Table name
/// \param name_len Length of name in bytes
/// \return 0 on success, nonzero on failure (abort traversal)
using ib_schema_visitor_table_all_fn = int(void *arg, const char *name, int name_len);

/// \brief Table visitor
/// \param arg       User callback arg
/// \param name      Table name
/// \param tbl_fmt   Table type
/// \param page_size Table page size
/// \param n_cols    No. of cols defined
/// \param n_indexes No. of indexes defined
/// \return 0 on success, nonzero on failure (abort traversal)
using ib_schema_visitor_table_fn = int(void *arg, const char *name, ib_tbl_fmt tbl_fmt, ib_ulint page_size, int n_cols, int n_indexes);

/// \brief Table column visitor
/// \param arg User callback arg
/// \param name Column name
/// \param col_type Column type
/// \param len Column len
/// \param attr Column attributes
/// \return 0 on success, nonzero on failure (abort traversal)
using ib_schema_visitor_table_col_fn = int(void *arg, const char *name, ib_col_type col_type, ib_ulint len, ib_col_attr attr);

/// \brief Index visitor
/// \param arg User callback arg
/// \param name Index name
/// \param clustered True if clustered
/// \param unique True if unique
/// \param n_cols No. of cols defined
/// \return 0 on success, nonzero on failure (abort traversal)
using ib_schema_visitor_index_fn = int(void *arg, const char *name, ib_bool clustered, ib_bool unique, int n_cols);

/// \brief Index column visitor
/// \param arg User callback arg
/// \param name Column name
/// \param prefix_len Prefix length
/// \return 0 on success, nonzero on failure (abort traversal)
using ib_schema_visitor_index_col_fn = int(void *arg, const char *name, ib_ulint prefix_len);

/// \brief Callback functions to traverse the schema of a table.
struct ib_schema_visitor {
	/// \brief Visitor version
	ib_schema_visitor_version version;

	/// \brief For traversing table info
	ib_schema_visitor_table_fn table;

	/// \brief For traversing table column info
	ib_schema_visitor_table_col_fn table_col;

	/// \brief For traversing index info
	ib_schema_visitor_index_fn index;

	/// \brief For traversing index column info
	ib_schema_visitor_index_col_fn index_col;
};

/// \brief This function is used to compare two data fields for which the data type is such that we must use the client code to compare them.
/// \param col_meta Column meta data
/// \param p1 Key
/// \param p1_len Key length
/// \param p2 Key
/// \param p2_len Key length
/// \return 1, 0, -1, if a is greater, equal, less than b, respectively
using ib_client_cmp_fn = int(const ib_col_meta *col_meta, const ib_byte *p1, ib_ulint p1_len, const ib_byte *p2, ib_ulint p2_len);

/// \brief Represents SQL_NULL length
/// \ingroup prim
constinit ib_u32 IB_SQL_NULL = 0xFFFFFFFF;

/// \brief The number of system columns in a row.
/// \ingroup prim
constinit ib_u32 IB_N_SYS_COLS = 3;

/// \brief The maximum length of a text column.
/// \ingroup prim
constinit ib_u32 MAX_TEXT_LEN = 4096;

/// \brief The maximum length of a column name in a table schema.
/// \ingroup prim
constinit ib_u32 IB_MAX_COL_IB_NAME_LEN = 64 * 3;

/// \brief The maximum length of a table name (plus database name).
/// \ingroup prim
constinit ib_u32 IB_MAX_TABLE_IB_NAME_LEN = 64 * 3;

/// \brief Type of callback in the event of INNODB panicing.
/// \details Your callback should
/// call exit() rather soon, as continuing after a panic will lead to errors
/// returned from every API function. We have also not fully tested
/// every possible outcome from not immediately calling exit().
/// \ingroup prim
using ib_panic_handler = void(void *, int, char *, ...);

/// \brief Callback for checking if a transaction has been interrupted.
/// This callback lets you implement the MySQL KILL command kind of
/// functionality.
/// A transaction may block in the thread it's running in (for example, while
/// acquiring row locks or doing IO) but other threads may do something that
/// causes ib_trx_is_interrupted_handler_t to return true.
using ib_trx_is_interrupted_handler = int(void *);

/// \brief InnoDB Table and index statistics.
struct ib_table_stats {
	/// \brief approximate number of rows in the table; we periodically calculate new estimates
	ib_i64 stat_n_rows;

	/// \brief approximate clustered index size in bytes.
	ib_i64 stat_clustered_index_size;

	/// \brief other indexes in bytes
	ib_i64 stat_sum_of_other_index_sizes;

	/// \brief When a row is inserted, updated, or deleted, we add 1 to this number.
	/// \details we calculate new estimates for the stat_* values for the table and the indexes at an interval of 2 GB or when about 1 / 16 of table has been modified;
	/// also when an estimate operation is called for; the counter is reset to zero at statistics calculation;
	/// this counter is not protected by any latch, because this is only used for heuristics
	ib_u64 stat_modified_counter;
};



INNODB_API ib_api_version ib_get_api_version() noexcept;

INNODB_API ib_u64 ib_get_sga_state_required_size(ib_sga_state_desc *desc) noexcept;
INNODB_API ib_u64 ib_get_state_required_size(ib_state_desc *desc) noexcept;

INNODB_API ib_sga_hdl ib_sga_init(void *buffer, ib_u64 buffer_size, ib_sga_state_desc *desc, ib_err *out_err = nullptr) noexcept;
INNODB_API void ib_sga_fini(ib_sga_hdl state) noexcept;

INNODB_API int ib_run_worker(ib_worker_main_fn *worker_fn, ib_sga_hdl sga) noexcept;

INNODB_API ib_async<ib_bool> ib_database_create(ib_state_hdl db_hdl, const char *db_name) noexcept;
INNODB_API ib_async<ib_err> ib_database_drop(ib_state_hdl db_hdl, const char *db_name) noexcept;
INNODB_API ib_async<ib_err> ib_shutdown(ib_state_hdl db, ib_shutdown flag) noexcept;
INNODB_API ib_async<ib_err> ib_startup(ib_state_hdl db, const char *format) noexcept;

INNODB_API ib_async<ib_err> ib_cfg_get(ib_state_hdl db, const char *name, void *value) noexcept;
INNODB_API ib_async<ib_err> ib_cfg_get_all(ib_state_hdl db, const char ***names, ib_u32 *names_num) noexcept;
INNODB_API ib_async<ib_err> ib_cfg_set(ib_state_hdl db, const char *name, ...) noexcept;
INNODB_API ib_async<ib_err> ib_cfg_var_get_type(ib_state_hdl db, const char *name, ib_cfg_type *type) noexcept;

INNODB_API ib_async<ib_tpl_hdl> ib_clust_read_tuple_create(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_tpl_hdl> ib_clust_search_tuple_create(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;

/// @}



/// \addtogroup cursor
/// @{

INNODB_API ib_async<ib_err> ib_cursor_open_table(ib_state_hdl db, const char *name, ib_trx_hdl trx, ib_crsr_hdl *out_cur) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_open_table_using_id(ib_state_hdl db, ib_id table_id, ib_trx_hdl trx, ib_crsr_hdl *out_crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_open_index_using_name(ib_state_hdl db, ib_crsr_hdl open_crsr, const char *index_name, ib_crsr_hdl *out_crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_open_index_using_id(ib_state_hdl db, ib_id index_id, ib_trx_hdl trx, ib_crsr_hdl *out_crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_reset(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_close(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<void> ib_cursor_attach_trx(ib_state_hdl db, ib_crsr_hdl crsr, ib_trx_hdl trx) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_insert_row(ib_state_hdl db, ib_crsr_hdl crsr, const ib_tpl_hdl tpl) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_read_row(ib_state_hdl db, ib_crsr_hdl crsr, ib_tpl_hdl tpl) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_delete_row(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_bool> ib_cursor_is_positioned(ib_state_hdl db, const ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_first(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_last(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_next(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_prev(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_lock(ib_state_hdl db, ib_crsr_hdl crsr, ib_lck_mode mode) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_moveto(ib_state_hdl db, ib_crsr_hdl crsr, ib_tpl_hdl tpl, ib_srch_mode ib_srch_mode, int *result) noexcept;
INNODB_API ib_async<void> ib_cursor_set_cluster_access(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<void> ib_cursor_set_lock_mode(ib_state_hdl db, ib_crsr_hdl crsr, ib_lck_mode mode) noexcept;
INNODB_API ib_async<void> ib_cursor_set_match_mode(ib_state_hdl db, ib_crsr_hdl crsr, ib_match_mode match_mode) noexcept;
INNODB_API ib_async<void> ib_cursor_set_simple_select(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<void> ib_cursor_stmt_begin(ib_state_hdl db, ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_truncate(ib_state_hdl db, ib_crsr_hdl crsr, ib_id *table_id) noexcept;
INNODB_API ib_async<ib_err> ib_cursor_update_row(ib_state_hdl db, ib_crsr_hdl crsr, const ib_tpl_hdl old_tpl, const ib_tpl_hdl new_tpl) noexcept;
INNODB_API ib_async<ib_err> ib_get_table_statistics(ib_state_hdl db, ib_crsr_hdl crsr, ib_table_stats *out_table_stats) noexcept;
INNODB_API ib_async<ib_err> ib_get_index_stat_n_diff_key_vals(ib_state_hdl db, ib_crsr_hdl crsr, const char *index_name, ib_u64 *out_ncols, ib_i64 **out_n_diff) noexcept;
/// @}


/// \addtogroup index
/// @{

/// \brief Index schema
/// \details Index schema is used to create, drop, and manage indexes.

INNODB_API ib_async<ib_err> ib_index_schema_create(ib_trx_hdl usr_trx, const char *name, const char *table_name, ib_idx_sch_hdl *idx_sch) noexcept;
INNODB_API ib_async<void> ib_index_schema_delete(ib_idx_sch_hdl idx_sch) noexcept;
INNODB_API ib_async<ib_err> ib_index_schema_add_col(ib_idx_sch_hdl idx_sch, const char *name, ib_ulint prefix_len) noexcept;
INNODB_API ib_async<ib_err> ib_index_schema_set_clustered(ib_idx_sch_hdl idx_sch) noexcept;
INNODB_API ib_async<ib_err> ib_index_schema_set_unique(ib_idx_sch_hdl idx_sch) noexcept;
INNODB_API ib_async<ib_err> ib_index_create(ib_trx_hdl trx, ib_idx_sch_hdl idx_sch, ib_id *out_index_id) noexcept;
INNODB_API ib_async<ib_err> ib_index_drop(ib_trx_hdl trx, ib_id index_id) noexcept;
INNODB_API ib_async<ib_err> ib_index_get_id(ib_state_hdl db, const char *table_name, const char *index_name, ib_id *index_id) noexcept;

/// @}


/// \addtogroup table
/// @{
enum ib_client_type {

};

INNODB_API ib_async<ib_err> ib_table_schema_create(ib_state_hdl db, const char *name, ib_tbl_sch_hdl tbl_sch, ib_tbl_fmt tbl_fmt, ib_ulint page_size) noexcept;
INNODB_API ib_async<ib_err> ib_table_schema_add_col(ib_state_hdl db, ib_tbl_sch_hdl tbl_sch, const char *name, ib_col_type col_type, ib_col_attr col_attr, ib_client_type client_type, ib_ulint len) noexcept;
INNODB_API ib_async<ib_err> ib_table_schema_add_index(ib_state_hdl db, ib_tbl_sch_hdl tbl_sch, const char *name, ib_tbl_sch_hdl idx_sch) noexcept;
INNODB_API ib_async<void> ib_table_schema_delete(ib_state_hdl db, ib_tbl_sch_hdl tbl_sch) noexcept;
INNODB_API ib_async<ib_err> ib_table_schema_visit(ib_state_hdl db, ib_trx_hdl trx, const char *name, const ib_schema_visitor *visitor, void *arg) noexcept;

INNODB_API ib_async<ib_err> ib_table_create(ib_trx_hdl trx, const ib_tbl_sch_hdl sch, ib_id *out_table_id) noexcept;
INNODB_API ib_async<ib_err> ib_table_rename(ib_trx_hdl trx, const char *old_name, const char *new_name) noexcept;
INNODB_API ib_async<ib_err> ib_table_drop(ib_trx_hdl trx, const char *name) noexcept;
INNODB_API ib_async<ib_err> ib_table_get_id(const char *table_name, ib_id *out_table_id) noexcept;
INNODB_API ib_async<ib_err> ib_table_lock(ib_trx_hdl trx, ib_id table_id, ib_lck_mode mode) noexcept;
INNODB_API ib_async<ib_err> ib_table_truncate(ib_trx_hdl trx, const char *table_name, ib_id *table_id) noexcept;

/// @}


/// \addtogroup transaction
/// @{

INNODB_API ib_async<ib_err> ib_trx_begin(ib_state_hdl db, ib_trx_level ib_trx_level) noexcept;
INNODB_API ib_async<void> ib_savepoint_take(ib_trx_hdl trx, const char *name, ib_ulint name_len) noexcept;
INNODB_API ib_async<ib_err> ib_trx_commit(ib_trx_hdl trx) noexcept;
INNODB_API ib_async<ib_err> ib_trx_rollback(ib_trx_hdl trx) noexcept;
INNODB_API ib_async<void> ib_trx_set_client_data(ib_trx_hdl trx, void *client_data) noexcept;
INNODB_API ib_async<ib_err> ib_trx_release(ib_trx_hdl trx) noexcept;
INNODB_API ib_async<ib_err> ib_trx_start(ib_trx_hdl trx, ib_trx_level ib_trx_level) noexcept;
INNODB_API ib_async<ib_trx_state> ib_trx_state(ib_trx_hdl trx) noexcept;
INNODB_API ib_async<ib_err> ib_get_duplicate_key(ib_trx_hdl trx, const char **table_name, ib_ulint table_name_len, const char **index_name, ib_ulint index_name_len) noexcept;
INNODB_API ib_async<ib_err> ib_savepoint_release(ib_trx_hdl trx, const char *name, ib_ulint name_len) noexcept;
INNODB_API ib_async<ib_err> ib_savepoint_rollback(ib_trx_hdl trx, const char *name, ib_ulint name_len) noexcept;

INNODB_API ib_async<ib_err> ib_schema_lock_exclusive(ib_trx_hdl trx) noexcept;
INNODB_API ib_async<ib_bool> ib_schema_lock_is_exclusive(const ib_trx_hdl trx) noexcept;
INNODB_API ib_async<ib_bool> ib_schema_lock_is_shared(const ib_trx_hdl trx) noexcept;
INNODB_API ib_async<ib_err> ib_schema_lock_shared(ib_trx_hdl trx) noexcept;
INNODB_API ib_async<ib_err> ib_schema_unlock(ib_trx_hdl trx) noexcept;

/// @}


/// \addtogroup tuple
/// @{

struct ib_charset;

/// \brief InnoDB column meta data.
struct ib_col_meta {
	/// \brief Column type
	ib_col_type type;

	/// \brief Column attributes
	ib_col_attr attr;

	/// \brief Length of type
	ib_u32 type_len;

	/// \brief 16 bits of data relevant only to the client. InnoDB doesn't care
	ib_u16 client_type;

	/// \brief Column charset
	ib_charset *charset;
};

INNODB_API ib_async<ib_err> ib_tuple_copy(ib_tpl_hdl dst, const ib_tpl_hdl src) noexcept;
INNODB_API ib_async<ib_tpl_hdl> ib_tuple_clear(ib_tpl_hdl tpl) noexcept;
INNODB_API ib_async<void> ib_tuple_delete(ib_tpl_hdl tpl) noexcept;
INNODB_API ib_async<ib_ulint> ib_tuple_get_n_cols(ib_tpl_hdl tpl) noexcept;
INNODB_API ib_async<ib_ulint> ib_tuple_get_n_user_cols(ib_tpl_hdl tpl) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_get_cluster_key(ib_crsr_hdl crsr, ib_tpl_hdl dst, const ib_tpl_hdl src) noexcept;

INNODB_API ib_async<ib_err> ib_tuple_read_double(ib_tpl_hdl tpl, ib_ulint col_no, double *dval) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_float(ib_tpl_hdl tpl, ib_ulint col_no, float *fval) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_i8(ib_tpl_hdl tpl, ib_ulint col_no, ib_i8 *ival) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_i16(ib_tpl_hdl tpl, ib_ulint col_no, ib_i16 *ival) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_i32(ib_tpl_hdl tpl, ib_ulint col_no, ib_i32 *ival) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_i64(ib_tpl_hdl tpl, ib_ulint col_no, ib_i64 *ival) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_u8(ib_tpl_hdl tpl, ib_ulint col_no, ib_u8 *ival) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_u16(ib_tpl_hdl tpl, ib_ulint col_no, ib_u16 *ival) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_u32(ib_tpl_hdl tpl, ib_ulint col_no, ib_u32 *ival) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_read_u64(ib_tpl_hdl tpl, ib_ulint col_no, ib_u64 *ival) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_double(ib_tpl_hdl tpl, ib_ulint col_no, double val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_float(ib_tpl_hdl tpl, ib_ulint col_no, float val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_i8(ib_tpl_hdl tpl, ib_ulint col_no, ib_i8 val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_i16(ib_tpl_hdl tpl, ib_ulint col_no, ib_i16 val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_i32(ib_tpl_hdl tpl, ib_ulint col_no, ib_i32 val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_i64(ib_tpl_hdl tpl, ib_ulint col_no, ib_i64 val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_u8(ib_tpl_hdl tpl, ib_ulint col_no, ib_u8 val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_u16(ib_tpl_hdl tpl, ib_ulint col_no, ib_u16 val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_u32(ib_tpl_hdl tpl, ib_ulint col_no, ib_u32 val) noexcept;
INNODB_API ib_async<ib_err> ib_tuple_write_u64(ib_tpl_hdl tpl, ib_ulint col_no, ib_u64 val) noexcept;

INNODB_API ib_async<ib_ulint> ib_col_copy_value(ib_tpl_hdl tpl, ib_ulint col_no, void *dst, ib_ulint dst_len) noexcept;
INNODB_API ib_async<ib_ulint> ib_col_get_len(ib_tpl_hdl tpl, ib_ulint col_no) noexcept;
INNODB_API ib_async<ib_ulint> ib_col_get_meta(ib_tpl_hdl tpl, ib_ulint col_no, ib_col_meta *out_col_meta) noexcept;
INNODB_API ib_async<const void *> ib_col_get_value(ib_tpl_hdl tpl, ib_ulint col_no) noexcept;
INNODB_API ib_async<ib_err> ib_col_set_value(ib_tpl_hdl tpl, ib_ulint col_no, const void *src, ib_ulint len) noexcept;
/// @}

/// \addtogroup other
/// @{

INNODB_API ib_async<const char *> ib_strerror(ib_err db_errno) noexcept;

INNODB_API ib_async<ib_err> ib_schema_tables_iterate(ib_trx_hdl trx, ib_schema_visitor_table_all visitor, void *arg) noexcept;

INNODB_API ib_async<ib_tpl_hdl> ib_sec_read_tuple_create(ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_tpl_hdl> ib_sec_search_tuple_create(ib_crsr_hdl crsr) noexcept;
INNODB_API ib_async<ib_err> ib_update_table_statistics(ib_crsr_hdl crsr) noexcept;

INNODB_API ib_async<void> ib_set_client_compare(ib_state_hdl db, ib_client_cmp client_cmp_func) noexcept;
INNODB_API ib_async<void> ib_set_panic_handler(ib_state_hdl db, ib_panic_handler handler) noexcept;
INNODB_API ib_async<void> ib_set_trx_is_interrupted_handler(ib_state_hdl db, ib_trx_is_interrupted_handler handler) noexcept;
INNODB_API ib_async<void> ib_logger_set(ib_state_hdl db, ib_msg_log msg_log, ib_msg_stream_hdl stream) noexcept;

INNODB_API ib_async<ib_err> ib_status_get_all(ib_state_hdl db, const char ***names, ib_u32 *names_num) noexcept;
INNODB_API ib_async<ib_err> ib_status_get_i64(ib_state_hdl db, const char *name, ib_i64 *dst) noexcept;
INNODB_API ib_async<ib_err> ib_error_inject(ib_state_hdl db, int err) noexcept;

/// @}

/// @}

