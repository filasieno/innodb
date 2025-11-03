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

/// \file fut_lst.hpp
/// \brief File-based list utilities
/// \details Originally created by Heikki Tuuri in 11/28/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"

#include "fil_fil.hpp"
#include "mtr_mtr.hpp"

// The C 'types' of base node and list node: these should be used to write self-documenting code. Of course, the sizeof macro cannot be applied to these types!
typedef byte flst_base_node_t;
typedef byte flst_node_t;

// The physical size of a list base node in bytes
constinit ulint FLST_BASE_NODE_SIZE = 4 + 2 * FIL_ADDR_SIZE;

// The physical size of a list node in bytes
constinit ulint FLST_NODE_SIZE = 2 * FIL_ADDR_SIZE;

#ifndef IB_HOTBACKUP

/// \brief Initializes a list base node.
/// \param [in] base pointer to base node
/// \param [in] mtr mini-transaction handle
IB_INLINE void flst_init(flst_base_node_t* base, mtr_t* mtr);

/// \brief Adds a node as the last node in a list.
/// \param [in] base pointer to base node of list
/// \param [in] node node to add
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_add_last(flst_base_node_t* base, flst_node_t* node, mtr_t* mtr);

/// \brief Adds a node as the first node in a list.
/// \param [in] base pointer to base node of list
/// \param [in] node node to add
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_add_first(flst_base_node_t* base, flst_node_t* node, mtr_t* mtr);

/// \brief Inserts a node after another in a list.
/// \param [in] base pointer to base node of list
/// \param [in] node1 node to insert after
/// \param [in] node2 node to add
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_insert_after(flst_base_node_t* base, flst_node_t* node1, flst_node_t* node2, mtr_t* mtr);

/// \brief Inserts a node before another in a list.
/// \param [in] base pointer to base node of list
/// \param [in] node2 node to insert
/// \param [in] node3 node to insert before
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_insert_before(flst_base_node_t* base, flst_node_t* node2, flst_node_t* node3, mtr_t* mtr);

/// \brief Removes a node.
/// \param [in] base pointer to base node of list
/// \param [in] node2 node to remove
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_remove(flst_base_node_t* base, flst_node_t* node2, mtr_t* mtr);

/// \brief Cuts off the tail of the list, including the node given.
/// \param [in] base pointer to base node of list
/// \param [in] node2 first node to remove
/// \param [in] n_nodes number of nodes to remove, must be >= 1
/// \param [in] mtr mini-transaction handle
/// \details The number of nodes which will be removed must be provided by the caller, as this function does not measure the length of the tail.
IB_INTERN void flst_cut_end(flst_base_node_t* base, flst_node_t* node2, ulint n_nodes, mtr_t* mtr);

/// \brief Cuts off the tail of the list, not including the given node.
/// \param [in] base pointer to base node of list
/// \param [in] node2 first node not to remove
/// \param [in] n_nodes number of nodes to remove
/// \param [in] mtr mini-transaction handle
/// \details The number of nodes which will be removed must be provided by the caller, as this function does not measure the length of the tail.
IB_INTERN void flst_truncate_end(flst_base_node_t* base, flst_node_t* node2, ulint n_nodes, mtr_t* mtr);

/// \brief Gets list length.
/// \param [in] base pointer to base node
/// \param [in] mtr mini-transaction handle
/// \return length
IB_INLINE ulint flst_get_len(const flst_base_node_t* base, mtr_t* mtr);

/// \brief Gets list first node address.
/// \param [in] base pointer to base node
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_get_first(const flst_base_node_t* base, mtr_t* mtr);

/// \brief Gets list last node address.
/// \param [in] base pointer to base node
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_get_last(const flst_base_node_t* base, mtr_t* mtr);

/// \brief Gets list next node address.
/// \param [in] node pointer to node
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_get_next_addr(const flst_node_t* node, mtr_t* mtr);

/// \brief Gets list prev node address.
/// \param [in] node pointer to node
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_get_prev_addr(const flst_node_t* node, mtr_t* mtr);

/// \brief Writes a file address.
/// \param [in] faddr pointer to file faddress
/// \param [in] addr file address
/// \param [in] mtr mini-transaction handle
IB_INLINE void flst_write_addr(fil_faddr_t* faddr, fil_addr_t addr, mtr_t* mtr);

/// \brief Reads a file address.
/// \param [in] faddr pointer to file faddress
/// \param [in] mtr mini-transaction handle
/// \return file address

IB_INLINE fil_addr_t flst_read_addr(const fil_faddr_t* faddr, mtr_t* mtr);
/// \brief Validates a file-based list.
/// \param [in] base pointer to base node of list
/// \param [in] mtr1 mtr
/// \return TRUE if ok

IB_INTERN ibool flst_validate(const flst_base_node_t* base, mtr_t* mtr1);
/// \brief Prints info of a file-based list.
/// \param [in] base pointer to base node of list
/// \param [in] mtr mtr
IB_INTERN void flst_print(const flst_base_node_t* base, mtr_t* mtr);


#ifndef IB_DO_NOT_INLINE
#include "fut0lst.inl"
#endif

#endif // !IB_HOTBACKUP
