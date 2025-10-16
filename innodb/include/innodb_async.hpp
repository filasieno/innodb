#pragma once

#include "innodb_types.h"

#define INNODB_API

template <typename T>
struct ib_async { };

struct innodb_state;

INNODB_API ib_async<ib_u64_t>       ib_api_version();

INNODB_API ib_i64_t                 ib_state_get_size();
INNODB_API ib_async<ib_err_t>       ib_init(innodb_state *out_db);

INNODB_API ib_async<ib_bool_t>      ib_database_create(innodb_state* db, const char* db_name);
INNODB_API ib_async<ib_err_t>       ib_database_drop(innodb_state* db, const char* db_name);
INNODB_API ib_async<ib_err_t>       ib_shutdown(innodb_state* db, ib_shutdown_t flag);
INNODB_API ib_async<ib_err_t>       ib_startup(innodb_state* db, const char* format);

INNODB_API ib_async<ib_err_t>       ib_cfg_get(innodb_state* db, const char* name, void* value);
INNODB_API ib_async<ib_err_t>       ib_cfg_get_all(innodb_state* db, const char*** names, ib_u32_t* names_num);
INNODB_API ib_async<ib_err_t>       ib_cfg_set(innodb_state* db, const char* name, ...);
INNODB_API ib_async<ib_err_t>       ib_cfg_var_get_type(innodb_state* db, const char* name, ib_cfg_type_t* type);

INNODB_API ib_async<ib_tpl_t>       ib_clust_read_tuple_create(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_tpl_t>       ib_clust_search_tuple_create(innodb_state* db, ib_crsr_t crsr);

INNODB_API ib_async<ib_ulint_t>     ib_col_copy_value(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, void* dst, ib_ulint_t len);
INNODB_API ib_async<ib_ulint_t>     ib_col_get_len(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i);
INNODB_API ib_async<ib_ulint_t>     ib_col_get_meta(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_col_meta_t* col_meta);
INNODB_API ib_async<const void*>    ib_col_get_value(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i);
INNODB_API ib_async<ib_err_t>       ib_col_set_value(innodb_state* db, ib_tpl_t tpl, ib_ulint_t col_no, const void* src, ib_ulint_t len);

INNODB_API ib_async<void>           ib_cursor_attach_trx(innodb_state* db, ib_crsr_t crsr, ib_trx_t trx);
INNODB_API ib_async<ib_err_t>       ib_cursor_open_table(innodb_state* db, const char* name, ib_trx_t trx, ib_crsr_t* crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_close(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_delete_row(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_first(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_insert_row(innodb_state* db, ib_crsr_t crsr, const ib_tpl_t tpl);
INNODB_API ib_async<ib_bool_t>      ib_cursor_is_positioned(innodb_state* db, const ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_last(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_lock(innodb_state* db, ib_crsr_t crsr, ib_lck_mode_t mode);
INNODB_API ib_async<ib_err_t>       ib_cursor_moveto(innodb_state* db, ib_crsr_t crsr, ib_tpl_t tpl, ib_srch_mode_t ib_srch_mode, int* result);
INNODB_API ib_async<ib_err_t>       ib_cursor_next(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_open_index_using_id(innodb_state* db, ib_id_t index_id, ib_trx_t trx, ib_crsr_t* crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_open_index_using_name(innodb_state* db, ib_crsr_t open_crsr, const char* index_name, ib_crsr_t* crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_open_table_using_id(innodb_state* db, ib_id_t table_id, ib_trx_t trx, ib_crsr_t* crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_prev(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_read_row(innodb_state* db, ib_crsr_t crsr, ib_tpl_t tpl);
INNODB_API ib_async<ib_err_t>       ib_cursor_reset(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<void>           ib_cursor_set_cluster_access(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<void>           ib_cursor_set_lock_mode(innodb_state* db, ib_crsr_t crsr, ib_lck_mode_t mode);
INNODB_API ib_async<void>           ib_cursor_set_match_mode(innodb_state* db, ib_crsr_t crsr, ib_match_mode_t match_mode);
INNODB_API ib_async<void>           ib_cursor_set_simple_select(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<void>           ib_cursor_stmt_begin(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_cursor_truncate(innodb_state* db, ib_crsr_t* crsr, ib_id_t* table_id);
INNODB_API ib_async<ib_err_t>       ib_cursor_update_row(innodb_state* db, ib_crsr_t crsr, const ib_tpl_t ib_old_tpl, const ib_tpl_t ib_new_tpl);

INNODB_API ib_async<ib_err_t>       ib_get_duplicate_key(innodb_state* db, ib_trx_t trx, const char **table_name, const char **index_name);
INNODB_API ib_async<ib_err_t>       ib_get_index_stat_n_diff_key_vals(innodb_state* db, ib_crsr_t crsr, const char* index_name, ib_u64_t *ncols, ib_i64_t **n_diff);
INNODB_API ib_async<ib_err_t>       ib_get_table_statistics(innodb_state* db, ib_crsr_t crsr, ib_table_stats_t *table_stats, size_t sizeof_ib_table_stats_t);

INNODB_API ib_async<ib_err_t>       ib_index_create(innodb_state* db, ib_idx_sch_t idx_sch, ib_id_t* index_id);
INNODB_API ib_async<ib_err_t>       ib_index_drop(innodb_state* db, ib_trx_t trx, ib_id_t index_id);
INNODB_API ib_async<ib_err_t>       ib_index_get_id(innodb_state* db, const char* table_name, const char* index_name, ib_id_t* index_id);
INNODB_API ib_async<ib_err_t>       ib_index_schema_add_col(innodb_state* db, ib_idx_sch_t idx_sch, const char* name, ib_ulint_t prefix_len);
INNODB_API ib_async<ib_err_t>       ib_index_schema_create(innodb_state* db, ib_trx_t usr_trx, const char* name, const char* table_name, ib_idx_sch_t* idx_sch);
INNODB_API ib_async<void>           ib_index_schema_delete(innodb_state* db, ib_idx_sch_t idx_sch);
INNODB_API ib_async<ib_err_t>       ib_index_schema_set_clustered(innodb_state* db, ib_idx_sch_t idx_sch);
INNODB_API ib_async<ib_err_t>       ib_index_schema_set_unique(innodb_state* db, ib_idx_sch_t idx_sch);

INNODB_API ib_async<ib_err_t>       ib_savepoint_release(innodb_state* db, ib_trx_t trx, const void* name, ib_ulint_t len);
INNODB_API ib_async<ib_err_t>       ib_savepoint_rollback(innodb_state* db, ib_trx_t trx, const void* name, ib_ulint_t len);
INNODB_API ib_async<void>           ib_savepoint_take(innodb_state* db, ib_trx_t trx, const void* name, ib_ulint_t len);

INNODB_API ib_async<ib_err_t>       ib_schema_lock_exclusive(innodb_state* db, ib_trx_t trx);
INNODB_API ib_async<ib_bool_t>      ib_schema_lock_is_exclusive(innodb_state* db, const ib_trx_t trx);
INNODB_API ib_async<ib_bool_t>      ib_schema_lock_is_shared(innodb_state* db, const ib_trx_t trx);
INNODB_API ib_async<ib_err_t>       ib_schema_lock_shared(innodb_state* db, ib_trx_t trx);
INNODB_API ib_async<ib_err_t>       ib_schema_tables_iterate(innodb_state* db, ib_trx_t trx, ib_schema_visitor_table_all_t visitor, void* arg);
INNODB_API ib_async<ib_err_t>       ib_schema_unlock(innodb_state* db, ib_trx_t trx);

INNODB_API ib_async<ib_err_t>       ib_table_create(innodb_state* db, ib_trx_t trx, const ib_tbl_sch_t sch, ib_id_t* id);
INNODB_API ib_async<ib_err_t>       ib_table_rename(innodb_state* db, ib_trx_t trx, const char* old_name, const char* new_name);
INNODB_API ib_async<ib_err_t>       ib_table_drop(innodb_state* db, ib_trx_t trx, const char* name);
INNODB_API ib_async<ib_err_t>       ib_table_get_id(innodb_state* db, const char* table_name, ib_id_t* table_id);
INNODB_API ib_async<ib_err_t>       ib_table_lock(innodb_state* db, ib_trx_t trx, ib_id_t table_id, ib_lck_mode_t mode);
INNODB_API ib_async<ib_err_t>       ib_table_truncate(innodb_state* db, const char* table_name, ib_id_t* table_id);

INNODB_API ib_async<ib_err_t>       ib_table_schema_create(innodb_state* db, const char* name, ib_tbl_sch_t* tbl_sch, ib_tbl_fmt_t tbl_fmt, ib_ulint_t page_size);
INNODB_API ib_async<ib_err_t>       ib_table_schema_add_col(innodb_state* db, ib_tbl_sch_t tbl_sch, const char* name, ib_col_type_t col_type, ib_col_attr_t col_attr, ib_u16_t client_type, ib_ulint_t len);
INNODB_API ib_async<ib_err_t>       ib_table_schema_add_index(innodb_state* db, ib_tbl_sch_t tbl_sch, const char* name, ib_idx_sch_t* idx_sch);
INNODB_API ib_async<void>           ib_table_schema_delete(innodb_state* db, ib_tbl_sch_t tbl_sch);
INNODB_API ib_async<ib_err_t>       ib_table_schema_visit(innodb_state* db, ib_trx_t trx, const char* name, const ib_schema_visitor_t* visitor, void* arg);

INNODB_API ib_async<ib_err_t>       ib_trx_begin(innodb_state* db, ib_trx_level_t ib_trx_level);
INNODB_API ib_async<ib_err_t>       ib_trx_commit(innodb_state* db, ib_trx_t trx);
INNODB_API ib_async<void>           ib_trx_set_client_data(innodb_state* db, ib_trx_t trx, void* client_data);
INNODB_API ib_async<ib_err_t>       ib_trx_release(innodb_state* db, ib_trx_t trx);
INNODB_API ib_async<ib_err_t>       ib_trx_rollback(innodb_state* db, ib_trx_t trx);
INNODB_API ib_async<ib_err_t>       ib_trx_start(innodb_state* db, ib_trx_t trx, ib_trx_level_t ib_trx_level);
INNODB_API ib_async<ib_trx_state_t> ib_trx_state(innodb_state* db, ib_trx_t trx);

INNODB_API ib_async<ib_err_t>       ib_tuple_copy(innodb_state* db, ib_tpl_t dst, const ib_tpl_t src);
INNODB_API ib_async<ib_tpl_t>       ib_tuple_clear(innodb_state* db, ib_tpl_t tpl);
INNODB_API ib_async<void>           ib_tuple_delete(innodb_state* db, ib_tpl_t tpl);
INNODB_API ib_async<ib_ulint_t>     ib_tuple_get_n_cols(innodb_state* db, const ib_tpl_t tpl);
INNODB_API ib_async<ib_ulint_t>     ib_tuple_get_n_user_cols(innodb_state* db, const ib_tpl_t tpl);
INNODB_API ib_async<ib_err_t>       ib_tuple_get_cluster_key(innodb_state* db, ib_crsr_t crsr, ib_tpl_t* dst, const ib_tpl_t src);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_double(innodb_state* db, ib_tpl_t tpl, ib_ulint_t col_no, double* dval);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_float(innodb_state* db, ib_tpl_t tpl, ib_ulint_t col_no, float* fval);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_i8(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_i8_t* ival);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_i16(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_i16_t* ival);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_i32(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_i32_t* ival);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_i64(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_i64_t* ival);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_u8(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_u8_t* ival);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_u16(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_u16_t* ival);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_u32(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_u32_t* ival);
INNODB_API ib_async<ib_err_t>       ib_tuple_read_u64(innodb_state* db, ib_tpl_t tpl, ib_ulint_t i, ib_u64_t* ival);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_double(innodb_state* db, ib_tpl_t tpl, int col_no, double val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_float(innodb_state* db, ib_tpl_t tpl, int col_no, float val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_i8(innodb_state* db, ib_tpl_t tpl, int col_no, ib_i8_t val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_i16(innodb_state* db, ib_tpl_t tpl, int col_no, ib_i16_t val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_i32(innodb_state* db, ib_tpl_t tpl, int col_no, ib_i32_t val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_i64(innodb_state* db, ib_tpl_t tpl, int col_no, ib_i64_t val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_u8(innodb_state* db, ib_tpl_t tpl, int col_no, ib_u8_t val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_u16(innodb_state* db, ib_tpl_t tpl, int col_no, ib_u16_t val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_u32(innodb_state* db, ib_tpl_t tpl, int col_no, ib_u32_t val);
INNODB_API ib_async<ib_err_t>       ib_tuple_write_u64(innodb_state* db, ib_tpl_t tpl, int col_no, ib_u64_t val);

INNODB_API ib_async<ib_tpl_t>       ib_sec_read_tuple_create(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_tpl_t>       ib_sec_search_tuple_create(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<ib_err_t>       ib_update_table_statistics(innodb_state* db, ib_crsr_t crsr);
INNODB_API ib_async<const char*>    ib_strerror(innodb_state* db, ib_err_t db_errno);
INNODB_API ib_async<void>           ib_set_client_compare(innodb_state* db, ib_client_cmp_t client_cmp_func);
INNODB_API ib_async<void>           ib_set_panic_handler(innodb_state* db, ib_panic_handler_t handler);
INNODB_API ib_async<void>           ib_set_trx_is_interrupted_handler(innodb_state* db, ib_trx_is_interrupted_handler_t handler);
INNODB_API ib_async<void>           ib_logger_set(innodb_state* db, ib_msg_log_t ib_msg_log, ib_msg_stream_t ib_msg_stream);


INNODB_API ib_async<ib_err_t>       ib_status_get_all(innodb_state* db, const char*** names, ib_u32_t* names_num);
INNODB_API ib_async<ib_err_t>       ib_status_get_i64(innodb_state* db, const char* name, ib_i64_t* dst);
INNODB_API ib_async<ib_err_t>       ib_error_inject(innodb_state* db, int err);
