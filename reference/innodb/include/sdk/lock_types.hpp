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

/// \file lock_types.hpp
/// \brief lock module types header file
/// \author Fabio N. Filasieno
/// \date 2025-10-13

#pragma once

/// \brief Basic lock modes.
enum ib_lock_mode {
	LOCK_IS = 0,         //!< intention shared
	LOCK_IX,             //!< intention exclusive
	LOCK_S,              //!< shared
	LOCK_X,              //!< exclusive
	LOCK_AUTO_INC,	     //!< locks the auto-inc counter of a table in an exclusive mode
	LOCK_NONE,	         //!< this is used elsewhere to note consistent read
	LOCK_NUM = LOCK_NONE //!< number of lock modes
};

struct ib_lock_queue_iterator_t {
	const ib_lock_t* current_lock;
	ulint		     bit_no;       //!< In case this is a record lock queue (not table lock queue) then bit_no is the record number within the heap in which the record is stored.
};

/// \brief Lock operation struct
typedef struct ib_lock_op_struct lock_op_t;

/// \struct ib_lock_op_struct Lock operation struct
struct ib_lock_op_struct{
	dict_table_t* table; //!< table to be locked
	enum ib_lock_mode mode; //!< lock mode
};

/** The lock system struct */
struct lock_sys_struct{
	hash_table_t* rec_hash;	/*!< hash table of the record locks */
};