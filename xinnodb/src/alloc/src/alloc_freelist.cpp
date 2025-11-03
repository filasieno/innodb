#include "alloc.hpp" // IWYU pragma: keep
#include <cstdlib>

// Private Allocator API implementation
// ----------------------------------------------------------------------------------------------------------------
    
/// \brief Find the smallest free list that can store the alloc_size
/// 
/// \param alloc_size The size of the allocation
/// \param bit_field A pointer to a 64 byte aligned bit field
/// \return The index of the smallest free list that can store the alloc_size
/// \pre AVX2 is available
/// \pre bitField is 64 byte aligned
/// \internal
int alloc_freelist_find_index(const ib_u64* bit_field, ib_size alloc_size) noexcept {
    IB_ASSERT(bit_field != nullptr);
    // If no bins are populated, signal not found
    const ib_u64 word = *bit_field;
    if (word == 0ull) return -1;
    // Requests larger than the max small bin size are not eligible for small freelists
    if (alloc_size > 2048ull) return -1;
    // Map size to bin: bin = ceil(size/32) - 1, clamped to [0,63]
    ib_u64 required_bin = 0ull;
    if (alloc_size != 0) required_bin = (ib_u64)((alloc_size - 1u) >> 5);
    if (required_bin > 63ull) required_bin = 63ull;
    const ib_u64 mask = (~0ull) << required_bin;
    const ib_u64 value = word & mask;
    if (value == 0ull) return -1; // no suitable bin found
    return (int)__builtin_ctzll(value);
}


void alloc_freelist_set_mask(ib_u64* bit_field, ib_u64 bin_idx) noexcept {
    IB_ASSERT(bit_field != nullptr);
    IB_ASSERT(bin_idx < 64);
    *bit_field |= (1ull << bin_idx);
}

bool alloc_freelist_get_mask(const ib_u64* bit_field, ib_u64 bin_idx) noexcept {
    IB_ASSERT(bit_field != nullptr);
    IB_ASSERT(bin_idx < 64);
    return ((*bit_field >> bin_idx) & 1ull) != 0ull;
}

void alloc_freelist_clear_mask(ib_u64* bit_field, ib_u64 bin_idx) noexcept {
    IB_ASSERT(bit_field != nullptr);
    IB_ASSERT(bin_idx < 64);
    *bit_field &= ~(1ull << bin_idx);
}

alloc_block_header* alloc_block_next(alloc_block_header* header) noexcept {
    size_t sz = (size_t)header->this_desc.size;
    if (sz == 0) return header;
    return (alloc_block_header*)((char*)header + sz);
}

alloc_block_header* alloc_block_prev(alloc_block_header* header) noexcept {
    size_t sz = (size_t)header->prev_desc.size;
    if (sz == 0) return header;
    return (alloc_block_header*)((char*)header - sz);
}


ib_u64 alloc_freelist_get_index(ib_u64 sz) noexcept {
    // New mapping: 0..32 -> 0, 33..64 -> 1, ..., up to 2048 -> 63
    IB_ASSERT(sz > 0);
    ib_u64 bin = (ib_u64)((sz - 1ull) >> 5);
    const ib_u64 mask = (ib_u64)-(bin > 63u);
    bin = (bin & ~mask) | (63ull & mask);
    return bin;
}

ib_u32 alloc_freelist_get_index(const alloc_block_header* header) noexcept {
    switch ((enum alloc_block_state)header->this_desc.state) {
        case ALLOC_BLOCK_STATE_WILD_BLOCK:
            return 63;
        case ALLOC_BLOCK_STATE_FREE: 
        {
            const ib_u64 sz = header->this_desc.size;
            ib_u64 bin = (ib_u64)((sz - 1ull) >> 5);
            const ib_u64 mask = (ib_u64)-(bin > 63u);
            bin = (bin & ~mask) | (63u & mask);
            return bin;
        }
        case ALLOC_BLOCK_STATE_INVALID:
        case ALLOC_BLOCK_STATE_USED:
        case ALLOC_BLOCK_STATE_BEGIN_SENTINEL:
        case ALLOC_BLOCK_STATE_END_SENTINEL:
        default:
        {
            // Unreachable
            std::abort();
            return (ib_u32)~0;
        }
    }
}
