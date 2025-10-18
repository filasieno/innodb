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

/// \file fsp_fsp.inl
/// \brief File space management inline functions
/// \details Originally created by Heikki Tuuri in 12/18/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

/// \brief Checks if a page address is an extent descriptor page address.
/// \return TRUE if a descriptor page
/// \param [in] zip_size compressed page size in bytes; 0 for uncompressed pages
/// \param [in] page_no page number
IB_INLINE ibool fsp_descr_page(ulint zip_size, ulint page_no)
{
	ut_ad(ut_is_2pow(zip_size));

	if (!zip_size) {
		return(IB_UNLIKELY((page_no & (IB_PAGE_SIZE - 1))
				     == FSP_XDES_OFFSET));
	}

	return(IB_UNLIKELY((page_no & (zip_size - 1)) == FSP_XDES_OFFSET));
}
