#include "alloc.hpp" // IWYU pragma: keep
#include <fmt/core.h>

constexpr const char* DEBUG_ALLOC_COLOR_RESET  = "\033[0m";
constexpr const char* DEBUG_ALLOC_COLOR_WHITE  = "\033[37m"; 
constexpr const char* DEBUG_ALLOC_COLOR_GREEN  = "\033[1;32m"; 
constexpr const char* DEBUG_ALLOC_COLOR_YELLOW = "\033[1;33m"; 
constexpr const char* DEBUG_ALLOC_COLOR_CYAN   = "\033[36m";  
constexpr const char* DEBUG_ALLOC_COLOR_RED    = "\033[1;31m"; 
constexpr const char* DEBUG_ALLOC_COLOR_HDR    = "\033[36m"; 

static inline constexpr const char* alloc_get_color_by_block_state(enum alloc_block_state s) {
    switch (s) {
        case ALLOC_BLOCK_STATE_USED:               
            return DEBUG_ALLOC_COLOR_CYAN;
        case ALLOC_BLOCK_STATE_FREE:   
        case ALLOC_BLOCK_STATE_WILD_BLOCK: 
            return DEBUG_ALLOC_COLOR_GREEN;
        case ALLOC_BLOCK_STATE_BEGIN_SENTINEL:
        case ALLOC_BLOCK_STATE_LARGE_BLOCK_SENTINEL:
        case ALLOC_BLOCK_STATE_END_SENTINEL: 
            return DEBUG_ALLOC_COLOR_YELLOW;
        case ALLOC_BLOCK_STATE_INVALID: 
            return DEBUG_ALLOC_COLOR_RED;
        default: 
            return DEBUG_ALLOC_COLOR_RESET;
    }
}

// Fixed column widths (constants) in requested order
constexpr int DEBUG_COL_W_OFF     = 18; // 0x + 16 hex
constexpr int DEBUG_COL_W_SIZE    = 12;
constexpr int DEBUG_COL_W_STATE   = 10;
constexpr int DEBUG_COL_W_PSIZE   = 12;
constexpr int DEBUG_COL_W_PSTATE  = 10;
constexpr int DEBUG_COL_W_FL_PREV = 18;
constexpr int DEBUG_COL_W_FL_NEXT = 18;

static inline void alloc_debug_print_run(const char* s, int n, const char* color = DEBUG_ALLOC_COLOR_WHITE) {
    for (int i = 0; i < n; ++i) fmt::print("{}{}", color, s);
}

static inline void alloc_debug_dump_top_border() {
    fmt::print("{}┌{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_OFF + 2);
    fmt::print("{}┬{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_SIZE + 2);
    fmt::print("{}┬{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_STATE + 2);
    fmt::print("{}┬{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_PSIZE + 2);
    fmt::print("{}┬{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_PSTATE + 2);
    fmt::print("{}┬{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_FL_PREV + 2);
    fmt::print("{}┬{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_FL_NEXT + 2);
    fmt::print("{}┐{}\n", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
}

static inline void alloc_debug_dump_separator() {
    fmt::print("{}├{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_OFF + 2);
    fmt::print("{}┼{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_SIZE + 2);
    fmt::print("{}┼{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_STATE + 2);
    fmt::print("{}┼{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_PSIZE + 2);
    fmt::print("{}┼{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_PSTATE + 2);
    fmt::print("{}┼{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_FL_PREV + 2);
    fmt::print("{}┼{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_FL_NEXT + 2);
    fmt::print("{}┤{}\n", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
}

static inline void alloc_debug_dump_bottim_border() {
    fmt::print("{}└{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_OFF + 2);
    fmt::print("{}┴{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_SIZE + 2);
    fmt::print("{}┴{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_STATE + 2);
    fmt::print("{}┴{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_PSIZE + 2);
    fmt::print("{}┴{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_PSTATE + 2);
    fmt::print("{}┴{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_FL_PREV + 2);
    fmt::print("{}┴{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    alloc_debug_print_run("─", DEBUG_COL_W_FL_NEXT + 2);
    fmt::print("{}┘{}\n", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
}

static inline void alloc_debug_dump_header() {
    fmt::print("{}│{}"       , DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<18} "  , DEBUG_ALLOC_COLOR_HDR,   "Offset");
    fmt::print("{}│{}"       , DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<12} "  , DEBUG_ALLOC_COLOR_HDR,   "ib_size");
    fmt::print("{}│{}"       , DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<10} "  , DEBUG_ALLOC_COLOR_HDR,   "State");
    fmt::print("{}│{}"       , DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<12} "  , DEBUG_ALLOC_COLOR_HDR,   "PrevSize");
    fmt::print("{}│{}"       , DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<10} "  , DEBUG_ALLOC_COLOR_HDR,   "PrevState");
    fmt::print("{}│{}"       , DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<18} "  , DEBUG_ALLOC_COLOR_HDR,   "FreeListPrev");
    fmt::print("{}│{}"       , DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<18} "  , DEBUG_ALLOC_COLOR_HDR,   "FreeListNext");
    fmt::print("{}│{}\n"     , DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
}

static inline void alloc_debug_dump_row(const alloc_table* at, const alloc_block_header* h) {
    
    uintptr_t begin_addr = (uintptr_t)at->sentinel_begin;
    uintptr_t off = (uintptr_t)h - begin_addr;
    uint64_t  sz  = (uint64_t)h->this_desc.size;
    uint64_t  psz = (uint64_t)h->prev_desc.size;
    enum alloc_block_state st = (enum alloc_block_state)h->this_desc.state;
    enum alloc_block_state pst = (enum alloc_block_state)h->prev_desc.state;

    const char* state_text = to_string(st);
    const char* previous_state_text = to_string(pst);
    const char* state_color = alloc_get_color_by_block_state(st);

    fmt::print("{}│{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<18} ", state_color, (unsigned long long)off);
    fmt::print("{}│{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<12} ", state_color, (unsigned long long)sz);
    fmt::print("{}│{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<10} ", state_color, state_text);
    fmt::print("{}│{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<12} ", state_color, (unsigned long long)psz);
    fmt::print("{}│{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    fmt::print("{} {:<10} ", state_color, previous_state_text);
    fmt::print("{}│{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
    
    ib_size bin_idx = alloc_freelist_get_index(h->this_desc.size);

    // Print FreeListPrev (with AkDLink)
    if (h->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE && h->this_desc.size <= 2048) {
        const ut_dlink* free_list_link = &((alloc_pooled_free_block_header*)h)->freelist_link;
        const ut_dlink* prev = free_list_link->prev;
        const ut_dlink* head = &at->freelist_head[bin_idx];
        if (prev == head) {
            fmt::print("{} {:<18} ", state_color, "HEAD");
        } else {
            const ib_size link_off = offsetof(alloc_pooled_free_block_header, freelist_link);
            alloc_block_header* prev_block = (alloc_block_header*)((char*)prev - link_off);
            ib_size offset = (ib_size)((char*)prev_block - (char*)at->sentinel_begin);
            fmt::print("{} {:<18} ", state_color, offset);
        }
    } else {
        fmt::print("{} {:<18} ", state_color, "");
    }

    fmt::print("{}│{}", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);

    // Print FreeList Next (with AkDLink)
    if (h->this_desc.state == (ib_u32)ALLOC_BLOCK_STATE_FREE && h->this_desc.size <= 2048) {
        const ut_dlink* free_list_link = &((alloc_pooled_free_block_header*)h)->freelist_link;
        const ut_dlink* next = free_list_link->next;
        const ut_dlink* head = &at->freelist_head[bin_idx];
        if (next == head) {
            fmt::print("{} {:<18} ", state_color, "HEAD");
        } else {
            const ib_size link_off = offsetof(alloc_pooled_free_block_header, freelist_link);
            alloc_block_header* next_block = (alloc_block_header*)((char*)next - link_off);
            ib_size offset = (ib_size)((char*)next_block - (char*)at->sentinel_begin);
            fmt::print("{} {:<18} ", state_color, offset);
        }
    } else {
        fmt::print("{} {:<18} ", state_color, "");
    }

    fmt::print("{}│{}\n", DEBUG_ALLOC_COLOR_WHITE, DEBUG_ALLOC_COLOR_RESET);
}

void dump_alloc_table(const alloc_table* at) noexcept {
    
    // Basic layout and sizes
    fmt::print("AllocTable: {}\n", (void*)at);
    
    fmt::print("  heapBegin        : {}\n", (void*)at->heap_begin);
    fmt::print("  heapEnd          : {}; size: {}\n", (void*)at->heap_end, (intptr_t)(at->heap_end - at->heap_begin));
    fmt::print("  memBegin         : {}\n", (void*)at->mem_begin);
    fmt::print("  memEnd           : {}; size: {}\n", (void*)at->mem_end,  (intptr_t)(at->mem_end  - at->mem_begin));
    fmt::print("  memSize          : {}\n", at->mem_size);
    fmt::print("  freeMemSize      : {}\n", at->free_mem_size);

    // Sentinels and wild/large tracking (addresses only; do not dereference)
    fmt::print("  Key Offsets:\n");
    fmt::print("    Begin sentinel offset: {}\n", (intptr_t)at->sentinel_begin      - (intptr_t)at->mem_begin);
    fmt::print("    Wild  block    offset: {}\n", (intptr_t)at->wild_block          - (intptr_t)at->mem_begin);
    fmt::print("    End   sentinel offset: {}\n", (intptr_t)at->sentinel_end        - (intptr_t)at->mem_begin);

    // Free list availability mask (64 bits)
    fmt::print("  FreeListbinMask:\n    ");
    ib_u64 mask = at->freelist_mask;
    for (unsigned i = 0; i < 64; ++i) {
        fmt::print("{}", (mask >> i) & 1ull);
    }
    fmt::print("\n");

    // Optional per-bin size accounting
    
    fmt::print("  FreeListBinsSizes begin\n");
    for (unsigned i = 0; i < 64; ++i) {
        unsigned cc = at->freelist_count[i];
        if (cc == 0) continue;
        fmt::print("    {:>5} bytes class  : {}\n", (i + 1) * 32, cc);
    }
    fmt::print("  FreeListBinsSizes end\n");
    fmt::print("\n");
}

void alloc_debug_dump_alloc_table(const alloc_table* at) noexcept 
{
    alloc_debug_dump_top_border();
    alloc_debug_dump_header();
    alloc_debug_dump_separator();
    alloc_block_header* head = (alloc_block_header*) at->sentinel_begin;
    alloc_block_header* end  = (alloc_block_header*) alloc_block_next((alloc_block_header*)at->sentinel_end);
    
    for (; head != end; head = alloc_block_next(head)) {
        alloc_debug_dump_row(at, head);
    }

    alloc_debug_dump_bottim_border();
}



