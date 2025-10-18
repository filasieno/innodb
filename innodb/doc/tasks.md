
# Refactoring Constraints for InnoDB Module

**IMPORTANT:** All constraints must be applied manually without using any automated tools (sed, python, bash, etc.). Each change must be double-checked for correctness before proceeding.

## Documentation Constraints

This is the documentation style that my srouce code has.

### Constraint: Function Documentation Style 1


```C++
/**
Frees the space in a memory heap exceeding the pointer given. The
pointer must have been acquired from mem_heap_get_heap_top. The first
memory block of the heap is not freed. */
IB_INLINE
void
mem_heap_free_heap_top(
/*===================*/
	mem_heap_t*	heap,	/*!< in: heap from which to free */
	byte*		old_top);/*!< in: pointer to old top of heap */
```

It is unreadable for humans.

I prefere this style:

```C++
/// \brief Frees the space in a memory heap exceeding the pointer given. 
/// \details The pointer must have been acquired from mem_heap_get_heap_top. The first memory block of the heap is not freed.
/// \param [in] heap from which to free
/// \param [in] old_top pointer to old top of heap
IB_INLINE void mem_heap_free_heap_top(mem_heap_t* heap, byte* old_top);
```

If a comment has a `.` split the long comment in `\brief` and `\details`.

### Constraint: Function Documentation Style 2

**MUST:** Convert all function documentation from block-style `/** ... */` comments to line-style `///` comments. Move inline parameter documentation `/*!< ... */` to `\param [in/out]` tags in the documentation block above the function declaration. Place the complete function signature (IB_INLINE, return type, function name, and all parameters) on a single line with no tabs in the parameter list. Double-check each conversion for accuracy.

**BEFORE:**

```C++
/**
Frees the space in a memory heap exceeding the pointer given. The
pointer must have been acquired from mem_heap_get_heap_top. The first
memory block of the heap is not freed. */
IB_INLINE
void
mem_heap_free_heap_top(
/*===================*/
	mem_heap_t*	heap,	/*!< in: heap from which to free */
	byte*		old_top);/*!< in: pointer to old top of heap */
```

**AFTER:**

```C++
/// \brief Frees the space in a memory heap exceeding the pointer given. 
/// \param [in] heap from which to free
/// \param [in] old_top pointer to old top of heap
/// \details The pointer must have been acquired from mem_heap_get_heap_top. The first memory block of the heap is not freed.
IB_INLINE void mem_heap_free_heap_top(mem_heap_t* heap, byte* old_top);
```

### Constraint: Long Comment Structure

**MUST:** For function comments longer than one sentence, split into `\brief` and `\details` sections using a period (.) as separator. Double-check that the brief section captures the essential function purpose.

### Block Comment Conversion

**MUST:** Replace any `/** ... */` block comments found within function bodies with equivalent `//` line comments; use long lines of 256 columns; never go to the next line before 256 chars. Double-check that all comment content is preserved accurately.

### File Header Documentation

**MUST:** Convert file header documentation from block-style to proper Doxygen format using `/// \file`, `/// \brief`, `/// \details`, `/// \author`, and `/// \date` tags. Recognize original creation information and establish current ownership. Double-check that file paths and authorship information are correctly updated.

**BEFORE:**

```C++
/******************************************************************//**
@file include/fut0fut.h
File-based utilities

Created 12/13/1995 Heikki Tuuri
***********************************************************************/
```

**AFTER:**

```C++
/// \file fut_fut.hpp
/// \brief File-based utilities
/// \details Originally created by Heikki Tuuri in 12/13/1995 
/// \author Fabio N. Filasieno
/// \date 20/10/2025
```

### Documentation Integration

**MUST:** When a function has documentation in both header (.hpp) and implementation (.cpp/.inl) files, integrate the information as necessary to ensure complete and accurate documentation in the header file. Double-check that no important details are lost during integration.

## Licensing Constraints

### License Header Format

**MUST:** Convert license headers from block-style `/* ... */` comments to line-style `//` comments. Maintain exact copyright text and license terms. Double-check that all license text remains identical to the original.

**BEFORE:**

```C++
/*****************************************************************************

Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/
```

**AFTER:**

```C++
// Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.
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
```

## Header Guards Constraints

### Include Guard Replacement

**MUST:** Replace all `#ifndef/#define/#endif` include guard patterns in header files with `#pragma once`. Double-check that the replacement is applied consistently across all header files.

**BEFORE:**

```C++
#ifndef FUT_FUT_HPP
#define FUT_FUT_HPP

// header content...

#endif /* FUT_FUT_HPP */
```

**AFTER:**

```C++
#pragma once

// header content...
```

## Macro Definition Constraints

### Integer Macro Constants

**MUST:** Replace all integer macro constant definitions with `constinit ulint MACRO_NAME = MACRO_VALUE;` syntax. Double-check that the macro name and value are correctly transcribed.

**BEFORE:**

```C++
#define FUT_PAGE_SIZE 16384
#define FUT_MAX_PAGES 1000
```

**AFTER:**

```C++
constinit ulint FUT_PAGE_SIZE = 16384;
constinit ulint FUT_MAX_PAGES = 1000;
```

## Code Style Constraints

### Tab Usage

**MUST:** Remove all tabs from function implementations except for the initial indentation sequence. Double-check that indentation remains consistent and readable.

**BEFORE:**

```C++
void some_function(
	int	param1,		/*!< parameter 1 */
	char*	param2)		/*!< parameter 2 */
{
	int	local_var	= 0;
}
```

**AFTER:**
```C++
void some_function(int param1, char* param2)
{
    int local_var = 0;
}
```

### Variable Declaration Timing

**MUST:** Refactor to C++ style by declaring variables at the point of initialization, not at the beginning of functions. Only keep declarations at the top when there's a specific reason (like output parameters). Process one function at a time. Double-check each variable relocation for correctness.

**BEFORE:**

```C++
void process_data()
{
    int result;
    char* buffer;

    buffer = allocate_buffer();
    result = process_buffer(buffer);
    return result;
}
```

**AFTER:**

```C++
void process_data()
{
    char* buffer = allocate_buffer();
    int result = process_buffer(buffer);
    return result;
}
```

### String Literal Concatenation

**MUST:** Convert sequences of adjacent string literals `"" ""` into single concatenated strings. Double-check that string content remains unchanged.

**BEFORE:**

```C++
ib_log(state, "File operation failed: " "xxxx");
   
```

**AFTER:**

```C++
ib_log(state, "File operation failed: xxxx");
```

### Function Call Formatting

**MUST:** Ensure all function calls are placed on a single line. Double-check that long function calls are not unnecessarily split across multiple lines.

**BEFORE:**

```C++
some_function_call(param1,
                   param2,
                   param3);
```

**AFTER:**

```C++
some_function_call(param1, param2, param3);
```

### Constant


### Empty Line Management

**MUST:** Remove empty lines within routines. Double-check that code readability is maintained.

**BEFORE:**

```C++
void process_data()
{

    validate_input();

    process_step1();

    process_step2();

    finalize_result();

}
```

**AFTER:**

```C++
void process_data()
{
    validate_input();
    process_step1();
    process_step2();
    finalize_result();
}
```

### Constraint: Comment Width

**MUST:** Extend comments to roughly 256 columns width when necessary to minimize the number of lines while maintaining readability, but never exceed 256 chars and go to the next line if a `dot` if found or in general of the paragraph.
Make sure:

- that all comments go to the next line if a dot is found
- never exceed 256 chars for comments

Double-check that extended comments remain clear and properly formatted.

**BEFORE:**

```C++
// This is a long comment that spans multiple lines
// and contains detailed information about the function
// behavior and parameters
```

**AFTER:**

```C++
// This is a long comment that spans multiple lines and contains detailed information about the function behavior and parameters extending to 
// minimize line count while maintaining readability
```

## File Organization Constraints

### Constraint 15: Static Function Declarations

**MUST:** Write declarations for all static functions at the beginning of .cpp files. For functions with `#ifdef/#ifndef` conditional compilation, duplicate the conditional blocks for both declarations and definitions. Double-check that declarations match their corresponding definitions exactly.

**BEFORE:**

```C++

/// \brief doxygen comments here
static void helper_function(int param)
{
    // implementation
}

void module_function()
{
    helper_function(42);
}

```

**AFTER:**

```C++
// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

/// \brief doxygen comments here
static void helper_function(int param);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

void main_function()
{
    helper_function(42);
}

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

static void helper_function(int param)
{
    // implementation
}
```

### Constraint: File Section Organization

**MUST:** Organize .inl and .cpp files into the following sections using non-Doxygen comment separators. Double-check that all functions are placed in their correct sections and that the file structure is consistent.

**TARGET STRUCTURE:**

```C++
// ... other code

// -----------------------------------------------------------------------------------------
// type definitions
// -----------------------------------------------------------------------------------------

// < TYPE DEFINITIONS HERE >

// -----------------------------------------------------------------------------------------
// macro constants
// -----------------------------------------------------------------------------------------

// < MACRO CONSTANTS HERE >

// -----------------------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------------------

// < BOTH STATIC AND NON STATIC >

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

// <STATIC FUNCTION DECLARATIONS HERE>

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------


// <NON-STATIC FUNCTION IMPLEMENTATIONS HERE>

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------


// <STATIC FUNCTION IMPLEMENTATIONS HERE>
```

### Constraint: return statement

Often you'll find a useless group in parentheisis like:

```C++
return (...);
```
replace it with: 

```C++
return ...;
```

### Constraint: Tabs and fields

Use tabs sequences as indentation and spaces as alignment and ensure that there are no spaces at the end of the line 

