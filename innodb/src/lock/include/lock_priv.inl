// Copyright (c) 2007, 2009, Innobase Oy. All Rights Reserved.
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

/// \file lock_priv.inl
/// \brief Lock module internal inline methods.
/// \details Originally created by Vasil Dimov on July 16, 2007.
/// \author Fabio N. Filasieno
/// \date 2025-10-20

// This file contains only methods which are used in lock/lock0* files, other than lock/lock_lock.c.
// ie. lock_lock.c contains more internal inline methods but they are used only in that file.

#ifndef LOCK_MODULE_IMPLEMENTATION
	#error Do not include lock_priv.inl outside of the lock/ module
#endif

/// \brief Gets the type of a lock.
/// \return LOCK_TABLE or LOCK_REC
IB_INLINE ulint lock_get_type_low(const ib_lock_t* lock)
{
	ut_ad(lock);
	return(lock->type_mode & LOCK_TYPE_MASK);
}
