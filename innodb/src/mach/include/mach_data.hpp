// Copyright (c) 1995, 2025, Fabio N. Filasieno. All Rights Reserved.
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
// Place, Suite 330, Boston, MA 02111-1307 USA


/// \file mach_data.h
/// \brief Utilities for converting data from the database file to the machine format.
/// \details 
/// The data and all fields are always stored in a database file
/// in the same format: ascii, big-endian, ... .
/// All data in the files MUST be accessed using the functions in this module.
/// 
/// Updated 10/10/2025 Fabio N. Filasieno
/// Created 11/28/1995 Heikki Tuuri

#pragma once

#include "univ.i"
#include "ut_byte.hpp"

/// \brief The following function is used to store data in one byte.
/// \param [in] b pointer to byte where to store
/// \param [in] n ulint integer to be stored, >= 0, < 256
IB_INLINE void mach_write_to_1(byte* b, ulint n);

/// \brief The following function is used to fetch data from one byte.
/// \param [in] b pointer to byte
/// \return ulint integer, >= 0, < 256
IB_INLINE ulint mach_read_from_1(const byte* b);

/// \brief The following function is used to store data in two consecutive bytes.
/// \param [in] b pointer to two bytes where to store
/// \param [in] n ulint integer to be stored, >= 0, < 64k
/// \return void
IB_INLINE void mach_write_to_2(byte* b, ulint n);

/// \brief The following function is used to fetch data from two consecutive bytes.
/// \param [in] b pointer to two bytes
/// \return ulint integer, >= 0, < 64k
IB_INLINE ulint mach_read_from_2(const byte* b) __attribute__((nonnull, pure));

/// \brief The following function is used to convert a 16-bit data item to the canonical format, for fast bytewise equality test against memory.
/// \param [in] n ulint integer to be stored, >= 0, < 64k
/// \return 16-bit integer in canonical format
IB_INLINE ib_uint16_t mach_encode_2(ulint n) __attribute__((const));

/// \brief The following function is used to convert a 16-bit data item from the canonical format, for fast bytewise equality test against memory.
/// \param [in] n 16-bit integer in canonical format
/// \return integer in machine-dependent format
IB_INLINE ulint mach_decode_2(ib_uint16_t n) __attribute__((const));

/// \brief The following function is used to store data in 3 consecutive bytes. We store the most significant byte to the lowest address.
/// \param [in] b pointer to 3 bytes where to store
/// \param [in] n ulint integer to be stored
/// \return void
IB_INLINE void mach_write_to_3(byte* b, ulint n) __attribute__((nonnull));

// The following function is used to store data in 3 consecutive bytes. We store the most significant byte to the lowest address.
/// \param [in] b pointer to 3 bytes where to store
/// \param [in] n ulint integer to be stored
/// \return void
IB_INLINE void mach_write_to_3(byte* b, ulint n) __attribute__((nonnull));

/// \brief The following function is used to fetch data from 3 consecutive bytes. The most significant byte is at the lowest address.
/// \param [in] b pointer to 3 bytes
/// \return ulint integer
IB_INLINE ulint mach_read_from_3(const byte* b) __attribute__((nonnull, pure));

/// \brief The following function is used to store data in four consecutive bytes. We store the most significant byte to the lowest address.
/// \param [in] b pointer to 4 bytes where to store
/// \param [in] n ulint integer to be stored
/// \return void
IB_INLINE void mach_write_to_4(byte* b, ulint n) __attribute__((nonnull));

/// \brief The following function is used to fetch data from 4 consecutive bytes. The most significant byte is at the lowest address.
/// \param [in] b pointer to 4 bytes
/// \return ulint integer
IB_INLINE ulint mach_read_from_4(const byte* b) __attribute__((nonnull, pure));

/// \brief Writes a ulint in a compressed form (1..5 bytes).
/// \details Writes a ulint in a compressed form where the first byte codes the
/// length of the stored ulint. We look at the most significant bits of
/// the byte. If the most significant bit is zero, it means 1-byte storage,
/// else if the 2nd bit is 0, it means 2-byte storage, else if 3rd is 0,
/// it means 3-byte storage, else if 4th is 0, it means 4-byte storage,
/// else the storage is 5-byte.
/// \param [in] b pointer to memory where to store
/// \param [in] n ulint integer to be stored
/// \return compressed size in bytes
IB_INLINE ulint mach_write_compressed(byte* b, ulint n) __attribute__((nonnull));

/// \brief Returns the size of an ulint when written in the compressed form.
/// \param [in] n ulint integer to be stored
/// \return compressed size in bytes
IB_INLINE ulint mach_get_compressed_size(ulint n) __attribute__((const));

/// \brief Reads a ulint in a compressed form.
/// \param [in] b pointer to memory from where to read
/// \return read integer
IB_INLINE ulint mach_read_compressed(const byte* b) __attribute__((nonnull, pure));

/// \brief Reads a ulint in a compressed form.
/// \param [in] b pointer to memory from where to read
/// \return read integer
IB_INLINE ulint mach_read_compressed(const byte* b) __attribute__((nonnull, pure));

/// \brief The following function is used to store data in 6 consecutive bytes.
/// We store the most significant byte to the lowest address.
/// \param [in] b pointer to 6 bytes where to store
/// \param [in] n dulint integer to be stored
IB_INLINE void mach_write_to_6(byte* b, dulint n);

/// \brief The following function is used to fetch data from 6 consecutive bytes.
/// The most significant byte is at the lowest address.
/// \param [in] b pointer to 6 bytes
/// \return dulint integer
IB_INLINE dulint mach_read_from_6(const byte* b) __attribute__((nonnull, pure));

/// \brief The following function is used to store data in 7 consecutive bytes.
/// We store the most significant byte to the lowest address.
/// \param [in] b pointer to 7 bytes where to store
/// \param [in] n dulint integer to be stored
IB_INLINE void mach_write_to_7(byte* b, dulint n);

/// \brief The following function is used to fetch data from 7 consecutive bytes.
/// The most significant byte is at the lowest address.
/// \param [in] b pointer to 7 bytes
/// \return dulint integer
IB_INLINE dulint mach_read_from_7(const byte* b) __attribute__((nonnull, pure));

/// \brief The following function is used to store data in 8 consecutive bytes.
/// We store the most significant byte to the lowest address.
/// \param [in] b pointer to 8 bytes where to store
/// \param [in] n dulint integer to be stored
IB_INLINE void mach_write_to_8(byte* b, dulint n);

/// \brief The following function is used to store data in 8 consecutive bytes.
/// We store the most significant byte to the lowest address.
/// \param [in] b pointer to 8 bytes where to store
/// \param [in] n 64-bit integer to be stored
IB_INLINE void mach_write_ull(byte* b, ib_uint64_t n);

/// \brief The following function is used to fetch data from 8 consecutive bytes.
/// The most significant byte is at the lowest address.
/// \param [in] b pointer to 8 bytes
/// \return dulint integer
IB_INLINE dulint mach_read_from_8(const byte* b) __attribute__((nonnull, pure));

/// \brief The following function is used to fetch data from 8 consecutive bytes.
/// The most significant byte is at the lowest address.
/// \param [in] pointer to 8 bytes
/// \return 64-bit integer 
IB_INLINE ib_uint64_t mach_read_ull(const byte* b) __attribute__((nonnull, pure));

/// \brief Writes a dulint in a compressed form (5..9 bytes).
/// \param [in] b pointer to memory where to store
/// \param [in] n dulint integer to be stored
/// \return size in bytes
IB_INLINE ulint mach_dulint_write_compressed(byte* b, dulint n);

/// \brief Returns the size of a dulint when written in the compressed form.
/// \return compressed size in bytes 
/// \param [in] n dulint integer to be stored
IB_INLINE ulint mach_dulint_get_compressed_size(dulint n);

/// Reads a dulint in a compressed form.
/// \param [in] b pointer to memory from where to read
/// \return read dulint 
IB_INLINE dulint mach_dulint_read_compressed(const byte* b) __attribute__((nonnull, pure));

/// \brief Writes a dulint in a compressed form (1..11 bytes).
/// \param [in] b pointer to memory where to store
/// \param [in] n integer to be stored 
/// \return size in bytes 
IB_INLINE ulint mach_dulint_write_much_compressed(byte* b, dulint n); /*!< in: */

/// \brief Returns the size of a dulint when written in the compressed form.
/// \return compressed size in bytes 
/// \param [in] n dulint integer to be stored
IB_INLINE ulint mach_dulint_get_much_compressed_size(dulint n) __attribute__((const));

/// \brief Reads a dulint in a compressed form.
/// \param [in] b pointer to memory from where to read
/// \return read dulint
IB_INLINE dulint mach_dulint_read_much_compressed(const byte* b) __attribute__((nonnull, pure));

/// \brief Reads a ulint in a compressed form if the log record fully contains it.
/// \param [in] ptr pointer to buffer from where to read
/// \param [in] end_ptr pointer to end of the buffer
/// \param [out] val read value
/// \return pointer to end of the stored field, NULL if not complete
UNIV_INTERN byte* mach_parse_compressed(byte* ptr, byte* end_ptr, ulint* val);

/// \brief Reads a dulint in a compressed form if the log record fully contains it.
/// \param [in] ptr pointer to buffer from where to read
/// \param [in] end_ptr pointer to end of the buffer
/// \param [out] val read value
/// \return pointer to end of the stored field, NULL if not complete
UNIV_INTERN byte* mach_dulint_parse_compressed(byte* ptr, byte* end_ptr, dulint* val);

#ifndef UNIV_HOTBACKUP

	/// \brief Reads a double value stored in little-endian format.
	/// \param [in] b pointer to memory from where to read
	/// \return double value read
	IB_INLINE double mach_double_read(const byte* b) __attribute__((nonnull, pure));

	/// \brief Writes a pointer to a double. It is stored in a little-endian format.
	/// \param [in] b pointer to memory where to write
	/// \param [in] ptr pointer to a double
	IB_INLINE void mach_double_ptr_write(byte* b, const byte* ptr);

	/// \brief Writes a double. It is stored in a little-endian format.
	/// \param [in] b pointer to memory where to write
	/// \param [in] d double
	IB_INLINE void mach_double_write(byte* b, double d);

	/// \brief Reads a float. It is stored in a little-endian format.
	/// \param [in] b pointer to memory from where to read
	/// \return float read
	IB_INLINE float mach_float_read(const byte* b) __attribute__((nonnull, pure));

	/// \brief Writes a pointer to float. It is stored in a little-endian format.
	/// \param [in] b pointer to memory where to write
	/// \param [in] p pointer to float
	IB_INLINE void mach_float_ptr_write(byte* b, const byte* p);

	/// \brief Writes a float. It is stored in a little-endian format.
	/// \param [in] b pointer to memory where to write
	/// \param [in] d float
	IB_INLINE void mach_float_write(byte* b, float d);

	/// \brief Reads a ulint stored in the little-endian format.
	/// \param [in] buf from where to read
	/// \param [in] buf_size from how many bytes to read
	/// \return unsigned long int
	IB_INLINE ulint mach_read_from_n_little_endian(const byte* buf, ulint buf_size) __attribute__((nonnull, pure));

	/// \brief Writes a ulint in the little-endian format.
	/// \param [in] dest where to write
	/// \param [in] dest_size into how many bytes to write
	/// \param [in] n unsigned long int to write
	IB_INLINE void mach_write_to_n_little_endian(byte* dest, ulint dest_size, ulint n);

	/// \brief Reads a ulint stored in the little-endian format.
	/// \param [in] buf from where to read
	/// \return unsigned long int
	IB_INLINE ulint mach_read_from_2_little_endian(const byte* buf) __attribute__((nonnull, pure));

	/// \brief Writes a ulint in the little-endian format.
	/// \param [in] dest where to write
	/// \param [in] n unsigned long int to write
	IB_INLINE void mach_write_to_2_little_endian(byte* dest, ulint n);

	/// \brief Convert integral type from storage byte order (big endian) to host byte order.
	/// \param [out] dst where to write
	/// \param [in] src where to read from
	/// \param [in] len length of src
	/// \param [in] unsigned_type signed or unsigned flag
	IB_INLINE void mach_read_int_type(void* dst, const byte* src, ulint len, ibool unsigned_type);

	/// \brief Convert integral type from host byte order to (big-endian) storage byte order.
	/// \param [in] dest where to write
	/// \param [in] src where to read from
	/// \param [in] len length of src
	/// \param [in] unsigned_type signed or unsigned flag
	IB_INLINE void mach_write_int_type(byte* dest, const byte* src, ulint len, ibool unsigned_type);

	/// \brief Convert a 64 bit unsigned integral type to big endian from host byte order.
	/// \param [out] dest where to write
	/// \param [in] n where to read from
	IB_INLINE void mach_write_uint64(byte* dest, ib_uint64_t n);

#endif // ! UNIV_HOTBACKUP 

#ifndef UNIV_NONINL
  #include "mach_data.inl"
#endif

#endif
