// Copyright (c) 1994, 2025, Innobase Oy. All Rights Reserved.
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

/// \file data_data.cpp
/// \brief SQL data field and tuple
/// \details Originally created by Heikki Tuuri in 5/30/1994
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "data_data.hpp"

#ifdef IB_DO_NOT_INLINE
	#include "data_data.inl"
#endif

#ifndef IB_HOTBACKUP
	#include <ctype.h>
	#include "rem_rec.hpp"
	#include "rem_cmp.hpp"
	#include "page_page.hpp"
	#include "page_zip.hpp"
	#include "dict_dict.hpp"
	#include "btr_cur.hpp"
#endif // !IB_HOTBACKUP 

// -----------------------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------------------

#ifdef IB_DEBUG
	/// \brief Dummy variable to catch access to uninitialized fields. 
	/// \details In the debug version, dtuple_create() will make all fields of dtuple_t point to data_error.
	IB_INTERN byte data_error;

	#ifndef IB_DEBUG_VALGRIND
	
		/// \brief used to fool the compiler in dtuple_validate
		IB_INTERN ulint data_dummy;
	
	#endif // !IB_DEBUG_VALGRIND

#endif // IB_DEBUG 

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

static ibool dfield_check_typed_no_assert(const dfield_t* field);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

#ifndef IB_HOTBACKUP

/// \brief Reset dfield variables.
IB_INTERN void dfield_var_init(void)
{
#ifdef IB_DEBUG
	data_error = 0;
#ifndef IB_DEBUG_VALGRIND
	data_dummy = 0;
#endif // !IB_DEBUG_VALGRIND
#endif // IB_DEBUG
}

/// \brief Tests if dfield data length and content is equal to the given.
/// \param [in] field
/// \param [in] len data length or IB_SQL_NULL
/// \param [in] data
/// \return TRUE if equal
IB_INTERN ibool dfield_data_is_binary_equal(const dfield_t* field, ulint len, const byte* data)
{
	if (len != dfield_get_len(field)) {
		return FALSE;
	}
	if (len == IB_SQL_NULL) {
		return TRUE;
	}
	if (memcmp(dfield_get_data(field), data, len) != 0) {
		return FALSE;
	}
	return TRUE;
}

/// \brief Compare two data tuples, respecting the collation of character fields.
/// \param [in] cmp_ctx client compare context
/// \param [in] tuple1 tuple 1
/// \param [in] tuple2 tuple 2
/// \return 1, 0 , -1 if tuple1 is greater, equal, less, respectively, than tuple2
IB_INTERN int dtuple_coll_cmp(void* cmp_ctx, const dtuple_t* tuple1, const dtuple_t* tuple2)
{
	ut_ad(tuple1);
	ut_ad(tuple1->magic_n == DATA_TUPLE_MAGIC_N);
	ut_ad(dtuple_check_typed(tuple1));

	ut_ad(tuple2);
	ut_ad(tuple2->magic_n == DATA_TUPLE_MAGIC_N);	
	ut_ad(dtuple_check_typed(tuple2));

	ulint n_fields = dtuple_get_n_fields(tuple1);
	if (n_fields != dtuple_get_n_fields(tuple2)) {
		return n_fields < dtuple_get_n_fields(tuple2) ? -1 : 1;
	}

	for (ulint i = 0; i < n_fields; i++) {
		const dfield_t* field1 = dtuple_get_nth_field(tuple1, i);
		const dfield_t* field2 = dtuple_get_nth_field(tuple2, i);
		int cmp = cmp_dfield_dfield(cmp_ctx, field1, field2);
		if (cmp) {
			return cmp;
		}
	}

	return 0;
}

/// \brief Sets number of fields used in a tuple.
/// \param [in] tuple
/// \param [in] n_fields
/// \details Normally this is set in dtuple_create, but if you want later to set it smaller, you can use this.
IB_INTERN void dtuple_set_n_fields(dtuple_t* tuple, ulint n_fields)
{
	ut_ad(tuple);

	tuple->n_fields = n_fields;
	tuple->n_fields_cmp = n_fields;
}

/// \brief Checks that a data tuple is typed.
/// \param [in] tuple
/// \return TRUE if ok
IB_INTERN ibool dtuple_check_typed_no_assert(const dtuple_t* tuple)
{
	if (dtuple_get_n_fields(tuple) > REC_MAX_N_FIELDS) {
		ib_log(state, "InnoDB: Error: index entry has %lu fields\n", (ulong) dtuple_get_n_fields(tuple));
		ib_log(state, "InnoDB: Tuple contents: ");
		dtuple_print(state->stream, tuple);
		ib_log(state,"\n");
		return FALSE;
	}

	for (ulint i = 0; i < dtuple_get_n_fields(tuple); i++) {
		const dfield_t* field = dtuple_get_nth_field(tuple, i);
		if (!dfield_check_typed_no_assert(field)) {
			ib_log(state, "InnoDB: Tuple contents: ");
			dtuple_print(state->stream, tuple);
			ib_log(state,"\n");
			UT_ERROR;
			return FALSE;
		}
	}

	return TRUE;
}
#endif // !IB_HOTBACKUP

#ifdef IB_DEBUG

/// \brief Checks that a data field is typed.
/// \param [in] field
/// \return TRUE if ok
/// \details Asserts an error if not.
IB_INTERN ibool dfield_check_typed(const innodb_state* state, const dfield_t* field)
{
	if (dfield_get_type(field)->mtype > DATA_CLIENT || dfield_get_type(field)->mtype < DATA_VARCHAR) {
		ib_log(state, "InnoDB: Error: data field type %lu, len %lu\n", (ulong) dfield_get_type(field)->mtype, (ulong) dfield_get_len(field));
		UT_ERROR;
	}

	return TRUE;
}

/// \brief Checks that a data tuple is typed.
/// \param [in] tuple
/// \return TRUE if ok
/// \details Asserts an error if not.
IB_INTERN ibool dtuple_check_typed(const innodb_state* state, const dtuple_t* tuple)
{
	for (ulint i = 0; i < dtuple_get_n_fields(tuple); i++) {
		const dfield_t* field = dtuple_get_nth_field(tuple, i);
		ut_a(dfield_check_typed(state,field));
	}
	return TRUE;
}

/// \brief Validates the consistency of a tuple which must be complete.
/// \param [in] tuple
/// \return TRUE if ok
/// \details All fields must have been set.
IB_INTERN ibool dtuple_validate(const dtuple_t* tuple)
{
	ut_ad(tuple->magic_n == DATA_TUPLE_MAGIC_N);
	ulint n_fields = dtuple_get_n_fields(tuple);
	// We dereference all the data of each field to test for memory traps 
	for (ulint i = 0; i < n_fields; i++) {
		const dfield_t* field = dtuple_get_nth_field(tuple, i);
		ulint len = dfield_get_len(field);
	 	if (!dfield_is_null(field)) {
			const byte* data = dfield_get_data(field);
			if constexpr (IB_DEBUG_VALGRIND) {
				for (ulint j = 0; j < len; j++) {
					data_dummy += *data; // fool the compiler not to optimize out this code
					data++;
				}
			}
		}
		IB_MEM_ASSERT_RW(data, len);
	}
	ut_a(dtuple_check_typed(tuple));
	return TRUE;
}

#endif // IB_DEBUG

#ifndef IB_HOTBACKUP

/// \brief Pretty prints a dfield value according to its data type.
/// \param [in] dfield
IB_INTERN void dfield_print(const innodb_state* state, const dfield_t* dfield)
{
	ulint len = dfield_get_len(dfield);
	const byte* data = dfield_get_data(dfield);
	
	if (dfield_is_null(dfield)) {
		ib_log(state, "NULL");
	 	return;
	}

	switch (dtype_get_mtype(dfield_get_type(dfield))) {
	case DATA_CHAR:
	// fallthrough
	case DATA_VARCHAR:
		for (ulint i = 0; i < len; i++) {
			int c = *data++;
			ib_log(state, "%c", isprint(c) ? c : ' ');
		}	
		if (dfield_is_ext(dfield)) {
			ib_log(state, "(external)");
		}
		break;
	case DATA_INT:		
		ut_a(len == 4); // only works for 32-bit integers
		ib_log(state, "%d", (int)mach_read_from_4(data));
		break;
	default:
		UT_ERROR;
	}
}

/// \brief Pretty prints a dfield value according to its data type.
/// \param [in] dfield
/// \details Also the hex string is printed if a string contains non-printable characters.
IB_INTERN void dfield_print_also_hex(const innodb_state* state, const dfield_t* dfield)
{
	const byte* data = dfield_get_data(dfield);
	ulint len = dfield_get_len(dfield);
	ulint i;
	ibool print_also_hex;

	if (dfield_is_null(dfield)) {
		ib_log(state, "NULL");
		return;
	}

	ulint prtype = dtype_get_prtype(dfield_get_type(dfield));	
	switch (dtype_get_mtype(dfield_get_type(dfield))) {
	case DATA_INT: {
		switch (len) {			
		case 1: {
			ulint val = mach_read_from_1(data);
			if (!(prtype & DATA_UNSIGNED)) {
				val &= ~0x80;
				ib_log(state, "%ld", (long) val);
			} else {
				ib_log(state, "%lu", (ulong) val);
			}
			break;
		}	
		case 2: {
			ulint val = mach_read_from_2(data);
			if (!(prtype & DATA_UNSIGNED)) {
				val &= ~0x8000;
				ib_log(state, "%ld", (long) val);
			} else {
				ib_log(state, "%lu", (ulong) val);
			}
			break;
		}
		case 3: {
			ulint val = mach_read_from_3(data);
			if (!(prtype & DATA_UNSIGNED)) {
				val &= ~0x800000;
				ib_log(state, "%ld", (long) val);
			} else {
				ib_log(state, "%lu", (ulong) val);
			}
			break;
		}
		case 4: {
			ulint val = mach_read_from_4(data);
			if (!(prtype & DATA_UNSIGNED)) {
				val &= ~0x80000000;
				ib_log(state, "%ld", (long) val);
			} else {
				ib_log(state, "%lu", (ulong) val);
			}
			break;
		}
		case 6: {
			dulint id = mach_read_from_6(data);
			ib_log(state, "{%lu %lu}",
			ut_dulint_get_high(id),
			ut_dulint_get_low(id));
			break;	
		}
		case 7: {
			dulint id = mach_read_from_7(data);
			ib_log(state, "{%lu %lu}",
			ut_dulint_get_high(id),
			ut_dulint_get_low(id));
			break;
		}
		case 8: {
			dulint id = mach_read_from_8(data);
			ib_log(state, "{%lu %lu}",
			ut_dulint_get_high(id),
			ut_dulint_get_low(id));
			break;
		}
		default:
			goto print_hex;
		}
		break;
	}

	case DATA_SYS: {
		switch (prtype & DATA_SYS_PRTYPE_MASK) {
		case DATA_TRX_ID: {
			dulint id = mach_read_from_6(data);	
			ib_log(state, "trx_id " TRX_ID_FMT,
			TRX_ID_PREP_PRINTF(id));
			break;
		}
		case DATA_ROLL_PTR: {
			dulint id = mach_read_from_7(data);
			ib_log(state, "roll_ptr {%lu %lu}",
			ut_dulint_get_high(id), ut_dulint_get_low(id));
			break;
		}	
		case DATA_ROW_ID: {
			dulint id = mach_read_from_6(data);
			ib_log(state, "row_id {%lu %lu}",
			ut_dulint_get_high(id), ut_dulint_get_low(id));
			break;
		}
		default: {
			dulint id = mach_dulint_read_compressed(data);
			ib_log(state, "mix_id {%lu %lu}",
			ut_dulint_get_high(id), ut_dulint_get_low(id));
			break;
		}
		}
		break;
	}
	
	case DATA_CHAR: // fallthrough
	case DATA_VARCHAR:
		print_also_hex = FALSE;
		for (i = 0; i < len; i++) {
			int c = *data++;
			if (!isprint(c)) {
				print_also_hex = TRUE;
				ib_log(state, "\\x%02x", (unsigned char) c);
			} else {
				ib_log(state,"%c", c);
			}
		}

		if (dfield_is_ext(dfield)) {
			ib_log(state, "(external)");
		}

		if (!print_also_hex) {
			break;
		}

		data = dfield_get_data(dfield);
		// fall through

	case DATA_BINARY:
	default:
print_hex:
		ib_log(state, " Hex: ");
		for (i = 0; i < len; i++) {
			ib_log(state, "%02lx", (ulint) *data++);
		}
		if (dfield_is_ext(dfield)) {
			ib_log(state, "(external)");
		}
	}
}

/// \brief Print a dfield value using ut_print_buf.
static void dfield_print_raw(innodb_state* state, const dfield_t* dfield)
{
	ulint len = dfield_get_len(dfield);
	if (!dfield_is_null(dfield)) {
	 	ulint print_len = ut_min(len, 1000);
		ut_print_buf(state, dfield_get_data(dfield), print_len);
		if (len != print_len) {
			ib_log(state, "(total %lu bytes%s)", (ulong) len, dfield_is_ext(dfield) ? ", external" : "");
		}
	} else {
		ib_log(state, " SQL NULL");
	}
}

/// \brief The following function prints the contents of a tuple.
/// \param [in] state->stream output stream
/// \param [in] tuple
IB_INTERN void dtuple_print(innodb_state* state, const dtuple_t* tuple)
{
	ulint n_fields = dtuple_get_n_fields(tuple);
	ib_log(state, "DATA TUPLE: %lu fields;\n", (ulong) n_fields);
	for (ulint i = 0; i < n_fields; i++) {
	 	ib_log(state, " %lu:", (ulong) i);
		dfield_print_raw(state->stream, dtuple_get_nth_field(tuple, i));
		ib_log(state, ";\n");
	}
	ut_ad(dtuple_validate(tuple));
}

/// \brief Moves parts of long fields in entry to the big record vector so that the size of tuple drops below the maximum record size allowed in the database.
/// \param [in] index
/// \param [in,out] entry index entry
/// \param [in,out] n_ext number of externally stored columns
/// \return created big record vector, NULL if we are not able to shorten the entry enough
/// \details Moves data only from those fields which are not necessary to determine uniquely the insertion place of the tuple in the index. i.e., if there are too many fixed-length or short fields in entry or the index is clustered
IB_INTERN big_rec_t* dtuple_convert_big_rec(innodb_state* state, ib_dict_index_t* index, dtuple_t* entry, ulint* n_ext)
{
	big_rec_t* vector;
	ulint n_fields;	
	ulint local_prefix_len;

	if (IB_UNLIKELY(!dict_index_is_clust(index))) {
		return(NULL);
	}

	ulint local_len = BTR_EXTERN_FIELD_REF_SIZE;
	if (dict_table_get_format(index->table) < DICT_TF_FORMAT_ZIP) {
		// up to v5.1: store a 768-byte prefix locally 
		local_len += DICT_MAX_INDEX_COL_LEN;
	}
	
	ut_a(dtuple_check_typed_no_assert(entry));
	ulint size = rec_get_converted_size(index, entry, *n_ext);
	if (IB_UNLIKELY(size > 1000000000)) {
		ib_log(state, "InnoDB: Warning: tuple size very big: %lu\n", (ulong) size);
		ib_log(state, "InnoDB: Tuple contents: ");
		dtuple_print(state->stream, entry);
		ib_log(state, "\n");
	}

	mem_heap_t* heap = IB_MEM_HEAP_CREATE(size + dtuple_get_n_fields(entry) * sizeof(big_rec_field_t) + 1000);
	vector = mem_heap_alloc(heap, sizeof(big_rec_t));
	vector->heap = heap;
	vector->fields = mem_heap_alloc(heap, dtuple_get_n_fields(entry) * sizeof(big_rec_field_t));
	// Decide which fields to shorten: the algorithm is to look for a variable-length field that yields the biggest savings when stored externally
	n_fields = 0;

	while (page_rec_needs_ext(rec_get_converted_size(index, entry, *n_ext), dict_table_is_comp(index->table), dict_index_get_n_fields(index), dict_table_zip_size(index->table))) {		
		byte* data;
		big_rec_field_t* b;
		
		ulint longest_i = ULINT_MAX;
		ulint longest = 0;
	 	for (ulint i = dict_index_get_n_unique_in_tree(index); i < dtuple_get_n_fields(entry); ++i) {
			dfield_t* dfield = dtuple_get_nth_field(entry, i);
			dict_field_t* ifield = dict_index_get_nth_field(index, i);

	  		// Skip fixed-length, NULL, externally stored, or short columns

	  		if (ifield->fixed_len || dfield_is_null(dfield) || dfield_is_ext(dfield) || dfield_get_len(dfield) <= local_len || dfield_get_len(dfield) <= BTR_EXTERN_FIELD_REF_SIZE * 2) {
				continue; // skip
			}

			ulint savings = dfield_get_len(dfield) - local_len;

			// Check that there would be savings
			if (longest >= savings) {
				continue; // skip
			}

			/// This is because there is no room for the "external storage" flag when the maximum length is 255 bytes or less. 
			/// This restriction trivially holds in REDUNDANT and COMPACT format, because there we always store locally columns whose length is up to local_len == 788 bytes.
			/// \see rec_init_offsets_comp_ordinary
			/// In DYNAMIC and COMPRESSED format, store locally any non-BLOB columns whose maximum length does not exceed 256 bytes.  
			if (ifield->col->mtype != DATA_BLOB && ifield->col->len < 256) {
				continue; // skip 
			}

			longest_i = i;
			longest = savings;
	  		continue;
	 	}

		if (!longest) {
			// Cannot shorten more
			IB_MEM_HEAP_FREE(heap);
			return NULL;
		}

		// Move data from field longest_i to big rec vector. We store the first bytes locally to the record. Then we can calculate all ordering fields in all indexes from locally stored data.
		dfield_t* dfield = dtuple_get_nth_field(entry, longest_i);
		dict_field_t* ifield = dict_index_get_nth_field(index, longest_i);
		ulint local_prefix_len = local_len - BTR_EXTERN_FIELD_REF_SIZE;

		b = &vector->fields[n_fields];
		b->field_no = longest_i;
		b->len = dfield_get_len(dfield) - local_prefix_len;
		b->data = (char*) dfield_get_data(dfield) + local_prefix_len;

		// Allocate the locally stored part of the column.
		data = mem_heap_alloc(heap, local_len);

		// Copy the local prefix.
		memcpy(data, dfield_get_data(dfield), local_prefix_len);

		// Clear the extern field reference (BLOB pointer).
		memset(data + local_prefix_len, 0, BTR_EXTERN_FIELD_REF_SIZE);
#if 0
	 	// The following would fail the Valgrind checks in page_cur_insert_rec_low() and page_cur_insert_rec_zip(). The BLOB pointers in the record will be initialized after the record and the BLOBs have been written.
	 	IB_MEM_ALLOC(data + local_prefix_len, BTR_EXTERN_FIELD_REF_SIZE);
#endif

		dfield_set_data(dfield, data, local_len);
		dfield_set_ext(dfield);

		++n_fields;
		++(*n_ext);
	 	ut_ad(n_fields < dtuple_get_n_fields(entry));
	}

	vector->n_fields = n_fields;
	return vector;
}

/// \brief Puts back to entry the data stored in vector.
/// \param [in] index
/// \param [in] entry entry whose data was put to vector
/// \param [in] vector big rec vector; it is freed in this function
/// \note Note that to ensure the fields in entry can accommodate the data, vector must have been created from entry with dtuple_convert_big_rec.
/// \internal
IB_INTERN void dtuple_convert_back_big_rec(ib_dict_index_t* index, dtuple_t* entry, big_rec_t* vector)
{
	UT_UNUSED(index);
	
	big_rec_field_t* b = vector->fields;
	const big_rec_field_t*const end = b + vector->n_fields;

	for (; b < end; ++b) {
		dfield_t* dfield = dtuple_get_nth_field(entry, b->field_no);
		ulint local_len = dfield_get_len(dfield);
		ut_ad(dfield_is_ext(dfield));
		ut_ad(local_len >= BTR_EXTERN_FIELD_REF_SIZE);

		local_len -= BTR_EXTERN_FIELD_REF_SIZE;
		ut_ad(local_len <= DICT_MAX_INDEX_COL_LEN);
		dfield_set_data(dfield, (char*) b->data - local_len, b->len + local_len);
	}

	IB_MEM_HEAP_FREE(vector->heap);
}

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

static ibool dfield_check_typed_no_assert(const dfield_t* field)
{
	if (dfield_get_type(field)->mtype > DATA_CLIENT || dfield_get_type(field)->mtype < DATA_VARCHAR) {
		ib_log(state, "InnoDB: Error: data field type %lu, len %lu\n", (ulong) dfield_get_type(field)->mtype, (ulong) dfield_get_len(field));
		return FALSE;
	}

	return TRUE;
}
#endif // !IB_HOTBACKUP 