// MIT License
//
// Copyright (c) 2025 Fabio N. Filasieno
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// \file data_types.hpp
/// \brief Data module types header file
/// \author Fabio N. Filasieno
/// \date 2025-10-13

#pragma once


// Structure for an SQL data field
struct dfield_struct{
	void*		data;	/*!< pointer to data */
	unsigned	ext:1;	/*!< TRUE=externally stored, FALSE=local */
	unsigned	len:32;	/*!< data length; IB_SQL_NULL if SQL null */
	dtype_t		type;	/*!< type of data */
};

constinit ulint DATA_TUPLE_MAGIC_N = 65478679;

// Structure for an SQL data tuple of fields (logical record)
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
// Value of dtuple_struct::magic_n

#endif // IB_DEBUG
};

/// A slot for a field in a big rec vector
typedef struct big_rec_field_struct	big_rec_field_t;

/// A slot for a field in a big rec vector
struct big_rec_field_struct {
	ulint		field_no;	/*!< field number in record */
	ulint		len;		/*!< stored data length, in bytes */
	const void*	data;		/*!< stored data */
};

/// Storage format for overflow data in a big record, that is, a
clustered index record which needs external storage of data fields
struct big_rec_struct {
	mem_heap_t*	heap;		/*!< memory heap from which allocated */
	ulint		n_fields;	/*!< number of stored fields */
	big_rec_field_t*fields;		/*!< stored fields */
};

/// \brief Storage for overflow data in a big record.
/// \details A clustered index record which needs external storage of data fields.
typedef struct big_rec_struct big_rec_t;


/// \brief Structure for an SQL data type.
/// \details 
/// If you add fields to this structure, be sure to initialize them everywhere.
/// This structure is initialized in the following functions:
/// 
///  - `dtype_set()`
///  - `dtype_read_for_order_and_null_size()`
///  - `dtype_new_read_for_order_and_null_size()`
///  - `sym_tab_add_null_lit()`
/// The following are used in two places, dtype_t and  dict_field_t, we
/// want to ensure that they are identical and also want to ensure that
/// all bit-fields can be packed tightly in both structs. 
/// The following fields do not affect alphabetical ordering: len, mbminline, mbmaxlen
/// \field mtype main data type
/// \field precise type; user data type, charset code, flags to indicate nullability, signedness, whether this is a binary string
/// \field len length
/// \field mbminlen minimum length of a character, in bytes
/// \field mbmaxlen maximum length of bytes to store the string length)
struct dtype_struct
{
    unsigned mtype : 8;
    unsigned prtype : 24;

    unsigned len : 16;
    unsigned mbminlen : 2;
    unsigned mbmaxlen : 3;
};


// SQL data field struct 
typedef struct dfield_struct	dfield_t;

// SQL data tuple struct 
typedef struct dtuple_struct	dtuple_t;
