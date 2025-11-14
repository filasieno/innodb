#include "alloc.hpp" // IWYU pragma: keep
#include <cstring>

constexpr ib_size MAX_SMALL_BIN_SIZE = 2048;
static constexpr ib_size HEADER_SIZE    = 16;
static constexpr ib_size MIN_BLOCK_SIZE = 32;
static constexpr ib_size ALIGNMENT      = 32;
static constexpr int STATS_IDX_TREE = alloc_stats::ALLOCATOR_BIN_COUNT;       // 64
static constexpr int STATS_IDX_WILD = alloc_stats::ALLOCATOR_BIN_COUNT + 1;   // 65

const char* to_string(alloc_block_state s) noexcept {
    switch (s) {
        case ALLOC_BLOCK_STATE_USED:                 return "USED";
        case ALLOC_BLOCK_STATE_FREE:                 return "FREE";
        case ALLOC_BLOCK_STATE_WILD_BLOCK:           return "WILD";
        case ALLOC_BLOCK_STATE_BEGIN_SENTINEL:       return "SENTINEL B";
        case ALLOC_BLOCK_STATE_LARGE_BLOCK_SENTINEL: return "SENTINEL L";
        case ALLOC_BLOCK_STATE_END_SENTINEL:         return "SENTINEL E";
        default:                                        return "INVALID";
    }
}

int alloc_table_init(alloc_table* at, void* mem, ib_size size) noexcept {
    
    constexpr ib_u64 SENTINEL_SIZE = sizeof(alloc_pooled_free_block_header);

    IB_ASSERT(mem != nullptr);
    IB_ASSERT(size >= 4096);

    std::memset((void*)at, 0, sizeof(alloc_table));
    
    // Establish heap boundaries
    char* heap_begin = (char*)(mem);
    char* heap_end   = heap_begin + size;

    // // Align start up to 32 and end down to 32 to keep all blocks 32B-multiples
    ib_u64 aligned_begin = ((ib_u64)heap_begin + SENTINEL_SIZE) & ~31ull;
    ib_u64 aligned_end   = ((ib_u64)heap_end   - SENTINEL_SIZE) & ~31ull;

    at->heap_begin = heap_begin;
    at->heap_end   = heap_end;
    at->mem_begin  = (char*)aligned_begin;
    at->mem_end    = (char*)aligned_end;
    at->mem_size   = (ib_size)(at->mem_end - at->mem_begin);

    // Addresses
    // Layout: [BeginSentinel] ... blocks ... [EndSentinel]
    alloc_pooled_free_block_header* begin_sentinel      = (alloc_pooled_free_block_header*)aligned_begin;
    alloc_pooled_free_block_header* wild_block          = (alloc_pooled_free_block_header*)((char*)begin_sentinel + SENTINEL_SIZE);
    alloc_pooled_free_block_header* end_sentinel        = (alloc_pooled_free_block_header*)((char*)aligned_end    - SENTINEL_SIZE);
    // freelist links unused for wild block
    
    // Check alignments
    IB_ASSERT(((ib_u64)begin_sentinel       & 31ull) == 0ull);
    IB_ASSERT(((ib_u64)wild_block           & 31ull) == 0ull);
    IB_ASSERT(((ib_u64)end_sentinel         & 31ull) == 0ull);
    
    
    at->sentinel_begin       = begin_sentinel;
    at->wild_block           = wild_block;
    at->sentinel_end         = end_sentinel;
    alloc_freeblock_init_root(&at->root_free_block);
    
    begin_sentinel->this_desc.size       = (ib_u64)SENTINEL_SIZE;
    begin_sentinel->this_desc.state      = (ib_u32)ALLOC_BLOCK_STATE_BEGIN_SENTINEL;
    // Initialize prevSize for the begin sentinel to avoid reading
    // uninitialized memory in debug printers.
    begin_sentinel->prev_desc             = { 0ull, (ib_u32)ALLOC_BLOCK_STATE_INVALID, 0ull };
    wild_block->this_desc.size            = (ib_u64)((ib_u64)end_sentinel - (ib_u64)wild_block);
    wild_block->this_desc.state           = (ib_u32)ALLOC_BLOCK_STATE_WILD_BLOCK;
    end_sentinel->this_desc.size          = (ib_u64)SENTINEL_SIZE;
    end_sentinel->this_desc.state         = (ib_u32)ALLOC_BLOCK_STATE_END_SENTINEL;
    wild_block->prev_desc                 = begin_sentinel->this_desc;
    end_sentinel->prev_desc               = wild_block->this_desc;
    at->free_mem_size                     = wild_block->this_desc.size;

    for (int i = 0; i < alloc_table::ALLOCATOR_BIN_COUNT; ++i) {
        ut_dlink_init(&at->freelist_head[i]);
    }
    at->freelist_count[63] = 0; // bin 63 is a regular freelist bin (up to 2048)
    at->freelist_mask = 0ull;
    alloc_table_check_invariants(at);
    return 0;
}



// In TryMalloc (replace existing function starting at 2933):
/// \brief Attempts to synchronously allocate memory from the heap.
/// 
/// Algorithm:
/// 1. Compute aligned block size: Add HEADER_SIZE and round up to ALIGNMENT.
/// 2. Find smallest available bin >= required using SIMD-accelerated search.
/// 3. For small bins (<254): Pop free block, split if larger than needed.
/// 4. For medium bin (254): First-fit search on list, split if possible.
/// 5. For wild bin (255): Split from wild block or allocate entirely if exact match.
/// 
/// Returns nullptr if no suitable block found (heap doesn't grow).
/// For async version that suspends on failure, use co_await AllocMem(size).
void* alloc_table_try_malloc(alloc_table* at, ib_size size) noexcept {
    
    alloc_table_check_invariants(at);
    // Compute aligned block size
    ib_size maybe_block = HEADER_SIZE + size;
    ib_size unaligned = maybe_block & (ALIGNMENT - 1);
    ib_size requested_block_size = (unaligned != 0) ? maybe_block + (ALIGNMENT - unaligned) : maybe_block;
    IB_ASSERT((requested_block_size & (ALIGNMENT - 1)) == 0);
    IB_ASSERT(requested_block_size >= MIN_BLOCK_SIZE);

    
    // Try small bin freelists first when eligible (<= 2048)
    int bin_idx = -1;
    if (requested_block_size <= MAX_SMALL_BIN_SIZE) {
        bin_idx = alloc_freelist_find_index(&at->freelist_mask, requested_block_size);
    }
    
    // Small bin allocation case (bins 0..63)
    // ======================================
    if (bin_idx >= 0) {
        IB_ASSERT(at->freelist_count[bin_idx] > 0);
        IB_ASSERT(alloc_freelist_get_mask(&at->freelist_mask, bin_idx));
        
        ut_dlink* free_stack = &at->freelist_head[bin_idx];
        ut_dlink* link = ut_dlink_pop(free_stack);
        --at->freelist_count[bin_idx];
        if (at->freelist_count[bin_idx] == 0) {
            alloc_freelist_clear_mask(&at->freelist_mask, bin_idx);
        }
        alloc_block_header* block = (alloc_block_header*)((char*)link - IB_OFFSET_OF(alloc_pooled_free_block_header, freelist_link));
        alloc_block_header* next_block = alloc_block_next(block);
        __builtin_prefetch(next_block, 1, 3);
        
        ut_dlink_clear(link);

        ib_size block_size = block->this_desc.size;
        
        // Exact match case
        // ----------------
        if (block_size == requested_block_size) {  
            // Update This State
            IB_ASSERT(block->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE);
            block->this_desc.state = (ib_u32)ALLOC_BLOCK_STATE_USED;
            IB_ASSERT(block->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_USED);
            
            // Update Prev State
            IB_ASSERT(next_block->prev_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE);
            next_block->prev_desc.state = (ib_u32)ALLOC_BLOCK_STATE_USED;
            IB_ASSERT(next_block->prev_desc.state == (ib_u32)ALLOC_BLOCK_STATE_USED);

            at->free_mem_size -= requested_block_size;
            ++at->stats.alloc_counter[bin_idx];
            ++at->stats.reused_counter[bin_idx];
            
            alloc_table_check_invariants(at);
            return (void*)((char*)block + HEADER_SIZE);
        } 
        
        // Required Split case
        // -------------------
        
        ib_size new_free_size = block_size - requested_block_size;
        IB_ASSERT(new_free_size >= MIN_BLOCK_SIZE && new_free_size % ALIGNMENT == 0);
        
        // Prefetch the new free block
        // ----------------------------
        alloc_pooled_free_block_header* new_free = (alloc_pooled_free_block_header*)((char*)block + requested_block_size);
        __builtin_prefetch(new_free, 1, 3);

        // Prefetch stats
        // --------------
        ib_size new_bin_idx = alloc_freelist_get_index(new_free_size);
        __builtin_prefetch(&at->stats.split_counter[bin_idx], 1, 3);  
        __builtin_prefetch(&at->stats.alloc_counter[bin_idx], 1, 3);
        __builtin_prefetch(&at->stats.pooled_counter[new_bin_idx],  1, 3);

        // Update the new free block
        // -------------------------
        IB_ASSERT(block->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE);

        alloc_block_desc new_alloc_record_size = { requested_block_size, (ib_u32)ALLOC_BLOCK_STATE_USED, 0 };
        block->this_desc   = new_alloc_record_size;
        new_free->prev_desc = new_alloc_record_size;

        alloc_block_desc new_free_size_record = { new_free_size, (ib_u32)ALLOC_BLOCK_STATE_FREE, 0 };
        new_free->this_desc   = new_free_size_record;
        next_block->prev_desc = new_free_size_record;
        
        IB_ASSERT(block->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_USED);
        IB_ASSERT(next_block->prev_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE);
        IB_ASSERT(new_free->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE);

        // Update stats
        // ------------
        
        ++at->stats.split_counter[bin_idx];
        ++at->stats.alloc_counter[bin_idx];
        // push to head (LIFO)
        ut_dlink_push(&at->freelist_head[new_bin_idx], &new_free->freelist_link);
        alloc_freelist_set_mask(&at->freelist_mask, new_bin_idx);
        ++at->stats.pooled_counter[new_bin_idx];            
        ++at->freelist_count[new_bin_idx];
        at->free_mem_size -= requested_block_size;
        
        return (void*)((char*)block + HEADER_SIZE);            
    }

    // Large block tree allocation path for sizes > 2048
    if (requested_block_size > MAX_SMALL_BIN_SIZE) {
        alloc_free_block_header* free_block = alloc_freeblock_find_gte(at->root_free_block, requested_block_size);
        if (free_block != nullptr) {
            // Detach chosen block from the tree/list structure
            alloc_freeblock_detach(&at->root_free_block, free_block);

            alloc_block_header* block = (alloc_block_header*)free_block;
            alloc_block_header* next_block = alloc_block_next(block);
            __builtin_prefetch(next_block, 1, 3);

            ib_size block_size = block->this_desc.size;
            if (block_size == requested_block_size) {
                // Exact match
                IB_ASSERT(block->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE);
                block->this_desc.state = (ib_u32)ALLOC_BLOCK_STATE_USED;
                IB_ASSERT(next_block->prev_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE);
                next_block->prev_desc.state = (ib_u32)ALLOC_BLOCK_STATE_USED;
                at->free_mem_size -= requested_block_size;
                // Count as large allocation under stats index TREE
                ++at->stats.alloc_counter[STATS_IDX_TREE];
                ++at->stats.reused_counter[STATS_IDX_TREE];
                
                alloc_table_check_invariants(at);
                return (void*)((char*)block + HEADER_SIZE);
            }

            // Split large free block
            ib_size new_free_size = block_size - requested_block_size;
            IB_ASSERT(new_free_size >= MIN_BLOCK_SIZE && new_free_size % ALIGNMENT == 0);
            alloc_block_header* new_free_hdr = (alloc_block_header*)((char*)block + requested_block_size);
            __builtin_prefetch(new_free_hdr, 1, 3);

            alloc_block_desc alloc_desc = { requested_block_size, (ib_u32)ALLOC_BLOCK_STATE_USED, 0 };
            block->this_desc = alloc_desc;
            ((alloc_block_header*)new_free_hdr)->prev_desc = alloc_desc;

            alloc_block_desc free_desc = { new_free_size, (ib_u32)ALLOC_BLOCK_STATE_FREE, 0 };
            ((alloc_block_header*)new_free_hdr)->this_desc = free_desc;
            next_block->prev_desc = free_desc;

            // Place the remainder appropriately
            if (new_free_size > MAX_SMALL_BIN_SIZE) {
                alloc_freeblock_put(&at->root_free_block, (alloc_block_header*)new_free_hdr);
            } else {
                ib_u32 new_bin_idx = alloc_freelist_get_index(new_free_size);
                ut_dlink_push(&at->freelist_head[new_bin_idx], &((alloc_pooled_free_block_header*)new_free_hdr)->freelist_link);
                alloc_freelist_set_mask(&at->freelist_mask, new_bin_idx);
                ++at->freelist_count[new_bin_idx];
                ++at->stats.pooled_counter[new_bin_idx];
            }

            ++at->stats.alloc_counter[STATS_IDX_TREE];
            ++at->stats.split_counter[STATS_IDX_TREE];
            at->free_mem_size -= requested_block_size;

            alloc_table_check_invariants(at);
            return (void*)((char*)block + HEADER_SIZE);
        }
    }

    // Update the free block
    // ---------------------
    
    // Fallback: allocate from the Wild Block
    // ======================================
    {
        IB_ASSERT(at->wild_block != nullptr);                      // Wild block pointer always valid
        // No freelist bit for wild; use boundary bin 63 for accounting

        // Note: The wild block is a degenerate case; it does not use free bins
        //       and it must always be allocated; which means have at least MIN_BLOCK_SIZE free space
        
        alloc_block_header* old_wild = (alloc_block_header*)at->wild_block;            
        
        // Prefetch the next block, the prev block and the new wild block
        // --------------------------------------------------------------
        
        // 1. Prefetch the next block
        alloc_block_header* next_block = alloc_block_next(old_wild);
        __builtin_prefetch(next_block, 1, 3);
        
        // 2. Prefetch the new wild block
        alloc_pooled_free_block_header* new_wild = (alloc_pooled_free_block_header*)((char*)old_wild + requested_block_size);
        __builtin_prefetch(new_wild, 1, 3);

        // 3. Prefetch stats
        __builtin_prefetch(&at->stats.alloc_counter[STATS_IDX_WILD], 1, 3);
        __builtin_prefetch(&at->stats.split_counter[STATS_IDX_WILD], 1, 3);
        
        // Case there the wild block is full; memory is exhausted
        // ------------------------------------------------------
        ib_size old_size = old_wild->this_desc.size;
        if (requested_block_size > old_size - MIN_BLOCK_SIZE) {
            // the wild block must have at least MIN_BLOCK_SIZE free space
            ++at->stats.failed_counter[STATS_IDX_WILD];
            return nullptr; // not enough space
        }
        
        // Case there is enough space -> Split the wild block
        // --------------------------------------------------
        ib_size new_wild_size = old_size - requested_block_size;
        IB_ASSERT(new_wild_size >= MIN_BLOCK_SIZE && new_wild_size % ALIGNMENT == 0);
        
        alloc_block_desc allocated_size = { requested_block_size, (ib_u32)ALLOC_BLOCK_STATE_USED, 0 };
        alloc_block_header* allocated = old_wild;
        allocated->this_desc = allocated_size;
        
        alloc_block_desc new_wild_size_record = { new_wild_size, (ib_u32)ALLOC_BLOCK_STATE_WILD_BLOCK, 0 };
        new_wild->this_desc = new_wild_size_record;
        new_wild->prev_desc = allocated->this_desc;
        at->wild_block = new_wild;
        next_block->prev_desc = new_wild->this_desc;
        
        // Update stats
        ++at->stats.alloc_counter[STATS_IDX_WILD];
        ++at->stats.split_counter[STATS_IDX_WILD];
        at->free_mem_size -= requested_block_size;
        
        alloc_table_check_invariants(at);
        return (void*)((char*)allocated + HEADER_SIZE);
    }
}

/// \brief Frees allocated memory and coalesces with adjacent free blocks.
/// 
/// Algorithm:
/// 1. Locate the block from the pointer; return if null.
/// 2. Perform left coalescing in a loop: while the previous block is free and leftMerges < sideCoalescing, unlink it from its bin, merge it into the current block by adjusting sizes and shifting the block pointer left, update merge stats.
/// 3. Perform right coalescing in a loop: while the next block is free or wild and rightMerges < sideCoalescing, unlink it, merge into current by adjusting sizes, update next-next prevSize; if it was wild, flag mergedToWild and break the loop.
/// 4. If mergedToWild, set the block state to WILD_BLOCK and update wild pointer; else, set to FREE and push to the appropriate bin.
/// 5. Update global free memory size, free count stats, and final next block's prevSize.
///
/// This handles chains of adjacent free blocks up to the limit per side.
/// 
/// \param ptr Pointer returned by TryMalloc (must not be nullptr).
/// \param side_coalescing Maximum number of merges per side (0 = no coalescing, defaults to UINT_MAX for unlimited).
void alloc_table_free(alloc_table* at, void* ptr, ib_u32 side_coalescing) noexcept {
    IB_ASSERT(ptr != nullptr);
    (void)side_coalescing;

    alloc_table_check_invariants(at);
    // Release Block
    // -------------
    alloc_pooled_free_block_header* block = (alloc_pooled_free_block_header*)((char*)ptr - HEADER_SIZE);
    alloc_block_desc this_size = block->this_desc;
    ib_size block_size = this_size.size;

    // Update block state
    // -------------------
    enum alloc_block_state block_state = (enum alloc_block_state)this_size.state;
    IB_ASSERT(block_state == ALLOC_BLOCK_STATE_USED);        
    block->this_desc.state = (ib_u32)ALLOC_BLOCK_STATE_FREE;
    at->free_mem_size += block_size;

    // Update next block prevSize
    // --------------------------
    alloc_block_header* next_block = alloc_block_next((alloc_block_header*)block);
    next_block->prev_desc = block->this_desc;

    // Update stats
    // ------------

    // Place freed block back into appropriate structure
    if (block_size > MAX_SMALL_BIN_SIZE) {
        alloc_freeblock_put(&at->root_free_block, (alloc_block_header*)block);
        ++at->stats.free_counter[STATS_IDX_TREE];
        alloc_table_check_invariants(at);
        return;
    }

    // Small bin free case (bins 0..63)
    // --------------------------------
    unsigned orig_bin_idx = alloc_freelist_get_index(block_size);
    IB_ASSERT(orig_bin_idx < alloc_table::ALLOCATOR_BIN_COUNT);
    // push to head of freelist (AkDLink)
    ut_dlink_push(&at->freelist_head[orig_bin_idx], &block->freelist_link);
    ++at->stats.free_counter[orig_bin_idx];
    ++at->stats.pooled_counter[orig_bin_idx];
    ++at->freelist_count[orig_bin_idx];
    alloc_freelist_set_mask(&at->freelist_mask, orig_bin_idx);
    alloc_table_check_invariants(at);
}


// Coalesce helpers: merge adjacent free or wild blocks into the provided block
// Returns: total merged size added into '*out_block' (not including original block size), or -1 on error
ib_i64 alloc_table_coalesce_left(alloc_table* at, alloc_block_header** out_block, ib_u32 max_merges) noexcept {
    IB_ASSERT(out_block != nullptr);
    alloc_block_header* block = *out_block;
    IB_ASSERT(block != nullptr);
    alloc_table_check_invariants(at);
    enum alloc_block_state st = (enum alloc_block_state)block->this_desc.state;
    if (!(st == ALLOC_BLOCK_STATE_FREE || st == ALLOC_BLOCK_STATE_WILD_BLOCK)) return -1;

    // Detach starting block if FREE
    if (st == ALLOC_BLOCK_STATE_FREE) {
        ib_u64 sz = block->this_desc.size;
        if (sz <= MAX_SMALL_BIN_SIZE) {
            ib_u32 bin = alloc_freelist_get_index(sz);
            ut_dlink* link = &((alloc_pooled_free_block_header*)block)->freelist_link;
            if (!ut_dlink_is_detached(link)) {
                ut_dlink_detach(link);
                IB_ASSERT(at->freelist_count[bin] > 0);
                --at->freelist_count[bin];
                if (at->freelist_count[bin] == 0) {
                    alloc_freelist_clear_mask(&at->freelist_mask, bin);
                }
            }
        } else {
            alloc_freeblock_detach(&at->root_free_block, (alloc_free_block_header*)block);
        }
    }

    ib_i64 merged = 0;
    while (max_merges--) {
        alloc_block_header* left = alloc_block_prev(block);
        enum alloc_block_state lst = (enum alloc_block_state)left->this_desc.state;
        if (!(lst == ALLOC_BLOCK_STATE_FREE || lst == ALLOC_BLOCK_STATE_WILD_BLOCK)) break;

        // Detach the left neighbor from free structures immediately
        ib_u64 left_size = left->this_desc.size;
        if (lst == ALLOC_BLOCK_STATE_FREE) {
            if (left_size <= MAX_SMALL_BIN_SIZE) {
                ib_u32 lain = alloc_freelist_get_index(left_size);
                ut_dlink* link = &((alloc_pooled_free_block_header*)left)->freelist_link;
                if (!ut_dlink_is_detached(link)) {
                    ut_dlink_detach(link);
                    IB_ASSERT(at->freelist_count[lain] > 0);
                    --at->freelist_count[lain];
                    if (at->freelist_count[lain] == 0) {
                        alloc_freelist_clear_mask(&at->freelist_mask, lain);
                    }
                }
                ++at->stats.merged_counter[lain];
            } else {
                alloc_freeblock_detach(&at->root_free_block, (alloc_free_block_header*)left);
                ++at->stats.merged_counter[STATS_IDX_TREE];
            }
        } else { // WILD_BLOCK
            block->this_desc.state = (ib_u32)ALLOC_BLOCK_STATE_WILD_BLOCK;
            at->wild_block = (alloc_pooled_free_block_header*)block;
            ++at->stats.merged_counter[STATS_IDX_WILD];
        }

        ib_u64 cur_size  = block->this_desc.size;
        ib_u64 new_size  = left_size + cur_size;
        block = left; // shift to left block
        block->this_desc.size = new_size;
        alloc_block_header* right = alloc_block_next((alloc_block_header*)block);
        right->prev_desc = block->this_desc;
        merged += (ib_i64)left_size;
    }

    // Reinsert resulting block if it is FREE (not WILD)
    if ((enum alloc_block_state)block->this_desc.state == ALLOC_BLOCK_STATE_FREE) {
        ib_u64 sz = block->this_desc.size;
        if (sz <= MAX_SMALL_BIN_SIZE) {
            ib_u32 bin = alloc_freelist_get_index(sz);
            ut_dlink_push(&at->freelist_head[bin], &((alloc_pooled_free_block_header*)block)->freelist_link);
            alloc_freelist_set_mask(&at->freelist_mask, bin);
            ++at->freelist_count[bin];
            ++at->stats.pooled_counter[bin];
            ++at->stats.free_counter[bin];
        } else {
            alloc_freeblock_put(&at->root_free_block, (alloc_block_header*)block);
            ++at->stats.free_counter[STATS_IDX_TREE];
        }
    } else {
        // Ensure wild pointer is set
        at->wild_block = (alloc_pooled_free_block_header*)block;
    }

    *out_block = block;
    alloc_table_check_invariants(at);
    return merged;
}

ib_i64 alloc_table_coalesce_right(alloc_table* at, alloc_block_header** out_block, ib_u32 max_merges) noexcept {
    IB_ASSERT(out_block != nullptr);
    alloc_block_header* block = *out_block;
    IB_ASSERT(block != nullptr);
    alloc_table_check_invariants(at);
    enum alloc_block_state st = (enum alloc_block_state)block->this_desc.state;
    if (!(st == ALLOC_BLOCK_STATE_FREE || st == ALLOC_BLOCK_STATE_WILD_BLOCK)) return -1;

    // Detach starting block if FREE
    if (st == ALLOC_BLOCK_STATE_FREE) {
        ib_u64 sz = block->this_desc.size;
        if (sz <= MAX_SMALL_BIN_SIZE) {
            ib_u32 bin = alloc_freelist_get_index(sz);
            ut_dlink* link = &((alloc_pooled_free_block_header*)block)->freelist_link;
            if (!ut_dlink_is_detached(link)) {
                ut_dlink_detach(link);
                IB_ASSERT(at->freelist_count[bin] > 0);
                --at->freelist_count[bin];
                if (at->freelist_count[bin] == 0) {
                    alloc_freelist_clear_mask(&at->freelist_mask, bin);
                }
            }
        } else {
            alloc_freeblock_detach(&at->root_free_block, (alloc_free_block_header*)block);
        }
    }

    ib_i64 merged = 0;
    while (max_merges--) {
        alloc_block_header* right = alloc_block_next(block);
        enum alloc_block_state rst = (enum alloc_block_state)right->this_desc.state;
        if (!(rst == ALLOC_BLOCK_STATE_FREE || rst == ALLOC_BLOCK_STATE_WILD_BLOCK)) break;

        ib_u64 right_size = right->this_desc.size;
        if (rst == ALLOC_BLOCK_STATE_FREE) {
            if (right_size <= MAX_SMALL_BIN_SIZE) {
                ib_u32 rbin = alloc_freelist_get_index(right_size);
                ut_dlink* link = &((alloc_pooled_free_block_header*)right)->freelist_link;
                if (!ut_dlink_is_detached(link)) {
                    ut_dlink_detach(link);
                    IB_ASSERT(at->freelist_count[rbin] > 0);
                    --at->freelist_count[rbin];
                    if (at->freelist_count[rbin] == 0) {
                        alloc_freelist_clear_mask(&at->freelist_mask, rbin);
                    }
                }
                ++at->stats.merged_counter[rbin];
            } else {
                alloc_freeblock_detach(&at->root_free_block, (alloc_free_block_header*)right);
                ++at->stats.merged_counter[STATS_IDX_TREE];
            }
        } else { // WILD_BLOCK
            block->this_desc.state = (ib_u32)ALLOC_BLOCK_STATE_WILD_BLOCK;
            at->wild_block = (alloc_pooled_free_block_header*)block;
            ++at->stats.merged_counter[STATS_IDX_WILD];
        }

        ib_u64 cur_size   = block->this_desc.size;
        ib_u64 new_size   = cur_size + right_size;
        block->this_desc.size = new_size;
        alloc_block_header* right_right = alloc_block_next(block);
        right_right->prev_desc = block->this_desc;
        merged += (ib_i64)right_size;
    }

    // Reinsert resulting block if it is FREE (not WILD)
    if ((enum alloc_block_state)block->this_desc.state == ALLOC_BLOCK_STATE_FREE) {
        ib_u64 sz = block->this_desc.size;
        if (sz <= MAX_SMALL_BIN_SIZE) {
            ib_u32 bin = alloc_freelist_get_index(sz);
            ut_dlink_push(&at->freelist_head[bin], &((alloc_pooled_free_block_header*)block)->freelist_link);
            alloc_freelist_set_mask(&at->freelist_mask, bin);
            ++at->freelist_count[bin];
            ++at->stats.pooled_counter[bin];
            ++at->stats.free_counter[bin];
        } else {
            alloc_freeblock_put(&at->root_free_block, (alloc_block_header*)block);
            ++at->stats.free_counter[STATS_IDX_TREE];
        }
    } else {
        // Ensure wild pointer is set
        at->wild_block = (alloc_pooled_free_block_header*)block;
    }

    *out_block = block;
    alloc_table_check_invariants(at);
    return merged;
}

int alloc_table_defrag(alloc_table* at, ib_u64 millis_budget) noexcept {
    (void)millis_budget;
    alloc_table_check_invariants(at);
    
    int defragged = 0;
    alloc_block_header* begin = (alloc_block_header*)at->sentinel_begin;
    alloc_block_header* end   = (alloc_block_header*)alloc_block_next((alloc_block_header*)at->sentinel_end);
    for (alloc_block_header* h = begin; h != end; h = alloc_block_next(h)) {
        enum alloc_block_state st = (enum alloc_block_state)h->this_desc.state;
        if (st != ALLOC_BLOCK_STATE_FREE) continue;
        alloc_block_header* cur = h;
        ib_i64 merged = alloc_table_coalesce_right(at, &cur, 1);
        if (merged > 0) ++defragged;
        h = cur; // continue from the merged block
    }
    alloc_table_check_invariants(at);
    return defragged;
}
