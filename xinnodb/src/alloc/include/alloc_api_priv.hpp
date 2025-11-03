#pragma once

// Private API declaration
// ------------------------------------------------
// Defines the private API of the Allocator module.
// The contents of this file are part of the private API and WILL NOT be part of the public API

#include "xinnodb.hpp"        // IWYU pragma: keep
#include "alloc_api.hpp"      // IWYU pragma: keep

#include <source_location>

constexpr ib_u64 ALLOC_STATE_IS_USED_MASK     = 0;
constexpr ib_u64 ALLOC_STATE_IS_FREE_MASK     = 1;
constexpr ib_u64 ALLOC_STATE_IS_SENTINEL_MASK = 4;

// Allocator Table
int    alloc_table_init(alloc_table* at, void* mem, ib_size size) noexcept;
void*  alloc_table_try_malloc(alloc_table* at, ib_size size) noexcept;
void   alloc_table_free(alloc_table* at, void* ptr, ib_u32 side_coalescing) noexcept;
int    alloc_table_defrag(alloc_table* at, ib_u64 millis_budget) noexcept;
void   alloc_table_check_invariants(alloc_table* at, std::source_location loc = std::source_location::current()) noexcept;
ib_i64 alloc_table_coalesce_right(alloc_table* at, alloc_block_header** out_block, ib_u32 max_merges) noexcept;
ib_i64 alloc_table_coalesce_left(alloc_table* at, alloc_block_header** out_block, ib_u32 max_merges) noexcept;

// Free block Tree
void                     alloc_freeblock_init_root(alloc_free_block_header** root) noexcept;
void                     alloc_freeblock_put(alloc_free_block_header** root, alloc_block_header* block) noexcept;
alloc_free_block_header* alloc_freeblock_find_gte(alloc_free_block_header* root, ib_u64 block_size) noexcept;
void                     alloc_freeblock_detach(alloc_free_block_header** root, alloc_free_block_header* node) noexcept;
void                     alloc_freeblock_clear(alloc_free_block_header* link) noexcept;
bool                     alloc_freeblock_is_detached(const alloc_free_block_header* link) noexcept;

// Freeblock list bitmask utilities
void   alloc_freelist_set_mask(ib_u64* bit_field, ib_u64 bin_idx) noexcept;
bool   alloc_freelist_get_mask(const ib_u64* bit_field, ib_u64 bin_idx) noexcept;
void   alloc_freelist_clear_mask(ib_u64* bit_field, ib_u64 bin_idx) noexcept;
int    alloc_freelist_find_index(const ib_u64* bit_field, ib_size alloc_size) noexcept;
ib_u32 alloc_freelist_get_index(const alloc_block_header* header) noexcept;
ib_u64 alloc_freelist_get_index(ib_u64 size) noexcept;

// Block Iteration
alloc_block_header* alloc_block_next(alloc_block_header* header) noexcept;
alloc_block_header* alloc_block_prev(alloc_block_header* header) noexcept;    



