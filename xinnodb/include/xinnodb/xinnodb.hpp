#pragma once

#define INNODB_API

using ib_u64 = unsigned long long;
using size_t = ib_u64;

struct ib_sga_state;
struct ib_state;

/// \brief InnoDB Shared Global Area state descriptor
struct ib_sga_state_desc {

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

struct ib_api_version {
    int major;
    int minor; 
    int revision;
    const char* build;
};

struct ib_err {
    int code;
};

extern int ib_version;

using ib_worker_main_fn = ib_main_task_hdl(ib_state* state);


struct ib_bool;
struct ib_shutdown;
struct ib_u32;
struct ib_tpl;
struct ib_ulint;
struct ib_i64;
struct ib_i32;
struct ib_i16;
struct ib_i8;

struct ib_u16;
struct ib_u8;
struct ib_cfg_type;
struct ib_crsr;
struct ib_trx;
struct ib_lck_mode;
struct ib_id;
struct ib_col_meta;
struct ib_idx_sch;
struct ib_match_mode;
struct ib_table_stats;
struct ib_tbl_sch;
struct ib_tbl_fmt;
struct ib_col_type;
struct ib_col_attr;
struct ib_client_cmp;
struct ib_srch_mode;
struct ib_msg_log;
struct ib_schema_visitor_table_all;
struct ib_schema_visitor;
struct ib_trx_level;
struct ib_trx_state;
struct ib_msg_stream;
struct ib_trx_is_interrupted_handler;
struct ib_panic_handler;

INNODB_API ib_api_version           ib_get_version();

INNODB_API ib_u64                   ib_get_sga_state_required_size(ib_sga_state_desc* desc);
INNODB_API ib_u64                   ib_get_state_required_size(ib_state_desc* desc);

INNODB_API ib_sga_state*            ib_sga_init(void* buffer, ib_u64 buffer_size, ib_sga_state_desc* desc, ib_err* out_err);
INNODB_API void                     ib_sga_fini(ib_sga_state* state);

INNODB_API int                      ib_run_worker(ib_worker_main_fn* worker_fn, ib_sga_state* sga);

INNODB_API ib_async<ib_bool>        ib_database_create(ib_state* db, const char* db_name);
INNODB_API ib_async<ib_err>         ib_database_drop(ib_state* db, const char* db_name);
INNODB_API ib_async<ib_err>         ib_shutdown(ib_state* db, ib_shutdown flag);
INNODB_API ib_async<ib_err>         ib_startup(ib_state* db, const char* format);

INNODB_API ib_async<ib_err>         ib_cfg_get(ib_state* db, const char* name, void* value);
INNODB_API ib_async<ib_err>         ib_cfg_get_all(ib_state* db, const char*** names, ib_u32* names_num);
INNODB_API ib_async<ib_err>         ib_cfg_set(ib_state* db, const char* name, ...);
INNODB_API ib_async<ib_err>         ib_cfg_var_get_type(ib_state* db, const char* name, ib_cfg_type* type);

INNODB_API ib_async<ib_tpl>         ib_clust_read_tuple_create(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_tpl>         ib_clust_search_tuple_create(ib_state* db, ib_crsr crsr);

INNODB_API ib_async<ib_ulint>       ib_col_copy_value(ib_state* db, ib_tpl tpl, ib_ulint i, void* dst, ib_ulint len);
INNODB_API ib_async<ib_ulint>       ib_col_get_len(ib_state* db, ib_tpl tpl, ib_ulint i);
INNODB_API ib_async<ib_ulint>       ib_col_get_meta(ib_state* db, ib_tpl tpl, ib_ulint i, ib_col_meta* col_meta);
INNODB_API ib_async<const void*>    ib_col_get_value(ib_state* db, ib_tpl tpl, ib_ulint i);
INNODB_API ib_async<ib_err>         ib_col_set_value(ib_state* db, ib_tpl tpl, ib_ulint col_no, const void* src, ib_ulint len);

INNODB_API ib_async<void>           ib_cursor_attach_trx(ib_state* db, ib_crsr crsr, ib_trx trx);
INNODB_API ib_async<ib_err>         ib_cursor_open_table(ib_state* db, const char* name, ib_trx trx, ib_crsr* crsr);
INNODB_API ib_async<ib_err>         ib_cursor_close(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_cursor_delete_row(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_cursor_first(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_cursor_insert_row(ib_state* db, ib_crsr crsr, const ib_tpl tpl);
INNODB_API ib_async<ib_bool>        ib_cursor_is_positioned(ib_state* db, const ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_cursor_last(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_cursor_lock(ib_state* db, ib_crsr crsr, ib_lck_mode mode);
INNODB_API ib_async<ib_err>         ib_cursor_moveto(ib_state* db, ib_crsr crsr, ib_tpl tpl, ib_srch_mode ib_srch_mode, int* result);
INNODB_API ib_async<ib_err>         ib_cursor_next(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_cursor_open_index_using_id(ib_state* db, ib_id index_id, ib_trx trx, ib_crsr* out_crsr);
INNODB_API ib_async<ib_err>         ib_cursor_open_index_using_name(ib_state* db, ib_crsr open_crsr, const char* index_name, ib_crsr* out_crsr);
INNODB_API ib_async<ib_err>         ib_cursor_open_table_using_id(ib_state* db, ib_id table_id, ib_trx trx, ib_crsr* out_crsr);
INNODB_API ib_async<ib_err>         ib_cursor_prev(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_cursor_read_row(ib_state* db, ib_crsr crsr, ib_tpl tpl);
INNODB_API ib_async<ib_err>         ib_cursor_reset(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<void>           ib_cursor_set_cluster_access(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<void>           ib_cursor_set_lock_mode(ib_state* db, ib_crsr crsr, ib_lck_mode mode);
INNODB_API ib_async<void>           ib_cursor_set_match_mode(ib_state* db, ib_crsr crsr, ib_match_mode match_mode);
INNODB_API ib_async<void>           ib_cursor_set_simple_select(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<void>           ib_cursor_stmt_begin(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_cursor_truncate(ib_state* db, ib_crsr* crsr, ib_id* table_id);
INNODB_API ib_async<ib_err>         ib_cursor_update_row(ib_state* db, ib_crsr crsr, const ib_tpl ib_old_tpl, const ib_tpl ib_new_tpl);

INNODB_API ib_async<ib_err>         ib_get_duplicate_key(ib_state* db, ib_trx trx, const char **table_name, const char **index_name);
INNODB_API ib_async<ib_err>         ib_get_index_stat_n_diff_key_vals(ib_state* db, ib_crsr crsr, const char* index_name, ib_u64 *ncols, ib_i64 **n_diff);
INNODB_API ib_async<ib_err>         ib_get_table_statistics(ib_state* db, ib_crsr crsr, ib_table_stats *table_stats, size_t sizeof_ib_table_stats_t);

INNODB_API ib_async<ib_err>         ib_index_create(ib_state* db, ib_idx_sch idx_sch, ib_id* index_id);
INNODB_API ib_async<ib_err>         ib_index_drop(ib_state* db, ib_trx trx, ib_id index_id);
INNODB_API ib_async<ib_err>         ib_index_get_id(ib_state* db, const char* table_name, const char* index_name, ib_id* index_id);
INNODB_API ib_async<ib_err>         ib_index_schema_add_col(ib_state* db, ib_idx_sch idx_sch, const char* name, ib_ulint prefix_len);
INNODB_API ib_async<ib_err>         ib_index_schema_create(ib_state* db, ib_trx usr_trx, const char* name, const char* table_name, ib_idx_sch* idx_sch);
INNODB_API ib_async<void>           ib_index_schema_delete(ib_state* db, ib_idx_sch idx_sch);
INNODB_API ib_async<ib_err>         ib_index_schema_set_clustered(ib_state* db, ib_idx_sch idx_sch);
INNODB_API ib_async<ib_err>         ib_index_schema_set_unique(ib_state* db, ib_idx_sch idx_sch);

INNODB_API ib_async<ib_err>         ib_savepoint_release(ib_state* db, ib_trx trx, const void* name, ib_ulint len);
INNODB_API ib_async<ib_err>         ib_savepoint_rollback(ib_state* db, ib_trx trx, const void* name, ib_ulint len);
INNODB_API ib_async<void>           ib_savepoint_take(ib_state* db, ib_trx trx, const void* name, ib_ulint len);

INNODB_API ib_async<ib_err>         ib_schema_lock_exclusive(ib_state* db, ib_trx trx);
INNODB_API ib_async<ib_bool>        ib_schema_lock_is_exclusive(ib_state* db, const ib_trx trx);
INNODB_API ib_async<ib_bool>        ib_schema_lock_is_shared(ib_state* db, const ib_trx trx);
INNODB_API ib_async<ib_err>         ib_schema_lock_shared(ib_state* db, ib_trx trx);
INNODB_API ib_async<ib_err>         ib_schema_tables_iterate(ib_state* db, ib_trx trx, ib_schema_visitor_table_all visitor, void* arg);
INNODB_API ib_async<ib_err>         ib_schema_unlock(ib_state* db, ib_trx trx);

INNODB_API ib_async<ib_err>         ib_table_create(ib_state* db, ib_trx trx, const ib_tbl_sch sch, ib_id* id);
INNODB_API ib_async<ib_err>         ib_table_rename(ib_state* db, ib_trx trx, const char* old_name, const char* new_name);
INNODB_API ib_async<ib_err>         ib_table_drop(ib_state* db, ib_trx trx, const char* name);
INNODB_API ib_async<ib_err>         ib_table_get_id(ib_state* db, const char* table_name, ib_id* table_id);
INNODB_API ib_async<ib_err>         ib_table_lock(ib_state* db, ib_trx trx, ib_id table_id, ib_lck_mode mode);
INNODB_API ib_async<ib_err>         ib_table_truncate(ib_state* db, const char* table_name, ib_id* table_id);

INNODB_API ib_async<ib_err>         ib_table_schema_create(ib_state* db, const char* name, ib_tbl_sch* tbl_sch, ib_tbl_fmt tbl_fmt, ib_ulint page_size);
INNODB_API ib_async<ib_err>         ib_table_schema_add_col(ib_state* db, ib_tbl_sch tbl_sch, const char* name, ib_col_type col_type, ib_col_attr col_attr, ib_u16 client_type, ib_ulint len);
INNODB_API ib_async<ib_err>         ib_table_schema_add_index(ib_state* db, ib_tbl_sch tbl_sch, const char* name, ib_idx_sch* idx_sch);
INNODB_API ib_async<void>           ib_table_schema_delete(ib_state* db, ib_tbl_sch tbl_sch);
INNODB_API ib_async<ib_err>         ib_table_schema_visit(ib_state* db, ib_trx trx, const char* name, const ib_schema_visitor* visitor, void* arg);

INNODB_API ib_async<ib_err>         ib_trx_begin(ib_state* db, ib_trx_level ib_trx_level);
INNODB_API ib_async<ib_err>         ib_trx_commit(ib_state* db, ib_trx trx);
INNODB_API ib_async<void>           ib_trx_set_client_data(ib_state* db, ib_trx trx, void* client_data);
INNODB_API ib_async<ib_err>         ib_trx_release(ib_state* db, ib_trx trx);
INNODB_API ib_async<ib_err>         ib_trx_rollback(ib_state* db, ib_trx trx);
INNODB_API ib_async<ib_err>         ib_trx_start(ib_state* db, ib_trx trx, ib_trx_level ib_trx_level);
INNODB_API ib_async<ib_trx_state>   ib_trx_state(ib_state* db, ib_trx trx);

INNODB_API ib_async<ib_err>         ib_tuple_copy(ib_state* db, ib_tpl dst, const ib_tpl src);
INNODB_API ib_async<ib_tpl>         ib_tuple_clear(ib_state* db, ib_tpl tpl);
INNODB_API ib_async<void>           ib_tuple_delete(ib_state* db, ib_tpl tpl);
INNODB_API ib_async<ib_ulint>       ib_tuple_get_n_cols(ib_state* db, const ib_tpl tpl);
INNODB_API ib_async<ib_ulint>       ib_tuple_get_n_user_cols(ib_state* db, const ib_tpl tpl);
INNODB_API ib_async<ib_err>         ib_tuple_get_cluster_key(ib_state* db, ib_crsr crsr, ib_tpl* dst, const ib_tpl src);
INNODB_API ib_async<ib_err>         ib_tuple_read_double(ib_state* db, ib_tpl tpl, ib_ulint col_no, double* dval);
INNODB_API ib_async<ib_err>         ib_tuple_read_float(ib_state* db, ib_tpl tpl, ib_ulint col_no, float* fval);
INNODB_API ib_async<ib_err>         ib_tuple_read_i8(ib_state* db, ib_tpl tpl, ib_ulint i, ib_i8* ival);
INNODB_API ib_async<ib_err>         ib_tuple_read_i16(ib_state* db, ib_tpl tpl, ib_ulint i, ib_i16* ival);
INNODB_API ib_async<ib_err>         ib_tuple_read_i32(ib_state* db, ib_tpl tpl, ib_ulint i, ib_i32* ival);
INNODB_API ib_async<ib_err>         ib_tuple_read_i64(ib_state* db, ib_tpl tpl, ib_ulint i, ib_i64* ival);
INNODB_API ib_async<ib_err>         ib_tuple_read_u8(ib_state* db, ib_tpl tpl, ib_ulint i, ib_u8* ival);
INNODB_API ib_async<ib_err>         ib_tuple_read_u16(ib_state* db, ib_tpl tpl, ib_ulint i, ib_u16* ival);
INNODB_API ib_async<ib_err>         ib_tuple_read_u32(ib_state* db, ib_tpl tpl, ib_ulint i, ib_u32* ival);
INNODB_API ib_async<ib_err>         ib_tuple_read_u64(ib_state* db, ib_tpl tpl, ib_ulint i, ib_u64* ival);
INNODB_API ib_async<ib_err>         ib_tuple_write_double(ib_state* db, ib_tpl tpl, int col_no, double val);
INNODB_API ib_async<ib_err>         ib_tuple_write_float(ib_state* db, ib_tpl tpl, int col_no, float val);
INNODB_API ib_async<ib_err>         ib_tuple_write_i8(ib_state* db, ib_tpl tpl, int col_no, ib_i8 val);
INNODB_API ib_async<ib_err>         ib_tuple_write_i16(ib_state* db, ib_tpl tpl, int col_no, ib_i16 val);
INNODB_API ib_async<ib_err>         ib_tuple_write_i32(ib_state* db, ib_tpl tpl, int col_no, ib_i32 val);
INNODB_API ib_async<ib_err>         ib_tuple_write_i64(ib_state* db, ib_tpl tpl, int col_no, ib_i64 val);
INNODB_API ib_async<ib_err>         ib_tuple_write_u8(ib_state* db, ib_tpl tpl, int col_no, ib_u8 val);
INNODB_API ib_async<ib_err>         ib_tuple_write_u16(ib_state* db, ib_tpl tpl, int col_no, ib_u16 val);
INNODB_API ib_async<ib_err>         ib_tuple_write_u32(ib_state* db, ib_tpl tpl, int col_no, ib_u32 val);
INNODB_API ib_async<ib_err>         ib_tuple_write_u64(ib_state* db, ib_tpl tpl, int col_no, ib_u64 val);

INNODB_API ib_async<ib_tpl>         ib_sec_read_tuple_create(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_tpl>         ib_sec_search_tuple_create(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<ib_err>         ib_update_table_statistics(ib_state* db, ib_crsr crsr);
INNODB_API ib_async<const char*>    ib_strerror(ib_state* db, ib_err db_errno);
INNODB_API ib_async<void>           ib_set_client_compare(ib_state* db, ib_client_cmp client_cmp_func);
INNODB_API ib_async<void>           ib_set_panic_handler(ib_state* db, ib_panic_handler handler);
INNODB_API ib_async<void>           ib_set_trx_is_interrupted_handler(ib_state* db, ib_trx_is_interrupted_handler handler);
INNODB_API ib_async<void>           ib_logger_set(ib_state* db, ib_msg_log ib_msg_log, ib_msg_stream ib_msg_stream);

INNODB_API ib_async<ib_err>         ib_status_get_all(ib_state* db, const char*** names, ib_u32* names_num);
INNODB_API ib_async<ib_err>         ib_status_get_i64(ib_state* db, const char* name, ib_i64* dst);
INNODB_API ib_async<ib_err>         ib_error_inject(ib_state* db, int err);
