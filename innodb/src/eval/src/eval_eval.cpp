// Copyright (c) 1997, 2009, Innobase Oy. All Rights Reserved.
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

/// \file eval_eval.cpp
/// \brief SQL evaluator: evaluates simple data structures, like expressions, in a query graph
/// \details Originally created by Heikki Tuuri in 12/29/1997
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "eval_eval.hpp"

#ifdef IB_DO_NOT_INLINE
	#include "eval_eval.inl"
#endif

#include "data_data.hpp"
#include "row_sel.hpp"

// -----------------------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------------------

// The RND function seed
constinit ulint eval_rnd = 128367121;

// Dummy address used when we should allocate a buffer of size 0 in eval_node_alloc_val_buf
static byte eval_dummy;

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

/// \brief Evaluates a predefined function node where the function is not relevant in benchmarks.
/// \param [in] func_node predefined function node
static void eval_predefined_2(func_node_t* func_node);

/// \brief Evaluates a replstr-procedure node.
/// \param [in] func_node function node
static void eval_replstr(func_node_t* func_node);

/// \brief Evaluates an instr-function node.
/// \param [in] func_node function node
static void eval_instr(func_node_t* func_node);

/// \brief Evaluates a predefined function node.
/// \param [in] func_node function node
static void eval_concat(func_node_t* func_node);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Allocate a buffer from global dynamic memory for a value of a que_node.
/// \details NOTE that this memory must be explicitly freed when the query graph is freed. If the node already has an allocated buffer, that buffer is freed here. NOTE that this is the only function where dynamic memory should be allocated for a query node val field.
/// \param [in] node query graph node; sets the val field data field to point to the new buffer, and len field equal to size
/// \param [in] size buffer size
/// \return pointer to allocated buffer
IB_INTERN byte* eval_node_alloc_val_buf(que_node_t* node, ulint size)
{
	ut_ad(que_node_get_type(node) == QUE_NODE_SYMBOL || que_node_get_type(node) == QUE_NODE_FUNC);
	dfield_t* dfield = que_node_get_val(node);
	byte* data = dfield_get_data(dfield);
	if (data && data != &eval_dummy) {
		mem_free(data);
	}
	if (size == 0) {
		data = &eval_dummy;
	} else {
		data = mem_alloc(size);
	}
	que_node_set_val_buf_size(node, size);
	dfield_set_data(dfield, data, size);
	return(data);
}

/// \brief Free the buffer from global dynamic memory for a value of a que_node, if it has been allocated in the above function.
/// \details The freeing for pushed column values is done in sel_col_prefetch_buf_free.
/// \param [in] node query graph node
IB_INTERN void eval_node_free_val_buf(que_node_t* node)
{
	ut_ad(que_node_get_type(node) == QUE_NODE_SYMBOL || que_node_get_type(node) == QUE_NODE_FUNC);
	dfield_t* dfield = que_node_get_val(node);
	byte* data = dfield_get_data(dfield);
	if (que_node_get_val_buf_size(node) > 0) {
		ut_a(data);
		mem_free(data);
	}
}

/// \brief Evaluates a comparison node.
/// \param [in] cmp_node comparison node
/// \return the result of the comparison
IB_INTERN ibool eval_cmp(func_node_t* cmp_node)
{
	ut_ad(que_node_get_type(cmp_node) == QUE_NODE_FUNC);
	que_node_t* arg1 = cmp_node->args;
	que_node_t* arg2 = que_node_get_next(arg1);
	int res = cmp_dfield_dfield(NULL, que_node_get_val(arg1), que_node_get_val(arg2));
	ibool val = TRUE;
	const int func = cmp_node->func;
	switch (func) {
	case '=':           if (res !=  0) { val = FALSE; } break;
	case '<':           if (res != -1) { val = FALSE; } break;
	case '>':           if (res !=  1) { val = FALSE; } break;
	case PARS_LE_TOKEN: if (res ==  1) { val = FALSE; } break;
	case PARS_NE_TOKEN: if (res ==  0) { val = FALSE; } break;
	case PARS_GE_TOKEN: if (res == -1) { val = FALSE; } break;
	default:
		ut_ad(func == '>'); // FAIL
	}
	eval_node_set_ibool_val(cmp_node, val);
	return(val);
}

/// \brief Evaluates a logical operation node.
/// \param [in] logical_node logical operation node
IB_INLINE void eval_logical(func_node_t* logical_node)
{
	ut_ad(que_node_get_type(logical_node) == QUE_NODE_FUNC);
	que_node_t* arg1 = logical_node->args;
	que_node_t* arg2 = que_node_get_next(arg1);
	ibool val1 = eval_node_get_ibool_val(arg1);
	ibool val2 = arg2 ? eval_node_get_ibool_val(arg2) : 0;
	int func = logical_node->func;
	ibool val;
	if (func == PARS_AND_TOKEN) {
		val = val1 & val2;
	} else if (func == PARS_OR_TOKEN) {
		val = val1 | val2;
	} else if (func == PARS_NOT_TOKEN) {
		val = TRUE - val1;
	} else {
		UT_ERROR;
	}
	eval_node_set_ibool_val(logical_node, val);
}

/// \brief Evaluates an arithmetic operation node.
/// \param [in] arith_node arithmetic operation node
IB_INLINE void eval_arith(func_node_t* arith_node)
{
	ut_ad(que_node_get_type(arith_node) == QUE_NODE_FUNC);
	que_node_t* arg1 = arith_node->args;
	que_node_t* arg2 = que_node_get_next(arg1); // arg2 is NULL if func is unary '-'
	lint val1 = eval_node_get_int_val(arg1);
	lint val2 = 0;
	if (arg2) {
		val2 = eval_node_get_int_val(arg2);
	}
	int func = arith_node->func;
	lint val;
	if (func == '+') {
		val = val1 + val2;
	} else if ((func == '-') && arg2) {
		val = val1 - val2;
	} else if (func == '-') {
		val = -val1;
	} else if (func == '*') {
		val = val1 * val2;
	} else {
		ut_ad(func == '/');
		val = val1 / val2;
	}
	eval_node_set_int_val(arith_node, val);
}

/// \brief Evaluates an aggregate operation node.
/// \param [in] node aggregate operation node
IB_INLINE void eval_aggregate(func_node_t* node)
{
	ut_ad(que_node_get_type(node) == QUE_NODE_FUNC);
	lint val = eval_node_get_int_val(node);
	int func = node->func;
	if (func == PARS_COUNT_TOKEN) {
		val = val + 1;
	} else {
		ut_ad(func == PARS_SUM_TOKEN);
		que_node_t* arg = node->args;
		lint arg_val = eval_node_get_int_val(arg);
		val = val + arg_val;
	}
	eval_node_set_int_val(node, val);
}

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Evaluates a predefined function node where the function is not relevant in benchmarks.
/// \param [in] func_node predefined function node
static void eval_predefined_2(func_node_t* func_node)
{
	ut_ad(que_node_get_type(func_node) == QUE_NODE_FUNC);
	que_node_t* arg1 = func_node->args;
	que_node_t* arg2 = NULL;
	if (arg1) {
		arg2 = que_node_get_next(arg1);
	}
	int func = func_node->func;
	if (func == PARS_PRINTF_TOKEN) {
		que_node_t* arg = arg1;
		while (arg) {
			dfield_print(que_node_get_val(arg));
			arg = que_node_get_next(arg);
		}
		ib_log(state, "\n");
	} else if (func == PARS_ASSERT_TOKEN) {
		if (!eval_node_get_ibool_val(arg1)) {
			ib_log(state, "SQL assertion fails in a stored procedure!\n");
		}
		ut_a(eval_node_get_ibool_val(arg1));
		// This function, or more precisely, a debug procedure, returns no value
	} else if (func == PARS_RND_TOKEN) {
		const ulint len1 = (ulint)eval_node_get_int_val(arg1);
		const ulint len2 = (ulint)eval_node_get_int_val(arg2);
		ut_ad(len2 >= len1);
		lint int_val;
		if (len2 > len1) {
			int_val = (lint)(len1 + (eval_rnd % (len2 - len1 + 1)));
		} else {
			int_val = (lint)len1;
		}
		eval_rnd = ut_rnd_gen_next_ulint(eval_rnd);
		eval_node_set_int_val(func_node, int_val);
	} else if (func == PARS_RND_STR_TOKEN) {
		const ulint len1 = (ulint)eval_node_get_int_val(arg1);
		byte* data = eval_node_ensure_val_buf(func_node, len1);
		for (ulint i = 0; i < len1; i++) {
			data[i] = (byte)(97 + (eval_rnd % 3));
			eval_rnd = ut_rnd_gen_next_ulint(eval_rnd);
		}
	} else {
		UT_ERROR;
	}
}

/// \brief Evaluates a notfound-function node.
/// \param [in] func_node function node
IB_INLINE void eval_notfound(func_node_t* func_node)
{
	que_node_t* arg1 = func_node->args;
	que_node_t* arg2 = que_node_get_next(arg1);
	ut_ad(func_node->func == PARS_NOTFOUND_TOKEN);
	sym_node_t* cursor = arg1;
	ut_ad(que_node_get_type(cursor) == QUE_NODE_SYMBOL);
	sel_node_t* sel_node;
	if (cursor->token_type == SYM_LIT) {
		ut_ad(ut_memcmp(dfield_get_data(que_node_get_val(cursor)), "SQL", 3) == 0);
		sel_node = cursor->sym_table->query_graph->last_sel_node;
	} else {
		sel_node = cursor->alias->cursor_def;
	}
	ibool ibool_val;
	if (sel_node->state == SEL_NODE_NO_MORE_ROWS) {
		ibool_val = TRUE;
	} else {
		ibool_val = FALSE;
	}
	eval_node_set_ibool_val(func_node, ibool_val);
}

/// \brief Evaluates a substr-function node.
/// \param [in] func_node function node
IB_INLINE void eval_substr(func_node_t* func_node)
{
	que_node_t* arg1 = func_node->args;
	que_node_t* arg2 = que_node_get_next(arg1);
	ut_ad(func_node->func == PARS_SUBSTR_TOKEN);
	que_node_t* arg3 = que_node_get_next(arg2);
	byte* str1 = dfield_get_data(que_node_get_val(arg1));
	ulint len1 = (ulint)eval_node_get_int_val(arg2);
	ulint len2 = (ulint)eval_node_get_int_val(arg3);
	dfield_t* dfield = que_node_get_val(func_node);
	dfield_set_data(dfield, str1 + len1, len2);
}

/// \brief Evaluates a replstr-procedure node.
/// \param [in] func_node function node
static void eval_replstr(func_node_t* func_node)
{
	que_node_t* arg1 = func_node->args;
	que_node_t* arg2 = que_node_get_next(arg1);
	ut_ad(que_node_get_type(arg1) == QUE_NODE_SYMBOL);
	que_node_t* arg3 = que_node_get_next(arg2);
	que_node_t* arg4 = que_node_get_next(arg3);
	byte* str1 = dfield_get_data(que_node_get_val(arg1));
	byte* str2 = dfield_get_data(que_node_get_val(arg2));
	ulint len1 = (ulint)eval_node_get_int_val(arg3);
	ulint len2 = (ulint)eval_node_get_int_val(arg4);
	if ((dfield_get_len(que_node_get_val(arg1)) < len1 + len2) || (dfield_get_len(que_node_get_val(arg2)) < len2)) {
		UT_ERROR;
	}
	ut_memcpy(str1 + len1, str2, len2);
}

/// \brief Evaluates an instr-function node.
/// \param [in] func_node function node
static void eval_instr(func_node_t* func_node)
{
	que_node_t* arg1 = func_node->args;
	que_node_t* arg2 = que_node_get_next(arg1);
	dfield_t* dfield1 = que_node_get_val(arg1);
	dfield_t* dfield2 = que_node_get_val(arg2);
	byte* str1 = dfield_get_data(dfield1);
	byte* str2 = dfield_get_data(dfield2);
	const ulint len1 = dfield_get_len(dfield1);
	const ulint len2 = dfield_get_len(dfield2);
	if (len2 == 0) {
		UT_ERROR;
	}
	byte match_char = str2[0];
	lint int_val;
	for (ulint i = 0; i < len1; i++) {
		// In this outer loop, the number of matched characters is 0
		if (str1[i] == match_char) {
			if (i + len2 > len1) {
				break;
			}
			for (ulint j = 1;; j++) {
				// We have already matched j characters
				if (j == len2) {
					int_val = i + 1;
					goto match_found;
				}
				if (str1[i + j] != str2[j]) {
					break;
				}
			}
		}
	}
	int_val = 0;
match_found:
	eval_node_set_int_val(func_node, int_val);
}

/// \brief Evaluates a predefined function node.
/// \param [in] func_node function node
IB_INLINE void eval_binary_to_number(func_node_t* func_node)
{
	que_node_t* arg1 = func_node->args;
	dfield_t* dfield = que_node_get_val(arg1);
	byte* str1 = dfield_get_data(dfield);
	const ulint len1 = dfield_get_len(dfield);
	if (len1 > 4) {
		UT_ERROR;
	}
	byte* str2;
	if (len1 == 4) {
		str2 = str1;
	} else {
		ulint int_val = 0;
		str2 = (byte*)&int_val;
		ut_memcpy(str2 + (4 - len1), str1, len1);
	}
	eval_node_copy_and_alloc_val(func_node, str2, 4);
}

/// \brief Evaluates a predefined function node.
/// \param [in] func_node function node
static void eval_concat(func_node_t* func_node)
{
	que_node_t* arg = func_node->args;
	ulint len = 0;
	while (arg) {
		ulint len1 = dfield_get_len(que_node_get_val(arg));
		len += len1;
		arg = que_node_get_next(arg);
	}
	byte* data = eval_node_ensure_val_buf(func_node, len);
	arg = func_node->args;
	len = 0;
	while (arg) {
		dfield_t* dfield = que_node_get_val(arg);
		ulint len1 = dfield_get_len(dfield);
		ut_memcpy(data + len, dfield_get_data(dfield), len1);
		len += len1;
		arg = que_node_get_next(arg);
	}
}

/// \brief Evaluates a predefined function node.
/// \details If the first argument is an integer, this function looks at the second argument which is the integer length in bytes, and converts the integer to a VARCHAR. If the first argument is of some other type, this function converts it to BINARY.
/// \param [in] func_node function node
IB_INLINE void eval_to_binary(func_node_t* func_node)
{
	que_node_t* arg1 = func_node->args;
	byte* str1 = dfield_get_data(que_node_get_val(arg1));
	if (dtype_get_mtype(que_node_get_data_type(arg1)) != DATA_INT) {
		ulint len = dfield_get_len(que_node_get_val(arg1));
		dfield_t* dfield = que_node_get_val(func_node);
		dfield_set_data(dfield, str1, len);
		return;
	}
	que_node_t* arg2 = que_node_get_next(arg1);
	const ulint len1 = (ulint)eval_node_get_int_val(arg2);
	if (len1 > 4) {
		UT_ERROR;
	}
	dfield_t* dfield = que_node_get_val(func_node);
	dfield_set_data(dfield, str1 + (4 - len1), len1);
}

/// \brief Evaluates a predefined function node.
/// \param [in] func_node function node
IB_INLINE void eval_predefined(func_node_t* func_node)
{
	int func = func_node->func;
	que_node_t* arg1 = func_node->args;
	if (func == PARS_LENGTH_TOKEN) {
		const lint int_val = (lint)dfield_get_len(que_node_get_val(arg1));
		eval_node_set_int_val(func_node, int_val);
	} else if (func == PARS_TO_CHAR_TOKEN) {
		// Convert number to character string as a signed decimal integer.
		const lint int_val = eval_node_get_int_val(arg1);
		// Determine the length of the string.
		int int_len;
		if (int_val == 0) {
			int_len = 1; // the number 0 occupies 1 byte
		} else {
			int_len = 0;
			ulint uint_val;
			if (int_val < 0) {
				uint_val = ((ulint) -int_val - 1) + 1;
				int_len++; // reserve space for minus sign
			} else {
				uint_val = (ulint) int_val;
			}
			for (; uint_val > 0; int_len++) {
				uint_val /= 10;
			}
		}
		// allocate the string
		byte* data = eval_node_ensure_val_buf(func_node, int_len + 1);
		// add terminating NUL character
		data[int_len] = 0;
		// convert the number
		if (int_val == 0) {
			data[0] = '0';
		} else {
			int tmp = int_len;
			ulint uint_val;
			if (int_val < 0) {
				data[0] = '-'; // preceding minus sign
				uint_val = ((ulint) -int_val - 1) + 1;
			} else {
				uint_val = (ulint) int_val;
			}
			for (; uint_val > 0; uint_val /= 10) {
				data[--tmp] = (byte)('0' + (byte)(uint_val % 10));
			}
		}
		dfield_set_len(que_node_get_val(func_node), int_len);
		return;
	} else if (func == PARS_TO_NUMBER_TOKEN) {
		const lint int_val = atoi((char*)dfield_get_data(que_node_get_val(arg1)));
		eval_node_set_int_val(func_node, int_val);
	} else if (func == PARS_SYSDATE_TOKEN) {
		const lint int_val = (lint)ut_time();
		eval_node_set_int_val(func_node, int_val);
	} else {
		eval_predefined_2(func_node);
		return;
	}
}

/// \brief Evaluates a function node.
/// \param [in] func_node function node
IB_INTERN void eval_func(func_node_t* func_node)
{
	ut_ad(que_node_get_type(func_node) == QUE_NODE_FUNC);
	ulint klass = func_node->class;
	ulint func = func_node->func;
	que_node_t* arg = func_node->args;
	// Evaluate first the argument list
	while (arg) {
		eval_exp(arg);
		// The functions are not defined for SQL null argument values, except for eval_cmp and notfound
		if (dfield_is_null(que_node_get_val(arg)) && (klass != PARS_FUNC_CMP) && (func != PARS_NOTFOUND_TOKEN) && (func != PARS_PRINTF_TOKEN)) {
			UT_ERROR;
		}
		arg = que_node_get_next(arg);
	}

	switch (klass) {
	case PARS_FUNC_CMP:        eval_cmp(func_node); break;
	case PARS_FUNC_ARITH:      eval_arith(func_node); break;
	case PARS_FUNC_AGGREGATE:  eval_aggregate(func_node); break;
	case PARS_FUNC_AGGREGATE:  eval_aggregate(func_node); break;
	case PARS_FUNC_LOGICAL:    eval_logical(func_node); break;
	case PARS_FUNC_PREDEFINED:
		switch (func) {
		case PARS_NOTFOUND_TOKEN:         eval_notfound(func_node); break;
		case PARS_SUBSTR_TOKEN:           eval_substr(func_node); break;
		case PARS_REPLSTR_TOKEN:          eval_replstr(func_node); break;
		case PARS_INSTR_TOKEN:            eval_instr(func_node); break;
		case PARS_BINARY_TO_NUMBER_TOKEN: eval_binary_to_number(func_node); break;
		case PARS_CONCAT_TOKEN:           eval_concat(func_node); break;
		case PARS_TO_BINARY_TOKEN:        eval_to_binary(func_node); break;
		default:                          eval_predefined(func_node); break;
		}
		break;
	default:
		ut_ad(klass == PARS_FUNC_LOGICAL); // FAIL
	}
}
