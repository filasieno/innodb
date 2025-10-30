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

/// \file mem_types.hpp
/// \brief mem module types header file
/// \author Fabio N. Filasieno
/// \date 2025-10-13

#pragma once

/// \struct mem_block_info_struct
/// \brief The info structure stored at the beginning of a heap block
///
/// This structure contains metadata and control information for memory blocks
/// used in the InnoDB memory management system. It is stored at the beginning
/// of each heap block and contains information necessary for memory management,
/// debugging, and list traversal.

/// \var mem_block_info_struct::magic_n
/// \brief Magic number for debugging
/// \var mem_block_info_struct::file_name
/// \brief File name where the memory heap was created
/// \var mem_block_info_struct::line
/// \brief Line number where the memory heap was created
/// \var mem_block_info_struct::base
/// \brief Base node for the list of blocks (defined only in the first block)
/// \var mem_block_info_struct::list
/// \brief List node pointers to next and previous blocks
/// \var mem_block_info_struct::len
/// \brief Physical length of this block in bytes
/// \var mem_block_info_struct::total_size
/// \brief Total physical length of all blocks in the heap (defined only in base node)
/// \var mem_block_info_struct::type
/// \brief Type of heap: MEM_HEAP_DYNAMIC, MEM_HEAP_BUF possibly ORed to MEM_HEAP_BTR_SEARCH
/// \var mem_block_info_struct::free
/// \brief Offset in bytes of the first free position for user data in the block
/// \var mem_block_info_struct::start
/// \brief The value of the 'free' field at block creation
/// \var mem_block_info_struct::free_block
/// \brief Free buffer frame for B-tree search heaps, appended as free block when needed
/// \var mem_block_info_struct::buf_block
/// \brief Buffer pool block handle if allocated from buffer pool, otherwise NULL
/// \var mem_block_info_struct::mem_block_list
/// \brief List of all allocated memory blocks, protected by mem_comm_pool mutex

typedef struct mem_block_info_struct mem_block_info_t;

struct mem_block_info_struct {
	ulint	magic_n;
	char	file_name[8];
	ulint	line;
	UT_LIST_BASE_NODE_T(mem_block_t) base;
	UT_LIST_NODE_T(mem_block_t) list;
	ulint	len;
	ulint	total_size;
	ulint	type;
	ulint	free;
	ulint	start;
#ifndef IB_HOTBACKUP
	void*	free_block;
	void*	buf_block;
#endif // !IB_HOTBACKUP
#ifdef IB_DEBUG
	UT_LIST_NODE_T(mem_block_t) mem_block_list;
#endif // IB_DEBUG
};