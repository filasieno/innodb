#include "alloc.hpp" // IWYU pragma: keep
#include "ut.hpp"


void alloc_table_check_invariants(alloc_table* at, std::source_location loc) noexcept {

        // Basic table invariants
        IB_ASSERT_LT(at->heap_begin, at->mem_begin, "heap_begin < mem_begin; heap_begin={} mem_begin={}", (void*)at->heap_begin, (void*)at->mem_begin);
        IB_ASSERT_LT(at->mem_begin,  at->mem_end,   "mem_begin < mem_end; mem_begin={} mem_end={}",       (void*)at->mem_begin,  (void*)at->mem_end);
        IB_ASSERT_LT(at->mem_end,    at->heap_end,  "mem_end < heap_end; mem_end={} heap_end={}",         (void*)at->mem_end,    (void*)at->heap_end);
        IB_ASSERT_EQ(((ib_u64)at->mem_begin & 31ull), 0ull, "mem_begin must be 32B aligned; mem_begin={} remainder={}", (void*)at->mem_begin, ((ib_u64)at->mem_begin & 31ull));
        IB_ASSERT_EQ(((ib_u64)at->mem_end   & 31ull), 0ull, "mem_end must be 32B aligned; mem_end={} remainder={}",     (void*)at->mem_end,   ((ib_u64)at->mem_end   & 31ull));
        IB_ASSERT_EQ(at->mem_size, (ib_size)(at->mem_end - at->mem_begin), "mem_size must equal (mem_end - mem_begin); mem_size={} expected={}", at->mem_size, (ib_size)(at->mem_end - at->mem_begin));

        // Sentinels positioning invariants
        IB_ASSERT_EQ((void*)at->sentinel_begin, (void*)at->mem_begin, "begin sentinel must be placed at mem_begin; sentinel_begin={} mem_begin={} ", (void*)at->sentinel_begin, (void*)at->mem_begin);
        IB_ASSERT_EQ(((ib_u64)at->sentinel_begin->this_desc.size % 32ull), 0ull, "begin sentinel size must be 32B multiple; size={}", (ib_u64)at->sentinel_begin->this_desc.size);
        IB_ASSERT_EQ(at->sentinel_begin->this_desc.state, (ib_u32)ALLOC_BLOCK_STATE_BEGIN_SENTINEL, "begin sentinel state must be BEGIN_SENTINEL; state={}", (ib_u32)at->sentinel_begin->this_desc.state);
        IB_ASSERT_EQ(at->sentinel_begin->prev_desc.size, 0ull, "begin sentinel prev_desc.size must be 0; prev_desc.size={}", (ib_u64)at->sentinel_begin->prev_desc.size);

        alloc_pooled_free_block_header* expected_end = (alloc_pooled_free_block_header*)((char*)at->mem_end - sizeof(alloc_pooled_free_block_header));
        IB_ASSERT_EQ((void*)at->sentinel_end, (void*)expected_end, "end sentinel must be placed at mem_end - sizeof(header); sentinel_end={} expected={} ", (void*)at->sentinel_end, (void*)expected_end);
        IB_ASSERT_EQ(((ib_u64)at->sentinel_end->this_desc.size % 32ull), 0ull, "end sentinel size must be 32B multiple; size={}", (ib_u64)at->sentinel_end->this_desc.size);
        IB_ASSERT_EQ(at->sentinel_end->this_desc.state, (ib_u32)ALLOC_BLOCK_STATE_END_SENTINEL, "end sentinel state must be END_SENTINEL; state={}", (ib_u32)at->sentinel_end->this_desc.state);

        // Wild block basic invariants
        IB_ASSERT_NOT_NULL(at->wild_block, "wild_block must be non-null");
        IB_ASSERT_NLT((char*)at->wild_block, at->mem_begin, "wild_block must lie within [mem_begin, mem_end); wild_block={} mem_begin={}", (void*)at->wild_block, (void*)at->mem_begin);
        IB_ASSERT_LT((char*)at->wild_block,  at->mem_end,   "wild_block must lie within [mem_begin, mem_end); wild_block={} mem_end={} ",   (void*)at->wild_block, (void*)at->mem_end);
        IB_ASSERT_EQ(((ib_u64)at->wild_block & 31ull), 0ull, "wild_block must be 32B aligned; addr={} remainder={}", (void*)at->wild_block, ((ib_u64)at->wild_block & 31ull));
        IB_ASSERT_EQ(at->wild_block->this_desc.state, (ib_u32)ALLOC_BLOCK_STATE_WILD_BLOCK, "wild_block state must be WILD_BLOCK; state={}", (ib_u32)at->wild_block->this_desc.state);

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
            IB_ASSERT_NLT((char*)h, at->mem_begin, "block must lie within [mem_begin, mem_end); block={} mem_begin={}", (void*)h, (void*)at->mem_begin);
            IB_ASSERT_LT((char*)h,  at->mem_end,   "block must lie within [mem_begin, mem_end); block={} mem_end={} ",     (void*)h, (void*)at->mem_end);
            IB_ASSERT_EQ(((ib_u64)h & 31ull), 0ull, "block address must be 32B aligned; addr={} remainder={}", (void*)h, ((ib_u64)h & 31ull));

            // ib_size constraints
            ib_u64 sz = h->this_desc.size;
            IB_ASSERT_NLT(sz, (ib_u64)sizeof(alloc_block_header), "block size must be >= header size; size={} header_size={}", sz, (ib_u64)sizeof(alloc_block_header));
            IB_ASSERT_EQ((sz & 31ull), 0ull, "block size must be a multiple of 32; size={} remainder={}", sz, (sz & 31ull));

            // Prev descriptor consistency
            if (prev) {
                IB_ASSERT_EQ(h->prev_desc.size,  prev->this_desc.size,  "prev_desc.size must equal previous this_desc.size; prev_size={} this_prev_size={} block={}", (ib_u64)h->prev_desc.size, (ib_u64)prev->this_desc.size, (void*)h);
                IB_ASSERT_EQ(h->prev_desc.state, prev->this_desc.state, "prev_desc.state must equal previous this_desc.state; prev_state={} this_prev_state={} block={}", (ib_u32)h->prev_desc.state, (ib_u32)prev->this_desc.state, (void*)h);
                // Bidirectional linkage check
                IB_ASSERT_EQ(alloc_block_next((alloc_block_header*)prev), h,    "next(prev) must equal current; prev={} next(prev)={} current={} ", (void*)prev, (void*)alloc_block_next((alloc_block_header*)prev), (void*)h);
                IB_ASSERT_EQ(alloc_block_prev((alloc_block_header*)h),    prev, "prev(current) must equal prev; current={} prev(current)={} prev={} ", (void*)h, (void*)alloc_block_prev((alloc_block_header*)h), (void*)prev);
            } else {
                // First block is the begin sentinel
                IB_ASSERT_EQ(h, begin, "first block must be the begin sentinel; block={} begin={} ", (void*)h, (void*)begin);
                IB_ASSERT_EQ(h->this_desc.state, (ib_u32)ALLOC_BLOCK_STATE_BEGIN_SENTINEL, "first block must have BEGIN_SENTINEL state; state={}", (ib_u32)h->this_desc.state);
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
                    IB_ASSERT_EQ((alloc_block_header*)h, (alloc_block_header*)at->wild_block, "wild block header must match table wild_block pointer; block={} wild_block={} ", (void*)h, (void*)at->wild_block);
                    IB_ASSERT_NLT(sz, 32ull, "wild block size must be >= 32; size={}", sz);
                    ++wild_block_instances;
                    counted_wild_bytes += sz;
                    counted_free_bytes += sz;
                    break;
                case ALLOC_BLOCK_STATE_FREE:
                    IB_ASSERT_NLT(sz, 32ull, "free block size must be >= 32; size={}", sz);
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
        IB_ASSERT_EQ(wild_block_instances, 1ull, "there must be exactly one wild block; observed={}", wild_block_instances);

        // Memory accounting: free + used should equal mem_size
        IB_ASSERT_EQ(counted_free_bytes + counted_used_bytes, at->mem_size,  "memory accounting failed: free({}) + used({}) must equal mem_size({})", counted_free_bytes, counted_used_bytes, at->mem_size);
        IB_ASSERT_EQ(counted_free_bytes, at->free_mem_size,                   "free bytes must equal table free_mem_size; free={} table.free_mem_size={}", counted_free_bytes, at->free_mem_size);

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
            IB_ASSERT_EQ(ring_count, (ib_u64)at->freelist_count[bin], "freelist[{}] count must match ring size; ring_count={} freelist_count={} ", bin, ring_count, (ib_u64)at->freelist_count[bin]);
            IB_ASSERT_EQ(ring_count, small_free_count_bin[bin],       "freelist[{}] count must match observed free blocks; ring_count={} observed={} ", bin, ring_count, small_free_count_bin[bin]);
            const bool mask_bit = ((at->freelist_mask >> bin) & 1ull) != 0ull;
            IB_ASSERT_EQ(mask_bit, (ring_count > 0),                  "freelist mask bit must reflect non-empty bin; bin={} mask_bit={} non_empty={} ", bin, mask_bit, (ring_count > 0));
        }
        IB_ASSERT_EQ(observed_mask, at->freelist_mask, "freelist mask must equal observed; observed={:064b} table={:064b}", observed_mask, at->freelist_mask);

        // Validate large free block AVL tree: states and ordering
        ib_u64 observed_large_free_count = 0ull;
        auto validate_tree = [&](auto&& self, alloc_free_block_header* node, ib_u64 min_key, ib_u64 max_key) -> int {
            if (!node) return 0;
            ib_u64 key = node->this_desc.size;
            IB_ASSERT_GT(key, 2048ull, "tree node key (size) must be > 2048; size={}", key);
            IB_ASSERT_GT(key, min_key, "tree in-order property violated (left subtree < key); key={} min_key={}", key, min_key);
            IB_ASSERT_LT(key, max_key, "tree in-order property violated (key < right subtree); key={} max_key={}", key, max_key);
            IB_ASSERT_EQ(node->this_desc.state, (ib_u32)ALLOC_BLOCK_STATE_FREE, "tree node state must be FREE; state={}", (ib_u32)node->this_desc.state);
            // children parent pointers
            if (node->left)  IB_ASSERT_EQ(node->left->parent,  node, "left child parent pointer must point to node; left={} parent={} node={}",  (void*)node->left,  (void*)node->left->parent,  (void*)node);
            if (node->right) IB_ASSERT_EQ(node->right->parent, node, "right child parent pointer must point to node; right={} parent={} node={}", (void*)node->right, (void*)node->right->parent, (void*)node);
            // left subtree
            int hl = self(self, node->left, min_key, key);
            // right subtree
            int hr = self(self, node->right, key, max_key);
            // multimap ring: all nodes must have the same size and FREE state
            ib_u64 list_count = 0ull;
            for (ut_dlink* it = node->multimap_link.next; it != &node->multimap_link; it = it->next) {
                alloc_free_block_header* n = (alloc_free_block_header*)((char*)it - IB_OFFSET_OF(alloc_free_block_header, multimap_link));
                IB_ASSERT_EQ(n->this_desc.size, key, "multimap ring node size must equal group key; node_size={} key={}", (ib_u64)n->this_desc.size, key);
                IB_ASSERT_EQ(n->this_desc.state, (ib_u32)ALLOC_BLOCK_STATE_FREE, "multimap ring node state must be FREE; state={}", (ib_u32)n->this_desc.state);
                ++list_count;
            }
            observed_large_free_count += 1ull + list_count;
            // AVL balance property based on computed heights
            int height = 1 + (hl > hr ? hl : hr);
            int balance = hl - hr;
            IB_ASSERT_NLT(balance, -1, "AVL balance must be >= -1; balance={} node_size={}", balance, key);
            IB_ASSERT_NGT(balance,  1, "AVL balance must be <= 1; balance={} node_size={}", balance, key);
            (void)list_count;
            return height;
        };
        if (at->root_free_block) {
            (void)validate_tree(validate_tree, at->root_free_block, 2048ull, ~0ull);
        }
        IB_ASSERT_EQ(observed_large_free_count, large_free_block_count, "tree free block count must equal observed; observed={} expected={}", observed_large_free_count, large_free_block_count);
}
