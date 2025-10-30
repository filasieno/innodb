// Copyright (c) 1994, 2009, Innobase Oy. All Rights Reserved.
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

/// \file data_data.inl
/// \brief SQL data field and tuple
/// \details Originally created by Heikki Tuuri in 5/30/1994
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "mem_mem.hpp"
#include "ut_rnd.hpp"

#ifdef IB_DEBUG

/// \brief Dummy variable to catch access to uninitialized fields. 
/// \details In the debug version, dtuple_create() will make all fields of dtuple_t point to data_error.
extern byte data_error;

/// \brief Gets pointer to the type struct of SQL data field.
/// \return pointer to the type struct
/// \param [in] field SQL data field
IB_INLINE dtype_t* dfield_get_type(const dfield_t* field);
{
	ut_ad(field);

	return (dtype_t*) &(field->type);
}
#endif // IB_DEBUG 

/// \brief Sets the type struct of SQL data field.
/// \param [in] field SQL data field
/// \param [in] type pointer to data type struct
IB_INLINE void dfield_set_type(dfield_t* field, dtype_t* type)
{
	ut_ad(field && type);

	field->type = *type;
}

#ifdef IB_DEBUG
/// \brief Gets pointer to the data in a field.
/// \return pointer to data
/// \param [in] field field
IB_INLINE void* dfield_get_data(const dfield_t* field)
{
	ut_ad(field);
	ut_ad((field->len == IB_SQL_NULL) || (field->data != &data_error));
	return (void*) field->data;
}
#endif // IB_DEBUG

/// \brief Gets length of field data.
/// \return length of data; IB_SQL_NULL if SQL null data
/// \param [in] field field
IB_INLINE ulint dfield_get_len(const dfield_t* field)
{
	ut_ad(field);
	ut_ad((field->len == IB_SQL_NULL) || (field->data != &data_error));
	return field->len;
}

/// \brief Sets length in a field.
/// \param [in] field field
/// \param [in] len length or IB_SQL_NULL
IB_INLINE void dfield_set_len(dfield_t* field, ulint len)
{
	ut_ad(field);
#ifdef IB_VALGRIND_DEBUG
	if (len != IB_SQL_NULL) IB_MEM_ASSERT_RW(field->data, len);
#endif /* IB_VALGRIND_DEBUG */

	field->ext = 0;
	field->len = len;
}

/// \brief Determines if a field is SQL NULL
/// \return nonzero if SQL null data
/// \param [in] field field
IB_INLINE ulint dfield_is_null(const dfield_t* field)
{
	ut_ad(field);
	return field->len == IB_SQL_NULL ;
}

/// \brief Determines if a field is externally stored
/// \return nonzero if externally stored
/// \param [in] field field
IB_INLINE ulint dfield_is_ext(const dfield_t* field)
{
	ut_ad(field);
	return IB_UNLIKELY(field->ext);
}

/// \brief Sets the "external storage" flag
/// \param [in,out] field field
IB_INLINE void dfield_set_ext(dfield_t* field)
{
	ut_ad(field);
	field->ext = 1;
}

/// \brief Sets pointer to the data and length in a field.
/// \param [in] field field
/// \param [in] data data
/// \param [in] len length or IB_SQL_NULL
IB_INLINE void dfield_set_data(dfield_t* field, const void* data, ulint len)
{
	ut_ad(field);

#ifdef IB_VALGRIND_DEBUG
	if (len != IB_SQL_NULL) IB_MEM_ASSERT_RW(data, len);
#endif /* IB_VALGRIND_DEBUG */
	field->data = (void*) data;
	field->ext = 0;
	field->len = len;
}

/// \brief Sets a data field to SQL NULL.
/// \param [in,out] field field
IB_INLINE void dfield_set_null(dfield_t* field)
{
	dfield_set_data(field, NULL, IB_SQL_NULL);
}

/// \brief Copies the data and len fields.
/// \param [out] field1 field to copy to
/// \param [in] field2 field to copy from
IB_INLINE void dfield_copy_data(dfield_t* field1, const dfield_t* field2)
{
	ut_ad(field1 && field2);

	field1->data = field2->data;
	field1->len = field2->len;
	field1->ext = field2->ext;
}

/// \brief Copies a data field to another.
/// \param field1 field to copy to
/// \param field2 field to copy from
IB_INLINE void dfield_copy(dfield_t* field1, const dfield_t* field2)
{
	*field1 = *field2;
}

/// \brief Copies the data pointed to by a data field.
/// \param field data field
/// \param heap memory heap where allocated
IB_INLINE void dfield_dup(dfield_t* field, mem_heap_t* heap)
{
	if (!dfield_is_null(field)) {
		IB_MEM_ASSERT_RW(field->data, field->len);
		field->data = mem_heap_dup(heap, field->data, field->len);
	}
}

/// \brief Tests if data length and content is equal for two dfields.
/// \return TRUE if equal
/// \param [in] field1 field
/// \param [in] field2 field
IB_INLINE ibool dfield_datas_are_binary_equal(const dfield_t* field1, const dfield_t* field2)	
{
	ulint len = field1->len;
	return len == field2->len && (len == IB_SQL_NULL || !memcmp(field1->data, field2->data, len));
}

/// \brief Gets info bits in a data tuple.
/// \return info bits
/// \param [in] tuple tuple
IB_INLINE ulint dtuple_get_info_bits(const dtuple_t* tuple);
{
	ut_ad(tuple);
	return tuple->info_bits;
}

/// \brief Sets info bits in a data tuple.
/// \param [in] tuple tuple
/// \param [in] info_bits info bits
IB_INLINE void dtuple_set_info_bits(dtuple_t* tuple, ulint info_bits);
{
	ut_ad(tuple);
	tuple->info_bits = info_bits;
}

/// \brief Gets number of fields used in record comparisons.
/// \return number of fields used in comparisons in rem_cmp.*
/// \param [in] tuple tuple
IB_INLINE ulint dtuple_get_n_fields_cmp(const dtuple_t* tuple);
{
	ut_ad(tuple);
	return tuple->n_fields_cmp;
}

/// \brief Sets number of fields used in record comparisons.
/// \param [in] tuple tuple
/// \param [in] n_fields_cmp number of fields used in comparisons in rem_cmp.*
IB_INLINE void dtuple_set_n_fields_cmp(dtuple_t* tuple, ulint n_fields_cmp);
{
	ut_ad(tuple);
	ut_ad(n_fields_cmp <= tuple->n_fields);
	tuple->n_fields_cmp = n_fields_cmp;
}

/// \brief Gets number of fields in a data tuple.
/// \return number of fields
/// \param [in] tuple tuple
IB_INLINE ulint dtuple_get_n_fields(const dtuple_t* tuple);
{
	ut_ad(tuple);
	return tuple->n_fields;
}

#ifdef IB_DEBUG
/// \brief Gets nth field of a tuple.
/// \return nth field
/// \param [in] tuple tuple
/// \param [in] n index of field
IB_INLINE dfield_t* dtuple_get_nth_field(const dtuple_t* tuple, ulint n);
{
	ut_ad(tuple);
	ut_ad(n < tuple->n_fields);
	return (dfield_t*) tuple->fields + n;
}
#endif // IB_DEBUG

/// \brief Creates a data tuple to a memory heap. The default value for number of fields used in record comparisons for this tuple is n_fields.
/// \return own: created tuple
/// \param [in] heap memory heap where the tuple is created
/// \param [in] n_fields number of fields
IB_INLINE dtuple_t* dtuple_create(mem_heap_t* heap, ulint n_fields);
{
	ut_ad(heap);

	dtuple_t* tuple = (dtuple_t*) mem_heap_alloc(heap, sizeof(dtuple_t) + n_fields * sizeof(dfield_t));
	tuple->info_bits = 0;
	tuple->n_fields = n_fields;
	tuple->n_fields_cmp = n_fields;
	tuple->fields = (dfield_t*) &tuple[1];

	if constexpr (IB_DEBUG) {
		tuple->magic_n = DATA_TUPLE_MAGIC_N;
		// In the debug version, initialize fields to an error value
		for (ulint i = 0; i < n_fields; i++) {
			dfield_t* field = dtuple_get_nth_field(tuple, i);
			dfield_set_len(field, IB_SQL_NULL);
			field->data = &data_error;
			dfield_get_type(field)->mtype = DATA_ERROR;
		}

		IB_MEM_INVALID(tuple->fields, n_fields * sizeof *tuple->fields);
	}
	return(tuple);
}

/// \brief Wrap data fields in a tuple. The default value for number of fields used in record comparisons for this tuple is n_fields.
/// \return data tuple
/// \param [in] tuple storage for data tuple
/// \param [in] fields fields
/// \param [in] n_fields number of fields
IB_INLINE const dtuple_t* dtuple_from_fields(dtuple_t* tuple, const dfield_t* fields, ulint n_fields);
{
	tuple->info_bits = 0;
	tuple->n_fields = tuple->n_fields_cmp = n_fields;
	tuple->fields = (dfield_t*) fields;
	ut_d(tuple->magic_n = DATA_TUPLE_MAGIC_N);
	return tuple;
}

/// \brief Copies a data tuple to another.
/// \details This is a shallow copy; if a deep copy is desired, dfield_dup() will have to be invoked on each field.
/// \return own: copy of tuple
/// \param [in] tuple tuple to copy from
/// \param [in] heap memory heap
IB_INLINE dtuple_t* dtuple_copy(const dtuple_t* tuple, mem_heap_t* heap);
{
	ulint n_fields = dtuple_get_n_fields(tuple);
	dtuple_t* new_tuple = dtuple_create(heap, n_fields);
	for (ulint i = 0; i < n_fields; i++) {
		dfield_copy(dtuple_get_nth_field(new_tuple, i), dtuple_get_nth_field(tuple, i));
	}
	return new_tuple;
}

/// \brief The following function returns the sum of data lengths of a tuple.
/// \details The space occupied by the field structs or the tuple struct is not counted. Neither is possible space in externally stored parts of the field.
/// \return sum of data lengths
/// \param [in] tuple typed data tuple
/// \param [in] comp nonzero=ROW_FORMAT=COMPACT
IB_INLINE ulint dtuple_get_data_size(const dtuple_t* tuple, ulint comp);
{
	ulint sum = 0;
	ut_ad(tuple);
	ut_ad(dtuple_check_typed(tuple));
	ut_ad(tuple->magic_n == DATA_TUPLE_MAGIC_N);
	ulint n_fields = tuple->n_fields;
	for (ulint i = 0; i < n_fields; i++) {
		const dfield_t* field = dtuple_get_nth_field(tuple, i);
		ulint len = dfield_get_len(field);
		if (len == IB_SQL_NULL) {
			len = dtype_get_sql_null_size(dfield_get_type(field), comp);
		}
		sum += len;
	}
	return sum;
}

/// \brief Computes the number of externally stored fields in a data tuple.
/// \return number of externally stored fields
/// \param [in] tuple tuple
IB_INLINE ulint dtuple_get_n_ext(const dtuple_t* tuple);
{
	ulint n_ext = 0;
	ut_ad(tuple);
	ut_ad(dtuple_check_typed(tuple));
	ut_ad(tuple->magic_n == DATA_TUPLE_MAGIC_N);
	ulint n_fields = tuple->n_fields;
	for (ulint i = 0; i < n_fields; i++) {
		n_ext += dtuple_get_nth_field(tuple, i)->ext;
	}
	return n_ext;
}

/// \brief Sets types of fields binary in a tuple.
/// \param [in] tuple data tuple
/// \param [in] n number of fields to set
IB_INLINE void dtuple_set_types_binary(dtuple_t* tuple, ulint n);
{
	for (ulint i = 0; i < n; i++) {
		dtype_t* dfield_type = dfield_get_type(dtuple_get_nth_field(tuple, i));
		dtype_set(dfield_type, DATA_BINARY, 0, 0);
	}
}

/// \brief Folds a prefix given as the number of fields of a tuple.
/// \return the folded value
/// \param [in] tuple the tuple
/// \param [in] n_fields number of complete fields to fold
/// \param [in] n_bytes number of bytes to fold in an incomplete last field
/// \param [in] tree_id index tree id
IB_INLINE ulint dtuple_fold(const dtuple_t* tuple, ulint n_fields, ulint n_bytes, dulint tree_id);
{
	ut_ad(tuple);
	ut_ad(tuple->magic_n == DATA_TUPLE_MAGIC_N);
	ut_ad(dtuple_check_typed(tuple));
	ulint fold = ut_fold_dulint(tree_id);

	for (ulint i = 0; i < n_fields; i++) {
		const dfield_t* field = dtuple_get_nth_field(tuple, i);
		const byte* data = (const byte*) dfield_get_data(field);
		ulint len = dfield_get_len(field);

		if (len != IB_SQL_NULL) {
			fold = ut_fold_ulint_pair(fold, ut_fold_binary(data, len));
		}
	}

	if (n_bytes > 0) {
		const dfield_t* field = dtuple_get_nth_field(tuple, i);
		const byte* data = (const byte*) dfield_get_data(field);
		ulint len = dfield_get_len(field);
		if (len != IB_SQL_NULL) {
			if (len > n_bytes) {
				len = n_bytes;
			}
			fold = ut_fold_ulint_pair(fold, ut_fold_binary(data, len));
		}
	}

	return fold;
}

/// \brief Writes an SQL null field full of zeros.
/// \param [in] data pointer to a buffer of size len
/// \param [in] len SQL null size in bytes
IB_INLINE void data_write_sql_null(byte* data, ulint len);
{
	memset(data, 0, len);
}

/// \brief Checks if a dtuple contains an SQL null value.
/// \return TRUE if some field is SQL null
/// \param [in] tuple dtuple
IB_INLINE ibool dtuple_contains_null(const dtuple_t* tuple);
{
	ulint n = dtuple_get_n_fields(tuple);
	for (ulint i = 0; i < n; i++) {
		if (dfield_is_null(dtuple_get_nth_field(tuple, i))) {
			return TRUE;
		}
	}
	return FALSE;
}

/// \brief Frees the memory in a big rec vector.
/// \param [in] vector big rec vector; it is freed in this function
IB_INLINE void dtuple_big_rec_free(big_rec_t* vector);
{
	IB_MEM_HEAP_FREE(vector->heap);
}
