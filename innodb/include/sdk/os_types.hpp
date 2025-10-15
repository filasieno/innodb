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

/// \file os_types.hpp
/// \brief os module types header file
/// \author Fabio N. Filasieno
/// \date 2025-10-13

#pragma once

/// \brief OS state
/// \field sync_mutex       Mutex protecting counts and the event and OS 'slow' mutex lists
/// \field thread_count     This is incremented by 1 in os_thread_create and decremented by 1 in os_thread_exit
/// \field event_count      Number of events created
/// \field mutex_count      Number of OS 'slow' mutexes created
/// \field fast_mutex_count Number of OS fast mutexes created
struct os_state {
    os_mutex_t sync_mutex;
    ulint      thread_count;
    ulint      event_count;
    ulint      mutex_count;
    ulint      fast_mutex_count;
};