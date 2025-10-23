// Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.
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

/// \file dict_boot.inl
/// \brief Data dictionary creation and booting
/// \details Originally created by Heikki Tuuri in 4/18/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

/// \brief Writes the current value of the row id counter to the dictionary header file page.
IB_INTERN void dict_hdr_flush_row_id(void);

IB_INLINE dulint dict_sys_get_new_row_id(void)
{
    mutex_enter(&(dict_sys->mutex));
    dulint id = dict_sys->row_id;
    if (0 == (ut_dulint_get_low(id) % DICT_HDR_ROW_ID_WRITE_MARGIN)) {
        dict_hdr_flush_row_id();
    }
    UT_DULINT_INC(dict_sys->row_id);
    mutex_exit(&(dict_sys->mutex));
    return(id);
}

IB_INLINE dulint dict_sys_read_row_id(byte* field)
{
    static_assert(DATA_ROW_ID_LEN == 6, "DATA_ROW_ID_LEN != 6");
    return(mach_read_from_6(field));
}

IB_INLINE void dict_sys_write_row_id(byte* field, dulint row_id)
{
    static_assert(DATA_ROW_ID_LEN == 6, "DATA_ROW_ID_LEN != 6");
    mach_write_to_6(field, row_id);
}