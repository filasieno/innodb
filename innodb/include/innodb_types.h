
#pragma once

// Types Moved:
//   - Error code enum (db_err)
//   - All basic type typedefs (ib_err_t, ib_byte_t, etc.)
//   - Configuration, column, table format, and lock mode enums
//   - Struct definitions (ib_col_meta_t, ib_schema_visitor_t, etc.)
//   - Function pointer typedefs for callbacks
//   - Some Constants and macros
//   - The ib_table_stats_t struct
//   - The files are now properly organized with types in innodb_types.h and function declarations in innodb.h, maintaining the existing API while improving code organization.

/// \brief InnoDB error codes. 
/// \details Most of the error codes are internal to the engine and will not be seen by user applications. 
/// The partial error codes reflect the sub-state of an operation within InnoDB. 
/// Some of the error codes are deprecated and are no longer used.
enum db_err {
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

	// The following are API only error codes.
	// ------------------------------------------------------

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

/// \brief InnoDB error codes are represented by ib_err_t. 
/// \details See \ref  for a complete list of possible error codes.
/// \see db_err
/// a complete list of possible error codes.
typedef enum db_err        ib_err_t;

/// \brief Representation of a byte within InnoDB
typedef unsigned char      ib_byte_t;

/// \brief Representation of an unsigned long int within InnoDB
typedef unsigned long int  ib_ulint_t;

/// \brief Representation of a void* within InnoDB
typedef void*              ib_opaque_t;

/// \brief Representation of a "boolean" type within InnoDB
/// \details Ideally we would like to have this as ib_byte_t, but we need to make it
/// the same as the InnoDB internal ibool.
typedef ib_ulint_t         ib_bool_t;

/// \brief A character set pointer
typedef ib_opaque_t        ib_charset_t;

// Integer types used by the API. Microsoft VS defines its own types
// and we use the Microsoft types when building with Visual Studio.

#if defined(_MSC_VER)

	/// \brief A signed 8 bit integral type.
	typedef __int8           ib_i8_t;
	/// \brief An unsigned 8 bit integral type.
	typedef unsigned __int8  ib_u8_t;
	/// \brief A signed 16 bit integral type.
	typedef __int16          ib_i16_t;
	/// \brief An unsigned 16 bit integral type.
	typedef unsigned __int16 ib_u16_t;
	/// \brief A signed 32 bit integral type.
	typedef __int32          ib_i32_t;
	/// \brief An unsigned 32 bit integral type.
	typedef unsigned __int32 ib_u32_t;
	/// \brief A signed 64 bit integral type.
	typedef __int64          ib_i64_t;
	/// \brief An unsigned 64 bit integral type.
	typedef unsigned __int64 ib_u64_t;

#else

	#include <stdint.h>
	/// \brief A signed 8 bit integral type.
	typedef int8_t   ib_i8_t;
	/// \brief An unsigned 8 bit integral type.
	typedef uint8_t  ib_u8_t;
	/// \brief A signed 16 bit integral type.
	typedef int16_t  ib_i16_t;
	/// \brief An unsigned 16 bit integral type.
	typedef uint16_t ib_u16_t;
	/// \brief A signed 32 bit integral type.
	typedef int32_t  ib_i32_t;
	/// \brief An unsigned 32 bit integral type.
	typedef uint32_t ib_u32_t;
	/// \brief A signed 64 bit integral type.
	typedef int64_t  ib_i64_t;
	/// \brief An unsigned 64 bit integral type.
	typedef uint64_t ib_u64_t;

#endif

/// \brief The integral type that represents internal table and index ids.
typedef ib_u64_t ib_id_t;

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
typedef enum {
	
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
} ib_cfg_type_t;

/// \brief Column types that are supported.
typedef enum {
	/// \brief Character varying length. The column is not padded.
	IB_VARCHAR = 1,

	/// \brief Fixed length character string. The column is padded to the right.
	IB_CHAR = 2,

	/// \brief Fixed length binary, similar to IB_CHAR but the column is not padded to the right.
	IB_BINARY =	3,

	/// \brief Variable length binary
	IB_VARBINARY = 4,

	/// \brief Binary large object, or a TEXT type
	IB_BLOB	= 5,

	/// \brief Integer: can be any size from 1 - 8 bytes. If the size is 1, 2, 4 and 8 bytes then you can use the typed read and write functions. For other sizes you will need to use the ib_col_get_value() function and do the conversion yourself.
	IB_INT = 6,

	/// \brief System column, this column can be one of DATA_TRX_ID, DATA_ROLL_PTR or DATA_ROW_ID.
	IB_SYS = 8,

	/// \brief C (float) floating point value.
	IB_FLOAT = 9,

	/// \brief C (double) floating point value.
	IB_DOUBLE =	10,

	/// \brief Decimal stored as an ASCII string.
	IB_DECIMAL = 11,

	/// \brief Any charset, varying length
	IB_VARCHAR_ANYCHARSET =	12,

	/// \brief Any charset, fixed length
	IB_CHAR_ANYCHARSET = 13
} ib_col_type_t;

/// \brief InnoDB table format types
typedef enum {
	/// \brief Redundant row format, the column type and length is stored in the row.
	IB_TBL_REDUNDANT,

	/// \brief Compact row format, the column type is not stored in the row. The length is stored in the row but the storage format uses a compact format to store the length of the column data and record data storage format also uses less storage.
	IB_TBL_COMPACT,

	/// \brief Compact row format. BLOB prefixes are not stored in the clustered index
	IB_TBL_DYNAMIC,

	/// \brief Similar to dynamic format but with pages compressed
	IB_TBL_COMPRESSED
} ib_tbl_fmt_t;

/// \brief InnoDB column attributes
typedef enum {
	IB_COL_NONE     = 0,  //!< No special attributes.
	IB_COL_NOT_NULL = 1,  //!< Column data can't be NULL.
	IB_COL_UNSIGNED = 2,  //!< Column is IB_INT and unsigned.
	IB_COL_NOT_USED = 4,  //!< Future use, reserved.
	IB_COL_CUSTOM1  = 8,  //!< Custom precision type, this is a bit that is ignored by InnoDB and so can be set and queried by users.
	IB_COL_CUSTOM2  = 16, //!< Custom precision type, this is a bit that is ignored by InnoDB and so can be set and queried by users.
	IB_COL_CUSTOM3  = 32  //!< Custom precision type, this is a bit that is ignored by InnoDB and so can be set and queried by users.
} ib_col_attr_t;

/// \brief ib_lck_mode_t InnoDB lock modes.
/// \note must match lock_types.h
typedef enum {
	IB_LOCK_IS = 0,            //!< Intention shared, an intention lock should be used to lock tables
	IB_LOCK_IX,                //!< Intention exclusive, an intention lock should be used to lock tables
	IB_LOCK_S,                 //!< Shared locks should be used to lock rows
	IB_LOCK_X,                 //!< Exclusive locks should be used to lock rows
	IB_LOCK_NOT_USED,          //!< Future use, reserved
	IB_LOCK_NONE,              //!< This is used internally to note consistent read
	IB_LOCK_NUM = IB_LOCK_NONE //!< number of lock modes
} ib_lck_mode_t;

/// \brief ib_srch_mode_t InnoDB cursor search modes for ib_cursor_moveto().
/// \details Values must match those found in page_cur.h
typedef enum {
	IB_CUR_G  = 1,  //!< If search key is not found then position the cursor on the row that is greater than the search key
	IB_CUR_GE = 2,  //!< If the search key not found then position the cursor on the row that is greater than or equal to the search key
	IB_CUR_L  = 3,  //!< If search key is not found then position the cursor on the row that is less than the search key
	IB_CUR_LE = 4   //!< If search key is not found then position the cursor on the row that is less than or equal to the search key
} ib_srch_mode_t;

/// \brief ib_match_mode_t Various match modes used by ib_cursor_moveto()
typedef enum {
	IB_CLOSEST_MATCH, //!< Closest match possible
	IB_EXACT_MATCH,   //!< Search using a complete key value
	IB_EXACT_PREFIX   //!<  Search using a key prefix which must match to rows: the prefix may contain an incomplete field (the last field in prefix may be just a prefix of a fixed length column)
} ib_match_mode_t;

/// \brief InnoDB column meta data.
typedef struct {
	/// \brief Column type
	ib_col_type_t type;

	/// \brief Column attributes
	ib_col_attr_t attr;

	/// \brief Length of type
	ib_u32_t type_len;

	/// \brief 16 bits of data relevant only to the client. InnoDB doesn't care
	ib_u16_t client_type;

	/// \brief Column charset
	ib_charset_t* charset;
} ib_col_meta_t;

/// \brief ib_trx_state_t The transaction state can be queried using th ib_trx_state() function. 
/// \details The InnoDB deadlock monitor can roll back a transaction and users should be prepared for this, especially where there is high contention. 
/// The way to determine the state of the transaction is to query it's state and check.
/// \note Must be in sync with trx_trx.h
typedef enum {
	/// \brief Has not started yet, the transaction has not ben started yet.
	IB_TRX_NOT_STARTED,

	/// \brief The transaction is currently active and needs to be either committed or rolled back.
	IB_TRX_ACTIVE,

	/// \brief Not committed to disk yet
	IB_TRX_COMMITTED_IN_MEMORY,

	/// \brief Support for 2PC/XA
	IB_TRX_PREPARED
} ib_trx_state_t;

/// \brief ib_trx_level_t Transaction isolation levels
/// \details Note: Must be in sync with trx0trx.h
typedef enum {
	/// \brief Dirty read: non-locking SELECTs are performed so that we do not look at a possible earlier version of a record; thus they are not 'consistent' reads under this isolation level; otherwise like level 2
	IB_TRX_READ_UNCOMMITTED = 0,

	/// \brief Somewhat Oracle-like isolation, except that in range UPDATE and DELETE we must block phantom rows with next-key locks; SELECT ... FOR UPDATE and ... LOCK IN SHARE MODE only lock the index records, NOT the gaps before them, and thus allow free inserting; each consistent read reads its own snapshot
	IB_TRX_READ_COMMITTED = 1,

	/// \brief All consistent reads in the same trx read the same snapshot; full next-key locking used in locking reads to block insertions into gaps
	IB_TRX_REPEATABLE_READ = 2,

	/// \brief All plain SELECTs are converted to LOCK IN SHARE MODE reads
	IB_TRX_SERIALIZABLE = 3
} ib_trx_level_t;

/// \brief ib_shutdown_t When ib_shutdown() is called InnoDB may take a long time to shutdown because of background tasks e.g., purging deleted records. The following flags allow the user to control the shutdown behavior.
typedef enum {
	/// \brief Normal shutdown, do insert buffer merge and purge before complete shutdown.
	IB_SHUTDOWN_NORMAL,

	/// \brief Do not do a purge and index buffer merge at shutdown.
	IB_SHUTDOWN_NO_IBUFMERGE_PURGE,

	/// \brief Same as NO_IBUFMERGE_PURGE and in addition do not even flush the buffer pool to data files. No committed transactions are lost
	IB_SHUTDOWN_NO_BUFPOOL_FLUSH
} ib_shutdown_t;

/// \brief Generical InnoDB callback prototype.
typedef void (*ib_cb_t)(void);

/// \brief The first argument to the InnoDB message logging function. 
/// \details By default it's set to stderr. You should treat ib_msg_stream_t as a void*, since it will probably change in the future.
typedef FILE* ib_msg_stream_t;

/// \brief All log messages are written to this function.It should have the same
/// behavior as fprintf(3).
typedef int (*ib_msg_log_t)(ib_msg_stream_t, const char*, ...);

// Note: This is to make it easy for API users to have type
// checking for arguments to our functions. Making it ib_opaque_t
// by itself will result in pointer decay resulting in subverting
// of the compiler's type checking.

/// \brief InnoDB tuple handle. 
/// \details This handle can refer to either a cluster index tuple or a secondary index tuple. 
/// There are two types of tuples for each type of index, making a total of four tuple handles. 
/// There is a tuple for reading the entire row contents and another for searching on the index key.
typedef struct ib_tpl_struct* ib_tpl_t;

/// \brief InnoDB transaction handle, all database operations need to be covered by transactions.
/// \details This handle represents a transaction. The handle can be created with ib_trx_begin(), you commit your changes with ib_trx_commit() and undo your changes using ib_trx_rollback(). 
/// If the InnoDB deadlock monitor rolls back the transaction then you need to free the transaction using the function ib_trx_release(). 
/// You can query the state of an InnoDB transaction by calling ib_trx_state().
typedef struct ib_trx_struct* ib_trx_t;

/// \brief InnoDB cursor handle
typedef struct ib_crsr_struct* ib_crsr_t;

/// \brief InnoDB table schema handle
typedef struct ib_tbl_sch_struct* ib_tbl_sch_t;

/// \brief InnoDB index schema handle
typedef struct ib_idx_sch_struct* ib_idx_sch_t;

/// \brief Currently, this is also the number of callback functions in the struct.
typedef enum {
	IB_SCHEMA_VISITOR_TABLE               = 1,
	IB_SCHEMA_VISITOR_TABLE_COL           = 2,
	IB_SCHEMA_VISITOR_TABLE_AND_INDEX     = 3,
	IB_SCHEMA_VISITOR_TABLE_AND_INDEX_COL = 4
} ib_schema_visitor_version_t;

/// \brief Visit all tables in the InnoDB schem.
/// \param arg User callback arg
/// \param name Table name
/// \param IB_NAME_LEN Length of name in bytes
/// \return 0 on success, nonzero on failure (abort traversal)
typedef int (*ib_schema_visitor_table_all_t) (void* arg, const char* name, int IB_NAME_LEN);

/// \brief Table visitor
/// \param arg       User callback arg
/// \param name      Table name
/// \param tbl_fmt   Table type
/// \param page_size Table page size
/// \param n_cols    No. of cols defined
/// \param n_indexes No. of indexes defined
/// \return 0 on success, nonzero on failure (abort traversal)
typedef int (*ib_schema_visitor_table_t) (void* arg, const char* name, ib_tbl_fmt_t	tbl_fmt, ib_ulint_t	page_size, int n_cols, int n_indexes);

/// \brief Table column visitor
/// \param arg User callback arg
/// \param name Column name
/// \param col_type Column type
/// \param len Column len
/// \param attr Column attributes
/// \return 0 on success, nonzero on failure (abort traversal)
typedef int (*ib_schema_visitor_table_col_t) (void* arg, const char* name, ib_col_type_t col_type, ib_ulint_t len, ib_col_attr_t attr);

/// \brief Index visitor
/// \param arg User callback arg
/// \param name Index name
/// \param clustered True if clustered
/// \param unique True if unique
/// \param n_cols No. of cols defined
/// \return 0 on success, nonzero on failure (abort traversal)
typedef int (*ib_schema_visitor_index_t) (void* arg, const char* name, ib_bool_t clustered, ib_bool_t unique, int n_cols);

/// \brief Index column visitor
/// \param arg User callback arg
/// \param name Column name
/// \param prefix_len Prefix length
/// \return 0 on success, nonzero on failure (abort traversal)
typedef int (*ib_schema_visitor_index_col_t) (void* arg, const char* name, ib_ulint_t prefix_len);

/// \brief Callback functions to traverse the schema of a table.
typedef struct {
	ib_schema_visitor_version_t	  version;   //!< Visitor version
	ib_schema_visitor_table_t     table;     //!< For traversing table info
	ib_schema_visitor_table_col_t table_col; //!< For traversing table column info
	ib_schema_visitor_index_t     index;     //!< For traversing index info
	ib_schema_visitor_index_col_t index_col; //!< For traversing index column info
} ib_schema_visitor_t;

/// \brief This function is used to compare two data fields for which the data type is such that we must use the client code to compare them.
/// \param col_meta Column meta data
/// \param p1 Key
/// \param p1_len Key length
/// \param p2 Key
/// \param p2_len Key length
/// \return 1, 0, -1, if a is greater, equal, less than b, respectively
typedef int (*ib_client_cmp_t)(const ib_col_meta_t* col_meta, const ib_byte_t* p1, ib_ulint_t p1_len, const ib_byte_t* p2, ib_ulint_t p2_len);

// This should be the same as univ.i
/// \brief Represents SQL_NULL length
#define	IB_SQL_NULL (0xFFFFFFFF)

/// \brief The number of system columns in a row.
#define IB_N_SYS_COLS (3)

/// \brief The maximum length of a text column.
#define MAX_TEXT_LEN (4096)

/// \brief The maximum length of a column name in a table schema.
#define IB_MAX_COL_IB_NAME_LEN	(64 * 3)

/// \brief The maximum length of a table name (plus database name).
#define IB_MAX_TABLE_IB_NAME_LEN (64 * 3)


/// \brief Callback function to compare InnoDB key columns in an index.
extern ib_client_cmp_t ib_client_compare;


/// \brief Type of callback in the event of INNODB panicing. Your callback should
/// call exit() rather soon, as continuing after a panic will lead to errors
/// returned from every API function. We have also not fully tested
/// every possible outcome from not immediately calling exit().
typedef void (*ib_panic_handler_t)(void*, int, char*, ...);

/// \brief Callback for checking if a transaction has been interrupted.
/// This callback lets you implement the MySQL KILL command kind of
/// functionality.
/// A transaction may block in the thread it's running in (for example, while
/// acquiring row locks or doing IO) but other threads may do something that
/// causes ib_trx_is_interrupted_handler_t to return true.
typedef int (*ib_trx_is_interrupted_handler_t)(void*);

/// \brief InnoDB Table and index statistics.
typedef struct {
	ib_i64_t stat_n_rows; //!< approximate number of rows in the table; we periodically calculate new estimates
	ib_u64_t stat_clustered_index_size; //!< approximate clustered index size in bytes.
	ib_u64_t stat_sum_of_other_index_sizes; //!< other indexes in bytes
	/// \brief when a row is inserted, updated, or deleted, we add 1 to this number; 
	/// we calculate new estimates for the stat_... 
	/// values for the table and the indexes at an interval of 2 GB or when about 1 / 16 of table has been modified; 
	/// also when an estimate operation is called for; the counter is reset to zero at statistics calculation; 
	/// this counter is not protected by any latch, because this is only used for heuristics
	ib_u64_t	stat_modified_counter;
} ib_table_stats_t;