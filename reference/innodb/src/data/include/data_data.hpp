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

/// \file data_data.hpp
/// \brief SQL data field and tuple
/// \details Originally created by Heikki Tuuri in 5/30/1994
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "defs.hpp"

#include "data_types.hpp"
#include "data_type.hpp"
#include "mem_mem.hpp"
#include "dict_types.hpp"

///TODO: use constexpr switch / inline function
#ifdef IB_DEBUG

	/// \brief Gets pointer to the type struct of SQL data field.
	/// \return pointer to the type struct
	/// \param [in] field SQL data field
	IB_INLINE dtype_t* dfield_get_type(const dfield_t* field);

	/// \brief Gets pointer to the data in a field.
	/// \return pointer to data
	/// \param [in] field field
	IB_INLINE void* dfield_get_data(const dfield_t* field);

#else // IB_DEBUG

	#define dfield_get_type(field) (&(field)->type)
	#define dfield_get_data(field) ((field)->data)

#endif // IB_DEBUG

/// \brief Sets the type struct of SQL data field.
/// \param [in] field SQL data field
/// \param [in] type pointer to data type struct
IB_INLINE void dfield_set_type(dfield_t* field, dtype_t* type);

/// \brief Gets length of field data.
/// \return length of data; IB_SQL_NULL if SQL null data
/// \param [in] field field
IB_INLINE ulint dfield_get_len(const dfield_t* field);

/// \brief Sets length in a field.
/// \param [in] field field
/// \param [in] len length or IB_SQL_NULL
IB_INLINE void dfield_set_len(dfield_t* field, ulint len);

/// \brief Determines if a field is SQL NULL.
/// \return nonzero if SQL null data
/// \param [in] field field
IB_INLINE ulint dfield_is_null(const dfield_t* field);

/// \brief Determines if a field is externally stored.
/// \return nonzero if externally stored
/// \param [in] field field
IB_INLINE ulint dfield_is_ext(const dfield_t* field);

/// \brief Sets the "external storage" flag.
/// \param [in,out] field field
IB_INLINE void dfield_set_ext(dfield_t* field);

/// \brief Sets pointer to the data and length in a field.
/// \param [in] field field
/// \param [in] data data
/// \param [in] len length or IB_SQL_NULL
IB_INLINE void dfield_set_data(dfield_t* field, const void* data, ulint len);

/// \brief Sets a data field to SQL NULL.
/// \param [in,out] field field
IB_INLINE void dfield_set_null(dfield_t* field);
/// \brief Writes an SQL null field full of zeros.

/// \param [in] data pointer to a buffer of size len
/// \param [in] len SQL null size in bytes
IB_INLINE void data_write_sql_null(byte* data, ulint len);

/// \brief Copies the data and len fields.
/// \param [out] field1 field to copy to
/// \param [in] field2 field to copy from
IB_INLINE void dfield_copy_data(dfield_t* field1, const dfield_t* field2);

/// \brief Copies a data field to another.
/// \param [out] field1 field to copy to
/// \param [in] field2 field to copy from
IB_INLINE void dfield_copy(dfield_t* field1, const dfield_t* field2);

/// \brief Copies the data pointed to by a data field.
/// \param [in,out] field data field
/// \param [in] heap memory heap where allocated
IB_INLINE void dfield_dup(dfield_t* field, mem_heap_t* heap);

/// \brief Tests if data length and content is equal for two dfields.
/// \return TRUE if equal
/// \param [in] field1 field
/// \param [in] field2 field
IB_INLINE ibool dfield_datas_are_binary_equal(const dfield_t* field1, const dfield_t* field2);

/// \brief Tests if dfield data length and content is equal to the given.
/// \return TRUE if equal
/// \param [in] field field
/// \param [in] len data length or IB_SQL_NULL
/// \param [in] data data
IB_INTERN ibool dfield_data_is_binary_equal(const dfield_t* field, ulint len, const byte* data);

/// \brief Gets number of fields in a data tuple.
/// \return number of fields
/// \param [in] tuple tuple
IB_INLINE ulint dtuple_get_n_fields(const dtuple_t* tuple);

#ifdef IB_DEBUG

	/// \brief Gets nth field of a tuple.
	/// \return nth field
	/// \param [in] tuple tuple
	/// \param [in] n index of field
	IB_INLINE dfield_t* dtuple_get_nth_field(const dtuple_t* tuple, ulint n);

#else // IB_DEBUG 
	
	/// TODO: use inline function/constexpr
	#define dtuple_get_nth_field(tuple, n) ((tuple)->fields + (n))

#endif // IB_DEBUG 

/// \brief Gets info bits in a data tuple.
/// \return info bits
/// \param [in] tuple tuple
IB_INLINE ulint dtuple_get_info_bits(const dtuple_t* tuple);

/// \brief Sets info bits in a data tuple.
/// \param [in] tuple tuple
/// \param [in] info_bits info bits
IB_INLINE void dtuple_set_info_bits(dtuple_t* tuple, ulint info_bits);

/// \brief Gets number of fields used in record comparisons.
/// \return number of fields used in comparisons in rem0cmp.*
/// \param [in] tuple tuple
IB_INLINE ulint dtuple_get_n_fields_cmp(const dtuple_t* tuple);

/// \brief Sets number of fields used in record comparisons.
/// \param [in] tuple tuple
/// \param [in] n_fields_cmp number of fields used in comparisons in rem0cmp.*
IB_INLINE void dtuple_set_n_fields_cmp(dtuple_t* tuple, ulint n_fields_cmp);

/// \brief Creates a data tuple to a memory heap.
/// \details The default value for number of fields used in record comparisons for this tuple is n_fields.
/// \return own: created tuple
/// \param [in] heap memory heap where the tuple is created
/// \param [in] n_fields number of fields
IB_INLINE dtuple_t* dtuple_create(mem_heap_t* heap, ulint n_fields);

/// \brief Wrap data fields in a tuple.
/// \details The default value for number of fields used in record comparisons for this tuple is n_fields.
/// \return data tuple
/// \param [in] tuple storage for data tuple
/// \param [in] fields fields
/// \param [in] n_fields number of fields
IB_INLINE const dtuple_t* dtuple_from_fields(dtuple_t* tuple, const dfield_t* fields, ulint n_fields);

/// \brief Sets number of fields used in a tuple.
/// \details Normally this is set in dtuple_create, but if you want later to set it smaller, you can use this.
/// \param [in] tuple tuple
/// \param [in] n_fields number of fields
IB_INTERN void dtuple_set_n_fields(dtuple_t* tuple, ulint n_fields);

/// \brief Copies a data tuple to another.
/// \details This is a shallow copy; if a deep copy is desired, dfield_dup() will have to be invoked on each field.
/// \return own: copy of tuple
/// \param [in] tuple tuple to copy from
/// \param [in] heap memory heap where the tuple is created

IB_INLINE dtuple_t* dtuple_copy(const dtuple_t* tuple, mem_heap_t* heap);
/// \brief The following function returns the sum of data lengths of a tuple.
/// \details The space occupied by the field structs or the tuple struct is not counted.
/// \return sum of data lens
/// \param [in] tuple typed data tuple
/// \param [in] comp nonzero=ROW_FORMAT=COMPACT

IB_INLINE ulint dtuple_get_data_size(const dtuple_t* tuple, ulint comp);
/// \brief Computes the number of externally stored fields in a data tuple.
/// \return number of fields
/// \param [in] tuple tuple

IB_INLINE ulint dtuple_get_n_ext(const dtuple_t* tuple);
/// \brief Compare two data tuples, respecting the collation of character fields.
/// \return 1, 0 , -1 if tuple1 is greater, equal, less, respectively, than tuple2
/// \param [in] cmp_ctx client compare context
/// \param [in] tuple1 tuple 1
/// \param [in] tuple2 tuple 2

IB_INTERN int dtuple_coll_cmp(void* cmp_ctx, const dtuple_t* tuple1, const dtuple_t* tuple2);
/// \brief Folds a prefix given as the number of fields of a tuple.
/// \return the folded value
/// \param [in] tuple the tuple
/// \param [in] n_fields number of complete fields to fold
/// \param [in] n_bytes number of bytes to fold in an incomplete last field
/// \param [in] tree_id index tree id

IB_INLINE ulint dtuple_fold(const dtuple_t* tuple, ulint n_fields, ulint n_bytes, dulint tree_id) __attribute__((pure));
/// \brief Sets types of fields binary in a tuple.
/// \param [in] tuple data tuple
/// \param [in] n number of fields to set

IB_INLINE void dtuple_set_types_binary(dtuple_t* tuple, ulint n);
/// \brief Checks if a dtuple contains an SQL null value.
/// \return TRUE if some field is SQL null
/// \param [in] tuple dtuple

IB_INLINE ibool dtuple_contains_null(const dtuple_t* tuple);
/// \brief Checks that a data field is typed.
/// \details Asserts an error if not.
/// \return TRUE if ok
/// \param [in] field data field

IB_INTERN ibool dfield_check_typed(const dfield_t* field);
/// \brief Checks that a data tuple is typed.
/// \details Asserts an error if not.
/// \return TRUE if ok
/// \param [in] tuple tuple

IB_INTERN ibool dtuple_check_typed(const dtuple_t* tuple);
/// \brief Checks that a data tuple is typed.
/// \return TRUE if ok
/// \param [in] tuple tuple
IB_INTERN ibool dtuple_check_typed_no_assert(const dtuple_t* tuple);


#ifdef IB_DEBUG

	/// \brief Validates the consistency of a tuple which must be complete, i.e, all fields must have been set.
	/// \return TRUE if ok
	/// \param [in] tuple tuple
	IB_INTERN ibool dtuple_validate(const dtuple_t* tuple);

#endif // IB_DEBUG

/// \brief Pretty prints a dfield value according to its data type.
/// \param [in] dfield dfield
IB_INTERN void dfield_print(innodb_state* state, const dfield_t* dfield);

/// \brief Pretty prints a dfield value according to its data type.
/// \details Also the hex string is printed if a string contains non-printable characters.
/// \param [in] dfield dfield
IB_INTERN void dfield_print_also_hex(const dfield_t* dfield);

/// \brief The following function prints the contents of a tuple.
/// \param [in] state->stream output stream
/// \param [in] tuple tuple
IB_INTERN void dtuple_print(ib_stream_t state->stream, const dtuple_t* tuple);

/// \brief Moves parts of long fields in entry to the big record vector so that the size of tuple drops below the maximum record size allowed in the database.
/// \details Moves data only from those fields which are not necessary to determine uniquely the insertion place of the tuple in the index.
/// \return own: created big record vector, NULL if we are not able to shorten the entry enough, i.e., if there are too many fixed-length or short fields in entry or the index is clustered
/// \param [in] index index
/// \param [in,out] entry index entry
/// \param [in,out] n_ext number of externally stored columns
IB_INTERN big_rec_t* dtuple_convert_big_rec(ib_dict_index_t* index, dtuple_t* entry, ulint* n_ext);

/// \brief Puts back to entry the data stored in vector.
/// \details Note that to ensure the fields in entry can accommodate the data, vector must have been created from entry with dtuple_convert_big_rec.
/// \param [in] index index
/// \param [in] entry entry whose data was put to vector
/// \param [in] vector big rec vector; it is freed in this function
IB_INTERN void dtuple_convert_back_big_rec(ib_dict_index_t* index, dtuple_t* entry, big_rec_t* vector);

/// \brief Frees the memory in a big rec vector.
/// \param [in] vector big rec vector; it is freed in this function
IB_INLINE void dtuple_big_rec_free(big_rec_t* vector);

/// \brief Reset dfield variables.
IB_INTERN void dfield_var_init(void);

#ifndef IB_DO_NOT_INLINE
	#include "data_data.inl"
#endif
