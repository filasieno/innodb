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

#include "univ.i"

#include "data_types.hpp"
#include "data_type.hpp"
#include "mem_mem.hpp"
#include "dict_types.hpp"

/** Storage for overflow data in a big record, that is, a clustered
index record which needs external storage of data fields */
typedef struct big_rec_struct		big_rec_t;

#ifdef IB_DEBUG
/*********************************************************************//**
Gets pointer to the type struct of SQL data field.
@return	pointer to the type struct */
IB_INLINE
dtype_t*
dfield_get_type(
/*============*/
	const dfield_t*	field);	/*!< in: SQL data field */
/*********************************************************************//**
Gets pointer to the data in a field.
@return	pointer to data */
IB_INLINE
void*
dfield_get_data(
/*============*/
	const dfield_t* field);	/*!< in: field */
#else /* IB_DEBUG */
# define dfield_get_type(field) (&(field)->type)
# define dfield_get_data(field) ((field)->data)
#endif /* IB_DEBUG */
/*********************************************************************//**
Sets the type struct of SQL data field. */
IB_INLINE
void
dfield_set_type(
/*============*/
	dfield_t*	field,	/*!< in: SQL data field */
	dtype_t*	type);	/*!< in: pointer to data type struct */
/*********************************************************************//**
Gets length of field data.
@return	length of data; IB_SQL_NULL if SQL null data */
IB_INLINE
ulint
dfield_get_len(
/*===========*/
	const dfield_t* field);	/*!< in: field */
/*********************************************************************//**
Sets length in a field. */
IB_INLINE
void
dfield_set_len(
/*===========*/
	dfield_t*	field,	/*!< in: field */
	ulint		len);	/*!< in: length or IB_SQL_NULL */
/*********************************************************************//**
Determines if a field is SQL NULL
@return	nonzero if SQL null data */
IB_INLINE
ulint
dfield_is_null(
/*===========*/
	const dfield_t* field);	/*!< in: field */
/*********************************************************************//**
Determines if a field is externally stored
@return	nonzero if externally stored */
IB_INLINE
ulint
dfield_is_ext(
/*==========*/
	const dfield_t* field);	/*!< in: field */
/*********************************************************************//**
Sets the "external storage" flag */
IB_INLINE
void
dfield_set_ext(
/*===========*/
	dfield_t*	field);	/*!< in/out: field */
/*********************************************************************//**
Sets pointer to the data and length in a field. */
IB_INLINE
void
dfield_set_data(
/*============*/
	dfield_t*	field,	/*!< in: field */
	const void*	data,	/*!< in: data */
	ulint		len);	/*!< in: length or IB_SQL_NULL */
/*********************************************************************//**
Sets a data field to SQL NULL. */
IB_INLINE
void
dfield_set_null(
/*============*/
	dfield_t*	field);	/*!< in/out: field */
/**********************************************************************//**
Writes an SQL null field full of zeros. */
IB_INLINE
void
data_write_sql_null(
/*================*/
	byte*	data,	/*!< in: pointer to a buffer of size len */
	ulint	len);	/*!< in: SQL null size in bytes */
/*********************************************************************//**
Copies the data and len fields. */
IB_INLINE
void
dfield_copy_data(
/*=============*/
	dfield_t*	field1,	/*!< out: field to copy to */
	const dfield_t*	field2);/*!< in: field to copy from */
/*********************************************************************//**
Copies a data field to another. */
IB_INLINE
void
dfield_copy(
/*========*/
	dfield_t*	field1,	/*!< out: field to copy to */
	const dfield_t*	field2);/*!< in: field to copy from */
/*********************************************************************//**
Copies the data pointed to by a data field. */
IB_INLINE
void
dfield_dup(
/*=======*/
	dfield_t*	field,	/*!< in/out: data field */
	mem_heap_t*	heap);	/*!< in: memory heap where allocated */
/*********************************************************************//**
Tests if data length and content is equal for two dfields.
@return	TRUE if equal */
IB_INLINE
ibool
dfield_datas_are_binary_equal(
/*==========================*/
	const dfield_t*	field1,	/*!< in: field */
	const dfield_t*	field2);/*!< in: field */
/*********************************************************************//**
Tests if dfield data length and content is equal to the given.
@return	TRUE if equal */
IB_INTERN
ibool
dfield_data_is_binary_equal(
/*========================*/
	const dfield_t*	field,	/*!< in: field */
	ulint		len,	/*!< in: data length or IB_SQL_NULL */
	const byte*	data);	/*!< in: data */
/*********************************************************************//**
Gets number of fields in a data tuple.
@return	number of fields */
IB_INLINE
ulint
dtuple_get_n_fields(
/*================*/
	const dtuple_t*	tuple);	/*!< in: tuple */
#ifdef IB_DEBUG
/*********************************************************************//**
Gets nth field of a tuple.
@return	nth field */
IB_INLINE
dfield_t*
dtuple_get_nth_field(
/*=================*/
	const dtuple_t*	tuple,	/*!< in: tuple */
	ulint		n);	/*!< in: index of field */
#else /* IB_DEBUG */
# define dtuple_get_nth_field(tuple, n) ((tuple)->fields + (n))
#endif /* IB_DEBUG */
/*********************************************************************//**
Gets info bits in a data tuple.
@return	info bits */
IB_INLINE
ulint
dtuple_get_info_bits(
/*=================*/
	const dtuple_t*	tuple);	/*!< in: tuple */
/*********************************************************************//**
Sets info bits in a data tuple. */
IB_INLINE
void
dtuple_set_info_bits(
/*=================*/
	dtuple_t*	tuple,		/*!< in: tuple */
	ulint		info_bits);	/*!< in: info bits */
/*********************************************************************//**
Gets number of fields used in record comparisons.
@return	number of fields used in comparisons in rem0cmp.* */
IB_INLINE
ulint
dtuple_get_n_fields_cmp(
/*====================*/
	const dtuple_t*	tuple);	/*!< in: tuple */
/*********************************************************************//**
Gets number of fields used in record comparisons. */
IB_INLINE
void
dtuple_set_n_fields_cmp(
/*====================*/
	dtuple_t*	tuple,		/*!< in: tuple */
	ulint		n_fields_cmp);	/*!< in: number of fields used in
					comparisons in rem0cmp.* */
/**********************************************************//**
Creates a data tuple to a memory heap. The default value for number
of fields used in record comparisons for this tuple is n_fields.
@return	own: created tuple */
IB_INLINE
dtuple_t*
dtuple_create(
/*==========*/
	mem_heap_t*	heap,	/*!< in: memory heap where the tuple
				is created */
	ulint		n_fields); /*!< in: number of fields */

/**********************************************************//**
Wrap data fields in a tuple. The default value for number
of fields used in record comparisons for this tuple is n_fields.
@return	data tuple */
IB_INLINE
const dtuple_t*
dtuple_from_fields(
/*===============*/
	dtuple_t*	tuple,		/*!< in: storage for data tuple */
	const dfield_t*	fields,		/*!< in: fields */
	ulint		n_fields);	/*!< in: number of fields */

/*********************************************************************//**
Sets number of fields used in a tuple. Normally this is set in
dtuple_create, but if you want later to set it smaller, you can use this. */
IB_INTERN
void
dtuple_set_n_fields(
/*================*/
	dtuple_t*	tuple,		/*!< in: tuple */
	ulint		n_fields);	/*!< in: number of fields */
/*********************************************************************//**
Copies a data tuple to another.  This is a shallow copy; if a deep copy
is desired, dfield_dup() will have to be invoked on each field.
@return	own: copy of tuple */
IB_INLINE
dtuple_t*
dtuple_copy(
/*========*/
	const dtuple_t*	tuple,	/*!< in: tuple to copy from */
	mem_heap_t*	heap);	/*!< in: memory heap
				where the tuple is created */
/**********************************************************//**
The following function returns the sum of data lengths of a tuple. The space
occupied by the field structs or the tuple struct is not counted.
@return	sum of data lens */
IB_INLINE
ulint
dtuple_get_data_size(
/*=================*/
	const dtuple_t*	tuple,	/*!< in: typed data tuple */
	ulint		comp);	/*!< in: nonzero=ROW_FORMAT=COMPACT  */
/*********************************************************************//**
Computes the number of externally stored fields in a data tuple.
@return	number of fields */
IB_INLINE
ulint
dtuple_get_n_ext(
/*=============*/
	const dtuple_t*	tuple);	/*!< in: tuple */
/************************************************************//**
Compare two data tuples, respecting the collation of character fields.
@return	1, 0 , -1 if tuple1 is greater, equal, less, respectively,
than tuple2 */
IB_INTERN
int
dtuple_coll_cmp(
/*============*/
	void*		cmp_ctx,/*!< in: client compare context */
	const dtuple_t*	tuple1,	/*!< in: tuple 1 */
	const dtuple_t*	tuple2);/*!< in: tuple 2 */
/************************************************************//**
Folds a prefix given as the number of fields of a tuple.
@return	the folded value */
IB_INLINE
ulint
dtuple_fold(
/*========*/
	const dtuple_t*	tuple,	/*!< in: the tuple */
	ulint		n_fields,/*!< in: number of complete fields to fold */
	ulint		n_bytes,/*!< in: number of bytes to fold in an
				incomplete last field */
	dulint		tree_id)/*!< in: index tree id */
	__attribute__((pure));
/*******************************************************************//**
Sets types of fields binary in a tuple. */
IB_INLINE
void
dtuple_set_types_binary(
/*====================*/
	dtuple_t*	tuple,	/*!< in: data tuple */
	ulint		n);	/*!< in: number of fields to set */
/**********************************************************************//**
Checks if a dtuple contains an SQL null value.
@return	TRUE if some field is SQL null */
IB_INLINE
ibool
dtuple_contains_null(
/*=================*/
	const dtuple_t*	tuple);	/*!< in: dtuple */
/**********************************************************//**
Checks that a data field is typed. Asserts an error if not.
@return	TRUE if ok */
IB_INTERN
ibool
dfield_check_typed(
/*===============*/
	const dfield_t*	field);	/*!< in: data field */
/**********************************************************//**
Checks that a data tuple is typed. Asserts an error if not.
@return	TRUE if ok */
IB_INTERN
ibool
dtuple_check_typed(
/*===============*/
	const dtuple_t*	tuple);	/*!< in: tuple */
/**********************************************************//**
Checks that a data tuple is typed.
@return	TRUE if ok */
IB_INTERN
ibool
dtuple_check_typed_no_assert(
/*=========================*/
	const dtuple_t*	tuple);	/*!< in: tuple */
#ifdef IB_DEBUG
/**********************************************************//**
Validates the consistency of a tuple which must be complete, i.e,
all fields must have been set.
@return	TRUE if ok */
IB_INTERN
ibool
dtuple_validate(
/*============*/
	const dtuple_t*	tuple);	/*!< in: tuple */
#endif /* IB_DEBUG */
/*************************************************************//**
Pretty prints a dfield value according to its data type. */
IB_INTERN
void
dfield_print(
/*=========*/
	const dfield_t*	dfield);/*!< in: dfield */
/*************************************************************//**
Pretty prints a dfield value according to its data type. Also the hex string
is printed if a string contains non-printable characters. */
IB_INTERN
void
dfield_print_also_hex(
/*==================*/
	const dfield_t*	dfield);	 /*!< in: dfield */
/**********************************************************//**
The following function prints the contents of a tuple. */
IB_INTERN
void
dtuple_print(
/*=========*/
	ib_stream_t	state->stream,	/*!< in: output stream */
	const dtuple_t*	tuple);		/*!< in: tuple */
/**************************************************************//**
Moves parts of long fields in entry to the big record vector so that
the size of tuple drops below the maximum record size allowed in the
database. Moves data only from those fields which are not necessary
to determine uniquely the insertion place of the tuple in the index.
@return own: created big record vector, NULL if we are not able to
shorten the entry enough, i.e., if there are too many fixed-length or
short fields in entry or the index is clustered */
IB_INTERN
big_rec_t*
dtuple_convert_big_rec(
/*===================*/
	dict_index_t*	index,	/*!< in: index */
	dtuple_t*	entry,	/*!< in/out: index entry */
	ulint*		n_ext);	/*!< in/out: number of
				externally stored columns */
/**************************************************************//**
Puts back to entry the data stored in vector. Note that to ensure the
fields in entry can accommodate the data, vector must have been created
from entry with dtuple_convert_big_rec. */
IB_INTERN
void
dtuple_convert_back_big_rec(
/*========================*/
	dict_index_t*	index,	/*!< in: index */
	dtuple_t*	entry,	/*!< in: entry whose data was put to vector */
	big_rec_t*	vector);/*!< in, own: big rec vector; it is
				freed in this function */
/**************************************************************//**
Frees the memory in a big rec vector. */
IB_INLINE
void
dtuple_big_rec_free(
/*================*/
	big_rec_t*	vector);/*!< in, own: big rec vector; it is
				freed in this function */
/*************************************************************************
Reset dfield variables. */
IB_INTERN
void
dfield_var_init(void);
/*=================*/

/*######################################################################*/

/** Structure for an SQL data field */
struct dfield_struct{
	void*		data;	/*!< pointer to data */
	unsigned	ext:1;	/*!< TRUE=externally stored, FALSE=local */
	unsigned	len:32;	/*!< data length; IB_SQL_NULL if SQL null */
	dtype_t		type;	/*!< type of data */
};

/** Structure for an SQL data tuple of fields (logical record) */
struct dtuple_struct {
	ulint		info_bits;	/*!< info bits of an index record:
					the default is 0; this field is used
					if an index record is built from
					a data tuple */
	ulint		n_fields;	/*!< number of fields in dtuple */
	ulint		n_fields_cmp;	/*!< number of fields which should
					be used in comparison services
					of rem0cmp.*; the index search
					is performed by comparing only these
					fields, others are ignored; the
					default value in dtuple creation is
					the same value as n_fields */
	dfield_t*	fields;		/*!< fields */
	UT_LIST_NODE_T(dtuple_t) tuple_list;
					/*!< data tuples can be linked into a
					list using this field */
#ifdef IB_DEBUG
	ulint		magic_n;	/*!< magic number, used in
					debug assertions */
/** Value of dtuple_struct::magic_n */
# define		DATA_TUPLE_MAGIC_N	65478679
#endif /* IB_DEBUG */
};

/** A slot for a field in a big rec vector */
typedef struct big_rec_field_struct	big_rec_field_t;
/** A slot for a field in a big rec vector */
struct big_rec_field_struct {
	ulint		field_no;	/*!< field number in record */
	ulint		len;		/*!< stored data length, in bytes */
	const void*	data;		/*!< stored data */
};

/** Storage format for overflow data in a big record, that is, a
clustered index record which needs external storage of data fields */
struct big_rec_struct {
	mem_heap_t*	heap;		/*!< memory heap from which
					allocated */
	ulint		n_fields;	/*!< number of stored fields */
	big_rec_field_t*fields;		/*!< stored fields */
};

#ifndef IB_DO_NOT_INLINE
#include "data0data.inl"
#endif

#endif
