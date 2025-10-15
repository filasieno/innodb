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

/// \file state_types.hpp
/// \brief state module types header file
/// \author Fabio N. Filasieno
/// \date 2025-10-13

#pragma once

#include "innodb_types.hpp"
#include "os_types.hpp"

struct innodb_state {
    ib_logger_t log;
    ib_stream_t stream;
    os_state    os;

	/// \brief The total amount of memory currently allocated from the operating system with os_mem_alloc_large() or malloc().  
    /// \details Does not count malloc()
	/// 
    /// if `srv_use_sys_malloc` is set.  
    /// Protected by \ref ut_list_mutex.
	ulint ut_total_allocated_memory;

	/// \brief Mutex protecting `ut_total_allocated_memory` and `ut_mem_block_list`
	os_fast_mutex_t	ut_list_mutex;
};

constexpr void ib_logger(innodb_state* state, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    state->log(state->stream, fmt, args);
    va_end(args);
}