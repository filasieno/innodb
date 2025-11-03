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

/// \file btr_types.hpp
/// \brief B-tree module types header file
/// \author Fabio N. Filasieno
/// \date 2025-10-13

#pragma once


/// \struct btr_pcur_struct
/// \brief The persistent B-tree cursor structure. This is used mainly for SQL selects, updates, and deletes.
/// \TODO: currently, the state can be BTR_PCUR_IS_POSITIONED, though it really should be BTR_PCUR_WAS_POSITIONED, because we have no obligation to commit the cursor with mtr; similarly latch_mode may be out of date. This can lead to problems if btr_pcur is not used the right way; all current code should be ok.
/// \var btr_cur_t btr_pcur_struct::btr_cur
/// \brief a B-tree cursor
///
/// \var ulint btr_pcur_struct::latch_mode
/// \brief see TODO note below! BTR_SEARCH_LEAF, BTR_MODIFY_LEAF, BTR_MODIFY_TREE, or BTR_NO_LATCHES, depending on the latching state of the page and tree where the cursor is positioned; the last value means that the cursor is not currently positioned: we say then that the cursor is detached; it can be restored to attached if the old position was stored in old_rec
///
/// \var ulint btr_pcur_struct::old_stored
/// \brief BTR_PCUR_OLD_STORED or BTR_PCUR_OLD_NOT_STORED
///
/// \var rec_t* btr_pcur_struct::old_rec
/// \brief if cursor position is stored, contains an initial segment of the latest record cursor was positioned either on, before, or after
///
/// \var ulint btr_pcur_struct::old_n_fields
/// \brief number of fields in old_rec
///
/// \var ulint btr_pcur_struct::rel_pos
/// \brief BTR_PCUR_ON, BTR_PCUR_BEFORE, or BTR_PCUR_AFTER, depending on whether cursor was on, before, or after the old_rec record
///
/// \var buf_block_t* btr_pcur_struct::block_when_stored
/// \brief buffer block when the position was stored
///
/// \var ib_uint64_t btr_pcur_struct::modify_clock
/// \brief the modify clock value of the buffer block when the cursor position was stored
///
/// \var ulint btr_pcur_struct::pos_state
/// \brief see TODO note below! BTR_PCUR_IS_POSITIONED, BTR_PCUR_WAS_POSITIONED, BTR_PCUR_NOT_POSITIONED
///
/// \var ulint btr_pcur_struct::search_mode
/// \brief PAGE_CUR_G, ...
///
/// \var trx_t* btr_pcur_struct::trx_if_known
/// \brief the transaction, if we know it; otherwise this field is not defined; can ONLY BE USED in error prints in fatal assertion failures!
///
/// \var mtr_t* btr_pcur_struct::mtr
/// \brief NULL, or this field may contain a mini-transaction which holds the latch on the cursor page; might have memory to free
///
/// \var byte* btr_pcur_struct::old_rec_buf
/// \brief NULL, or a dynamically allocated buffer for old_rec
///
/// \var ulint btr_pcur_struct::buf_size
/// \brief old_rec_buf size if old_rec_buf is not NULL

struct btr_pcur_struct{
	btr_cur_t    btr_cur;
	ulint        latch_mode;
	ulint        old_stored;
	rec_t*       old_rec;
	ulint        old_n_fields;
	ulint        rel_pos;
	buf_block_t* block_when_stored;
	ib_uint64_t	 modify_clock;
	ulint        pos_state;
	ulint        search_mode;
	trx_t*       trx_if_known;
	mtr_t*       mtr;
	byte*        old_rec_buf;
	ulint        buf_size;
};




/*######################################################################*/

/** In the pessimistic delete, if the page data size drops below this
limit, merging it to a neighbor is tried */
constinit ulint BTR_CUR_PAGE_COMPRESS_LIMIT = (IB_PAGE_SIZE / 2);

// -----------------------------------------------------------------------------------------
// type definitions
// -----------------------------------------------------------------------------------------

/** A slot in the path array. We store here info on a search path down the
tree. Each slot contains data on a single level of the tree. */

typedef struct btr_path_struct    btr_path_t;
struct btr_path_struct{
    ulint    nth_rec;    /*!< index of the record
				where the page cursor stopped on
				this level (index in alphabetical
				order); value ULINT_UNDEFINED
				denotes array end */
    ulint    n_recs;        /*!< number of records on the page */
};

constinit ulint BTR_PATH_ARRAY_N_SLOTS = 250;

/** Values for the flag documenting the used search method */
enum btr_cur_method {
    BTR_CUR_HASH = 1,    /*!< successful shortcut using
				the hash index */
    BTR_CUR_HASH_FAIL,    /*!< failure using hash, success using
				binary search: the misleading hash
				reference is stored in the field
				hash_node, and might be necessary to
				update */
    BTR_CUR_BINARY,        /*!< success using the binary search */
    BTR_CUR_INSERT_TO_IBUF    /*!< performed the intended insert to
				the insert buffer */
};

/** The tree cursor: the definition appears here only for the compiler
to know struct size! */
struct btr_cur_struct {
    dict_index_t*    index;        /*!< index where positioned */
    page_cur_t    page_cur;    /*!< page cursor */
    buf_block_t*    left_block;    /*!< this field is used to store
					a pointer to the left neighbor
					page, in the cases
					BTR_SEARCH_PREV and
					BTR_MODIFY_PREV */
	/*------------------------------*/
    que_thr_t*    thr;        /*!< this field is only used
					when btr_cur_search_to_nth_level
					is called for an index entry
					insertion: the calling query
					thread is passed here to be
					used in the insert buffer */
	/*------------------------------*/
	/** The following fields are used in
	btr_cur_search_to_nth_level to pass information: */
	/* @{ */
    enum btr_cur_method    flag;    /*!< Search method used */
    ulint        tree_height;    /*!< Tree height if the search is done
					for a pessimistic insert or update
					operation */
    ulint        up_match;    /*!< If the search mode was PAGE_CUR_LE,
					the number of matched fields to the
					the first user record to the right of
					the cursor record after
					btr_cur_search_to_nth_level;
					for the mode PAGE_CUR_GE, the matched
					fields to the first user record AT THE
					CURSOR or to the right of it;
					NOTE that the up_match and low_match
					values may exceed the correct values
					for comparison to the adjacent user
					record if that record is on a
					different leaf page! (See the note in
					row_ins_duplicate_key.) */
    ulint        up_bytes;    /*!< number of matched bytes to the
					right at the time cursor positioned;
					only used internally in searches: not
					defined after the search */
    ulint        low_match;    /*!< if search mode was PAGE_CUR_LE,
					the number of matched fields to the
					first user record AT THE CURSOR or
					to the left of it after
					btr_cur_search_to_nth_level;
					NOT defined for PAGE_CUR_GE or any
					other search modes; see also the NOTE
					in up_match! */
    ulint        low_bytes;    /*!< number of matched bytes to the
					right at the time cursor positioned;
					only used internally in searches: not
					defined after the search */
    ulint        n_fields;    /*!< prefix length used in a hash
					search if hash_node != NULL */
    ulint        n_bytes;    /*!< hash prefix bytes if hash_node !=
					NULL */
    ulint        fold;        /*!< fold value used in the search if
					flag is BTR_CUR_HASH */
	/*------------------------------*/
	/* @} */
    btr_path_t*    path_arr;    /*!< in estimating the number of
					rows in range, we store in this array
					information of the path through
					the tree */
};



// The search info struct in an index
struct btr_search_struct{
	ulint ref_count; // Number of blocks in this index tree that have search index built i.e. block->index points to this index. Protected by btr_search_latch except when during initialization in btr_search_info_create().

	// The following fields are not protected by any latch. Unfortunately, this means that they must be aligned to the machine word, i.e., they cannot be turned into bit-fields.
	buf_block_t* root_guess; // the root page frame when it was last time fetched, or NULL
	ulint hash_analysis; // when this exceeds BTR_SEARCH_HASH_ANALYSIS, the hash analysis starts; this is reset if no success noticed
	ibool last_hash_succ; // TRUE if the last search would have succeeded, or did succeed, using the hash index; NOTE that the value here is not exact: it is not calculated for every search, and the calculation itself is not always accurate!
	ulint n_hash_potential; // number of consecutive searches which would have succeeded, or did succeed, using the hash index; the range is 0 .. BTR_SEARCH_BUILD_LIMIT + 5

	ulint n_fields; // recommended prefix length for hash search: number of full fields
	ulint n_bytes; // recommended prefix: number of bytes in an incomplete field @see BTR_PAGE_MAX_REC_SIZE
	ibool left_side; // TRUE or FALSE, depending on whether the leftmost record of several records with the same prefix should be indexed in the hash index
#ifdef IB_SEARCH_PERF_STAT
	ulint n_hash_succ; // number of successful hash searches thus far
	ulint n_hash_fail; // number of failed hash searches
	ulint n_patt_succ; // number of successful pattern searches thus far
	ulint n_searches; // number of searches
#endif /* IB_SEARCH_PERF_STAT */
#ifdef IB_DEBUG
	ulint magic_n; // magic number @see BTR_SEARCH_MAGIC_N
// value of btr_search_struct::magic_n, used in assertions
#define BTR_SEARCH_MAGIC_N 1112765
#endif /* IB_DEBUG */
};

/// \brief Latching modes for btr_cur_search_to_nth_level(). 
enum btr_latch_mode {
    /// \brief Search a record on a leaf page and S-latch it. 
    BTR_SEARCH_LEAF = RW_S_LATCH,
    /// \brief(Prepare to) modify a record on a leaf page and X-latch it. 
    BTR_MODIFY_LEAF = RW_X_LATCH,
    /// \brief Obtain no latches.
    BTR_NO_LATCHES = RW_NO_LATCH,
    /// \brief Start modifying the entire B-tree. 
    BTR_MODIFY_TREE = 33,
    /// \brief Continue modifying the entire B-tree.
    BTR_CONT_MODIFY_TREE = 34,
    /// \brief Search the previous record.
    BTR_SEARCH_PREV = 35,
    /// \brief Modify the previous record.
    BTR_MODIFY_PREV = 36
};

/// \brief Persistent cursor
typedef struct btr_pcur_struct		btr_pcur_t;
/// \brief B-tree cursor
typedef struct btr_cur_struct		btr_cur_t;
/// \brief B-tree search information for the adaptive hash index
typedef struct btr_search_struct	btr_search_t;

/// \brief The size of a reference to data stored on a different page.
/// \details The reference is stored at the end of the prefix of the field in the index record.
constinit ulint BTR_EXTERN_FIELD_REF_SIZE = 20;
