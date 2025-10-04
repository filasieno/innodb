# Copyright (C) 2008, 2010 Oracle/Innobase Oy
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

ACLOCAL_AMFLAGS = -I m4 --force


include_HEADERS = haildb.hpp

libhaildb_la_LDFLAGS	= -version-info $(IB_API_VERSION) $(LD_VERSION_SCRIPT)

libhaildb_la_CFLAGS= \
        ${AM_CFLAGS} \
        ${CFLAG_VISIBILITY} \
        -DBUILDING_HAILDB

noinst_HEADERS 		=						\
			include/btr_btr.hpp include/btr_btr.inl		\
			include/btr_cur.hpp include/btr_cur.inl		\
			include/btr_pcur.hpp include/btr_pcur.inl		\
			include/btr_sea.hpp include/btr_sea.inl		\
			include/btr_types.hpp 				\
			include/buf_buf.hpp				\
			include/buf_buf.inl include/buf_flu.hpp		\
			include/buf_flu.inl include/buf_lru.hpp		\
			include/buf_lru.inl include/buf_rea.hpp		\
			include/buf_types.hpp include/data_data.hpp		\
			include/data_data.inl include/data_type.hpp	\
			include/data_type.inl include/data_types.hpp	\
			include/db_err.hpp include/dict_boot.hpp		\
			include/dict_boot.inl include/dict_crea.hpp	\
			include/dict_crea.inl include/dict_dict.hpp	\
			include/dict_dict.inl include/dict_load.hpp	\
			include/dict_load.inl include/dict_mem.hpp		\
			include/dict_mem.inl include/dict_types.hpp	\
			include/dyn_dyn.hpp include/dyn_dyn.inl		\
			include/eval_eval.hpp include/eval_eval.inl	\
			include/eval_proc.hpp include/eval_proc.inl	\
			include/fil_fil.hpp include/fsp_fsp.hpp		\
			include/fsp_fsp.inl include/fut_fut.hpp		\
			include/fsp_types.hpp				\
			include/fut_fut.inl include/fut_lst.hpp		\
			include/fut_lst.inl include/ha_ha.hpp		\
			include/ha_ha.inl				\
			include/ha_storage.hpp				\
			include/ha_storage.inl				\
			include/hash_hash.hpp				\
			include/hash_hash.inl include/ibuf_ibuf.hpp	\
			include/ibuf_ibuf.inl include/ibuf_types.hpp	\
			include/lock_iter.hpp				\
			include/lock_lock.hpp include/lock_lock.inl	\
			include/lock_priv.hpp include/lock_priv.inl	\
			include/lock_types.hpp include/log_log.hpp		\
			include/log_log.inl include/log_recv.hpp		\
			include/log_recv.inl include/mach_data.hpp		\
			include/mach_data.inl include/mem_dbg.hpp		\
			include/mem_dbg.inl 				\
			include/mem_mem.hpp include/mem_mem.inl		\
			include/mtr_log.hpp include/mtr_log.inl		\
			include/mtr_mtr.hpp include/mtr_mtr.inl		\
			include/mtr_types.hpp				\
			include/os_file.hpp				\
			include/os_proc.hpp include/os_proc.inl		\
			include/os_sync.hpp include/os_sync.inl		\
			include/os_thread.hpp include/os_thread.inl	\
			include/page_cur.hpp include/page_cur.inl		\
			include/page_page.hpp include/page_page.inl	\
			include/page_types.hpp include/pars_grm.hpp		\
			include/pars_opt.hpp include/pars_opt.inl		\
			include/pars_pars.hpp include/pars_pars.inl	\
			include/pars_sym.hpp include/pars_sym.inl		\
			include/pars_types.hpp include/que_que.hpp		\
			include/que_que.inl include/que_types.hpp		\
			include/read_read.hpp include/read_read.inl	\
			include/read_types.hpp include/rem_cmp.hpp		\
			include/rem_cmp.inl include/rem_rec.hpp		\
			include/rem_rec.inl include/rem_types.hpp		\
			include/row_ext.hpp include/row_ext.inl		\
			include/row_ins.hpp include/row_ins.inl		\
			include/row_merge.hpp				\
			include/row_purge.hpp include/row_purge.inl	\
			include/row_row.hpp include/row_row.inl		\
			include/row_sel.hpp include/row_sel.inl		\
			include/row_types.hpp include/row_uins.hpp		\
			include/row_uins.inl include/row_umod.hpp		\
			include/row_umod.inl include/row_undo.hpp		\
			include/row_undo.inl include/row_upd.hpp		\
			include/row_upd.inl include/row_vers.hpp		\
			include/row_vers.inl include/srv_que.hpp		\
			include/row_prebuilt.hpp				\
			include/srv_srv.hpp include/srv_srv.inl		\
			include/srv_start.hpp include/sync_arr.hpp		\
			include/sync_arr.inl include/sync_rw.hpp		\
			include/sync_rw.inl include/sync_sync.hpp		\
			include/sync_sync.inl include/sync_types.hpp	\
			include/thr_loc.hpp include/thr_loc.inl		\
			include/trx_purge.hpp include/trx_purge.inl	\
			include/trx_rec.hpp include/trx_rec.inl		\
			include/trx_roll.hpp include/trx_roll.inl		\
			include/trx_rseg.hpp include/trx_rseg.inl		\
			include/trx_sys.hpp include/trx_sys.inl		\
			include/trx_trx.hpp include/trx_trx.inl		\
			include/trx_types.hpp include/trx_undo.hpp		\
			include/trx_undo.inl include/trx_xa.hpp		\
			include/univ.i include/usr_sess.hpp		\
			include/usr_sess.inl include/usr_types.hpp		\
			include/ut_byte.hpp include/ut_byte.inl		\
			include/ut_dbg.hpp include/ut_lst.hpp		\
			include/ut_mem.hpp include/ut_mem.inl		\
			include/ut_rnd.hpp include/ut_rnd.inl		\
			include/ut_sort.hpp include/ut_ut.hpp		\
			include/ut_ut.inl include/ut_vec.hpp		\
			include/ut_vec.inl include/ut_list.hpp		\
			include/ut_list.inl				\
			include/ut_rbt.hpp				\
			include/ddl_ddl.hpp				\
			include/api_misc.hpp include/api_ucode.hpp		\
			include/api_api.hpp \
                        tests/ib_mt_base.hpp \
			tests/ib_mt_drv.hpp \
			tests/test0aux.hpp
if WITH_ZIP
noinst_HEADERS		+= 						\
			include/buf_buddy.hpp include/buf_buddy.inl	\
			include/page_zip.hpp include/page_zip.inl
endif

lib_LTLIBRARIES		= libhaildb.la

libhaildb_la_SOURCES	=						\
			btr/btr_btr.cpp btr/btr_cur.cpp btr/btr_pcur.cpp	\
			btr/btr_sea.cpp 					\
			buf/buf_buf.cpp buf/buf_flu.cpp			\
			buf/buf_lru.cpp buf/buf_rea.cpp data/data_data.cpp	\
			data/data_type.cpp dict/dict_boot.cpp		\
			dict/dict_crea.cpp dict/dict_dict.cpp		\
			dict/dict_load.cpp dict/dict_mem.cpp dyn/dyn_dyn.cpp	\
			eval/eval_eval.cpp eval/eval_proc.cpp		\
			fil/fil_fil.cpp fsp/fsp_fsp.cpp fut/fut_fut.cpp	\
			fut/fut_lst.cpp ha/ha_ha.cpp			\
			ha/ha_storage.cpp					\
			ha/hash_hash.cpp					\
			ibuf/ibuf_ibuf.cpp lock/lock_iter.cpp		\
			lock/lock_lock.cpp				\
			log/log_log.cpp log/log_recv.cpp mach/mach_data.cpp	\
			mem/mem_mem.cpp mtr/mtr_log.cpp			\
			mtr/mtr_mtr.cpp os/os_file.cpp os/os_proc.cpp		\
			os/os_sync.cpp os/os_thread.cpp page/page_cur.cpp	\
			page/page_page.cpp 				\
			pars/lexyy.cpp pars/pars0grm.y			\
			pars/pars_opt.cpp pars/pars_pars.cpp		\
			pars/pars_sym.cpp que/que_que.cpp read/read_read.cpp	\
			rem/rem_cmp.cpp rem/rem_rec.cpp row/row_ext.cpp	\
			row/row_ins.cpp row/row_merge.cpp			\
			row/row_purge.cpp row/row_row.cpp			\
			row/row_sel.cpp row/row_uins.cpp row/row_umod.cpp	\
			row/row_undo.cpp row/row_upd.cpp row/row_vers.cpp	\
			row/row_prebuilt.cpp				\
			srv/srv_que.cpp srv/srv_srv.cpp srv/srv_start.cpp	\
			sync/sync_arr.cpp sync/sync_rw.cpp			\
			sync/sync_sync.cpp thr/thr_loc.cpp			\
			trx/trx_purge.cpp					\
			trx/trx_rec.cpp trx/trx_roll.cpp trx/trx_rseg.cpp	\
			trx/trx_sys.cpp trx/trx_trx.cpp trx/trx_undo.cpp	\
			usr/usr_sess.cpp ut/ut_byte.cpp ut/ut_dbg.cpp		\
			ut/ut_list.cpp ut/ut_mem.cpp ut/ut_rnd.cpp		\
			ut/ut_ut.cpp ut/ut_vec.cpp				\
			ut/ut_rbt.cpp					\
			api/api_api.cpp api/api_ucode.cpp ddl/ddl_ddl.cpp	\
			api/api_misc.cpp api/api_cfg.cpp api/api_status.cpp	\
			api/api_sql.cpp
if WITH_ZIP
libhaildb_la_SOURCES	+= buf/buf_buddy.cpp page/page_zip.cpp
endif

EXTRA_DIST= \
	    .quickly \
	    config/autorun.sh \
	    config/pandora-plugin \
	    config/uncrustify.cfg \
	    CMakeLists.txt \
	    COPYING \
	    COPYING.Google \
	    COPYING.Percona \
	    COPYING.Sun_Microsystems \
	    config.h.cmake \
	    libhaildb.ver \
	    ${top_srcdir}/m4/*m4 \
	    pars/make_bison.sh \
	    pars/make_flex.sh \
	    pars/pars0grm.y \
	    pars/pars0lex.l \
	    tests/CMakeLists.examples \
	    tests/CMakeLists.txt \
	    tests/Makefile.examples \
	    tests/README \
	    tests/ib_compressed.cpp \
	    tests/ib_deadlock.cpp \
	    tests/ib_index.cpp \
	    tests/ib_mt_stress.cpp \
	    tests/ib_perf1.cpp \
	    tests/ib_recover.cpp \
	    tests/ib_search.cpp \
	    tests/ib_zip.cpp \
	    tests/run.sh \
	    win/innodb.def

DISTCLEANFILES= \
		config/top.h \
		tests/ibdata1 \
		tests/log/ib_logfile1 \
		tests/log/ib_logfile0

# keep the following lists alphabetically sorted

TEST_EXECUTABLES=\
	tests/bug579934_open_index_by_name_segv \
	tests/ib_cfg \
	tests/ib_cursor \
	tests/ib_ddl \
	tests/ib_dict \
	tests/ib_drop \
	tests/ib_duplicate_key_name \
	tests/ib_logger \
	tests/ib_mt_drv \
	tests/ib_shutdown \
	tests/ib_status \
	tests/ib_panic \
	tests/ib_trx_is_interrupted \
	tests/ib_tablename \
	tests/ib_table_statistics \
	tests/ib_test1 \
	tests/ib_client_compare \
	tests/ib_test2 \
	tests/ib_test3 \
	tests/ib_test5 \
	tests/ib_types \
	tests/ib_update

TESTS_ENVIRONMENT= ${top_srcdir}/tests/run.sh
LDADD = libhaildb.la libtest0aux.la

check_PROGRAMS= ${TEST_EXECUTABLES}
check_LTLIBRARIES=  libtest0aux.la
tests_ib_mt_drv_SOURCES= \
			 tests/ib_mt_base.cpp \
			 tests/ib_mt_drv.cpp \
			 tests/ib_mt_t1.cpp \
			 tests/ib_mt_t2.cpp
libtest0aux_la_SOURCES= tests/test0aux.cpppp

TESTS= $(check_PROGRAMS)
 
TEST_STRESS_EXECUTABLES=\
	ib_mt_stress \
	ib_perf1

test: test-clean check


# stress tests, these take longer to complete
test-stress: all test-clean
	mkdir -p $(top_srcdir)/tests/t && \
	cd $(top_srcdir)/tests/t && \
	for test in $(TEST_STRESS_EXECUTABLES) ; do \
		echo "Running $${test} ..." ; \
		echo "" ; \
		../$${test} ; \
		echo "" ; \
	done

test-clean:
	cd $(top_srcdir)/tests && \
	rm -fr log/ dict_test/ t/ test/ ibdata1
