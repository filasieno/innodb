#pragma once

#include "xinnodb.hpp"  // base types and macros
#include "ut_dlink.hpp" // dlink

enum alloc_block_state 
{
    ALLOC_BLOCK_STATE_INVALID              = 0b0000,
    ALLOC_BLOCK_STATE_USED                 = 0b0010,
    ALLOC_BLOCK_STATE_FREE                 = 0b0001,
    ALLOC_BLOCK_STATE_WILD_BLOCK           = 0b0011,
    ALLOC_BLOCK_STATE_BEGIN_SENTINEL       = 0b0100,
    ALLOC_BLOCK_STATE_LARGE_BLOCK_SENTINEL = 0b0110,
    ALLOC_BLOCK_STATE_END_SENTINEL         = 0b1100,
};
const char* to_string(enum alloc_block_state) noexcept;

enum class alloc_kind 
{
    INVALID = 0,
    GENERIC_MALLOC,
    PROMISE,
    FREE_SEGMENT_INDEX_LEAF,
    FREE_SEGMENT_INDEX_INNER,
    FREE_SEGMENT_INDEX_LEAF_EXTENSION
};

struct alloc_block_desc 
{ 
    ib_u64 size:48; 
    ib_u64 state:4; 
    ib_u64 kind:12; 
};

struct alloc_block_header 
{ 
    struct alloc_block_desc this_desc; 
    struct alloc_block_desc prev_desc; 
};

struct alloc_pooled_free_block_header : public alloc_block_header 
{ 
    ut_dlink freelist_link; 
};
static_assert(sizeof(alloc_pooled_free_block_header) == 32);

struct alloc_free_block_header : public alloc_block_header 
{
    struct ut_dlink                    multimap_link;
    struct alloc_free_block_header* parent;
    struct alloc_free_block_header* left;
    struct alloc_free_block_header* right;
    int                                height;
    int                                balance;
};
static_assert(sizeof(alloc_free_block_header) == 64, "AllocFreeBlockHeader size is not 64 bytes");

struct alloc_stats 
{
    static constexpr int ALLOCATOR_BIN_COUNT = 64;
    static constexpr int STATS_BIN_COUNT = 66;
    static constexpr int STATS_IDX_TREE = 64;
    static constexpr int STATS_IDX_WILD = 65;

    ib_size alloc_counter[STATS_BIN_COUNT];
    ib_size realloc_counter[STATS_BIN_COUNT];
    ib_size free_counter[STATS_BIN_COUNT];
    ib_size failed_counter[STATS_BIN_COUNT];
    ib_size split_counter[STATS_BIN_COUNT];
    ib_size merged_counter[STATS_BIN_COUNT];
    ib_size reused_counter[STATS_BIN_COUNT];
    ib_size pooled_counter[STATS_BIN_COUNT];
};


struct alloc_table 
{
    static constexpr int ALLOCATOR_BIN_COUNT = alloc_stats::ALLOCATOR_BIN_COUNT;

    alignas(64) struct ut_dlink               freelist_head[ALLOCATOR_BIN_COUNT];
    alignas(64) ib_u32                         freelist_count[ALLOCATOR_BIN_COUNT];
    alignas(8)  ib_u64                         freelist_mask;
    alignas(8)  char*                         heap_begin;
    alignas(8)  char*                         heap_end;
    alignas(8)  char*                         mem_begin;
    alignas(8)  char*                         mem_end;
    alignas(8)  ib_size                        mem_size;
    alignas(8)  ib_size                        free_mem_size;
    alignas(8)  ib_size                        max_free_block_size;
    alignas(8)  alloc_stats                stats;
    alignas(8)  alloc_pooled_free_block_header* sentinel_begin;
    alignas(8)  alloc_pooled_free_block_header* sentinel_end;
    alignas(8)  alloc_pooled_free_block_header* wild_block;
    alignas(8)  alloc_free_block_header*   root_free_block;
};



