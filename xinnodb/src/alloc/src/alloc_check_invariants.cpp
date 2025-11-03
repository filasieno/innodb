#include "alloc.hpp" // IWYU pragma: keep
#include "ut.hpp"


void alloc_table_check_invariants(alloc_table* at, std::source_location loc) noexcept {

        // Basic table invariants
        IB_ASSERT(at->heap_begin < at->mem_begin, "basic alloc table invariant failed");
        IB_ASSERT(at->mem_begin  < at->mem_end, "basic alloc table invariant failed");
        IB_ASSERT(at->mem_end    < at->heap_end, "basic alloc table invariant failed");
        IB_ASSERT(((ib_u64)at->mem_begin & 31ull) == 0ull, "basic alloc table invariant failed");
        IB_ASSERT(((ib_u64)at->mem_end   & 31ull) == 0ull, "basic alloc table invariant failed");
        IB_ASSERT(at->mem_size == (ib_size)(at->mem_end - at->mem_begin), "basic alloc table invariant failed");

        // Sentinels positioning invariants
        IB_ASSERT((void*)at->sentinel_begin == (void*)at->mem_begin, "sentinal position invariant failed");
        IB_ASSERT((ib_u64)at->sentinel_begin->this_desc.size % 32ull == 0ull, "sentinal position invariant failed");
        IB_ASSERT(at->sentinel_begin->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_BEGIN_SENTINEL, "sentinal position invariant failed");
        IB_ASSERT(at->sentinel_begin->prev_desc.size == 0ull, "sentinal position invariant failed");

        alloc_pooled_free_block_header* expected_end = (alloc_pooled_free_block_header*)((char*)at->mem_end - sizeof(alloc_pooled_free_block_header));
        IB_ASSERT((void*)at->sentinel_end == (void*)expected_end, "sentinal position invariant failed");
        IB_ASSERT((ib_u64)at->sentinel_end->this_desc.size % 32ull == 0ull, "sentinal position invariant failed");
        IB_ASSERT(at->sentinel_end->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_END_SENTINEL, "sentinal position invariant failed");

        // Wild block basic invariants
        IB_ASSERT(at->wild_block != nullptr, "wild block invariant failed");
        IB_ASSERT((char*)at->wild_block >= at->mem_begin, "wild block invariant failed");
        IB_ASSERT((char*)at->wild_block <  at->mem_end, "wild block invariant failed");
        IB_ASSERT(((ib_u64)at->wild_block & 31ull) == 0ull, "wild block invariant failed");
        IB_ASSERT(at->wild_block->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_WILD_BLOCK, "wild block invariant failed");

        // Scan heap blocks and verify local invariants
        ib_u64 counted_free_bytes = 0ull;
        ib_u64 counted_used_bytes = 0ull;
        ib_u64 counted_wild_bytes = 0ull;
        (void)counted_wild_bytes;

        ib_u64 small_free_count_bin[alloc_table::ALLOCATOR_BIN_COUNT] = {};
        ib_u64 large_free_block_count = 0ull;
        ib_u64 wild_block_instances = 0ull;

        const alloc_block_header* begin = (alloc_block_header*)at->sentinel_begin;
        const alloc_block_header* end   = (alloc_block_header*)((char*)at->sentinel_end + at->sentinel_end->this_desc.size);

        const alloc_block_header* prev = nullptr;
        for (const alloc_block_header* h = begin; h != end; h = alloc_block_next((alloc_block_header*)h)) {
            // Address bounds and alignment
            IB_ASSERT((char*)h >= at->mem_begin, "heap block invariant failed");
            IB_ASSERT((char*)h <  at->mem_end, "heap block invariant failed");
            IB_ASSERT(((ib_u64)h & 31ull) == 0ull, "heap block invariant failed");

            // ib_size constraints
            ib_u64 sz = h->this_desc.size;
            IB_ASSERT(sz >= sizeof(alloc_block_header), "heap block invariant failed");
            IB_ASSERT((sz & 31ull) == 0ull, "heap block invariant failed");

            // Prev descriptor consistency
            if (prev) {
                IB_ASSERT(h->prev_desc.size  == prev->this_desc.size, "heap block invariant failed");
                IB_ASSERT(h->prev_desc.state == prev->this_desc.state, "heap block invariant failed");
                // Bidirectional linkage check
                IB_ASSERT(alloc_block_next((alloc_block_header*)prev) == h, "heap block invariant failed");
                IB_ASSERT(alloc_block_prev((alloc_block_header*)h) == prev, "heap block invariant failed");
            } else {
                // First block is the begin sentinel
                IB_ASSERT(h == begin, "heap block invariant failed");
                IB_ASSERT(h->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_BEGIN_SENTINEL, "heap block invariant failed");
            }

            // State-specific checks and accounting
            alloc_block_state st = (alloc_block_state)h->this_desc.state;
            switch (st) {
                case ALLOC_BLOCK_STATE_BEGIN_SENTINEL:
                    IB_ASSERT(h == begin, "heap block invariant failed");
                    counted_used_bytes += sz;
                    break;
                case ALLOC_BLOCK_STATE_END_SENTINEL:
                    IB_ASSERT(h == (alloc_block_header*)at->sentinel_end, "heap block invariant failed");
                    counted_used_bytes += sz;
                    break;
                case ALLOC_BLOCK_STATE_WILD_BLOCK:
                    IB_ASSERT((alloc_block_header*)h == (alloc_block_header*)at->wild_block, "heap block invariant failed");
                    IB_ASSERT(sz >= 32ull, "heap block invariant failed");
                    ++wild_block_instances;
                    counted_wild_bytes += sz;
                    counted_free_bytes += sz;
                    break;
                case ALLOC_BLOCK_STATE_FREE:
                    IB_ASSERT(sz >= 32ull);
                    if (sz <= 2048ull) {
                        ++small_free_count_bin[alloc_freelist_get_index(h)];
                    } else {
                        ++large_free_block_count;
                    }
                    counted_free_bytes += sz;
                    break;
                case ALLOC_BLOCK_STATE_USED:
                    counted_used_bytes += sz;
                    break;
                default:
                    std::abort();
            }

            prev = h;
        }

        // Ensure we saw exactly one wild block instance
        IB_ASSERT(wild_block_instances == 1ull, "wild block invariant failed: {}", wild_block_instances);

        // Memory accounting: free + used should equal mem_size
        IB_ASSERT(counted_free_bytes + counted_used_bytes == at->mem_size, "memory accounting invariant failed: {} + {} != {}", counted_free_bytes, counted_used_bytes, at->mem_size);
        IB_ASSERT(counted_free_bytes == at->free_mem_size, "memory accounting invariant failed: {} != {}", counted_free_bytes, at->free_mem_size);

        // Validate small freelist structures: mask and counts
        ib_u64 observed_mask = 0ull;
        for (ib_u32 bin = 0; bin < alloc_table::ALLOCATOR_BIN_COUNT; ++bin) {
            ut_dlink* head = &at->freelist_head[bin];
            ib_u64 ring_count = 0ull;
            for (ut_dlink* it = head->next; it != head; it = it->next) {
                const ib_size link_off = IB_OFFSET_OF(alloc_pooled_free_block_header, freelist_link);
                alloc_block_header* b = (alloc_block_header*)((char*)it - link_off);
                // Each member must be FREE and in-range
                IB_ASSERT(b->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE, "small freelist invariant failed: {}", to_string((alloc_block_state)(b->this_desc.state)));
                IB_ASSERT(b->this_desc.size <= 2048ull, "small freelist invariant failed");
                IB_ASSERT(alloc_freelist_get_index(b) == bin, "small freelist invariant failed");
                ++ring_count;
            }
            if (ring_count > 0) observed_mask |= (1ull << bin);
            IB_ASSERT(ring_count == (ib_u64)at->freelist_count[bin], "small freelist invariant failed");
            IB_ASSERT(ring_count == small_free_count_bin[bin], "small freelist invariant failed");
            const bool mask_bit = ((at->freelist_mask >> bin) & 1ull) != 0ull;
            IB_ASSERT(mask_bit == (ring_count > 0), "small freelist invariant failed");
        }
        IB_ASSERT(observed_mask == at->freelist_mask, "small freelist invariant failed");

        // Validate large free block AVL tree: states and ordering
        ib_u64 observed_large_free_count = 0ull;
        auto validate_tree = [&](auto&& self, alloc_free_block_header* node, ib_u64 min_key, ib_u64 max_key) -> int {
            if (!node) return 0;
            ib_u64 key = node->this_desc.size;
            IB_ASSERT(key > 2048ull, "large freelist invariant failed");
            IB_ASSERT(key > min_key && key < max_key, "large freelist invariant failed");
            IB_ASSERT(node->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE, "large freelist invariant failed");
            // children parent pointers
            if (node->left)  IB_ASSERT(node->left->parent  == node, "large freelist invariant failed");
            if (node->right) IB_ASSERT(node->right->parent == node, "large freelist invariant failed");
            // left subtree
            int hl = self(self, node->left, min_key, key);
            // right subtree
            int hr = self(self, node->right, key, max_key);
            // multimap ring: all nodes must have the same size and FREE state
            ib_u64 list_count = 0ull;
            for (ut_dlink* it = node->multimap_link.next; it != &node->multimap_link; it = it->next) {
                alloc_free_block_header* n = (alloc_free_block_header*)((char*)it - IB_OFFSET_OF(alloc_free_block_header, multimap_link));
                IB_ASSERT(n->this_desc.size == key, "large freelist invariant failed");
                IB_ASSERT(n->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE, "large freelist invariant failed");
                ++list_count;
            }
            observed_large_free_count += 1ull + list_count;
            // AVL balance property based on computed heights
            int height = 1 + (hl > hr ? hl : hr);
            int balance = hl - hr;
            IB_ASSERT(balance >= -1 && balance <= 1, "large freelist invariant failed");
            (void)list_count;
            return height;
        };
        if (at->root_free_block) {
            (void)validate_tree(validate_tree, at->root_free_block, 2048ull, ~0ull);
        }
        IB_ASSERT(observed_large_free_count == large_free_block_count, "large freelist invariant failed");
}
