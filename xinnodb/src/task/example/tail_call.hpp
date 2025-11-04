#pragma once

#include <type_traits>

// OPTION 1: With [[noreturn]] - Maximum performance for CPS
#define IB_ASYNC [[noreturn]] void

using reg_t = unsigned long long int;

// Combined compile-time validation: signature match, size check, no floats
template<typename Fn, typename... Args>
constexpr bool check_tail_call() {
    static_assert(std::is_invocable_v<Fn, Args...>, "ib_tail_call: Function signature doesn't match arguments");
    static_assert((... && (sizeof(Args) <= sizeof(reg_t))), "ib_tail_call: Arguments must be ≤ 8 bytes");
    static_assert((... && (!std::is_floating_point_v<Args>)), "ib_tail_call: Float/double not allowed (use XMM registers)");
    return true;
}

// Helper macros to count arguments (supports 0-13 arguments max)
#define VA_NARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(0 __VA_OPT__(,) __VA_ARGS__, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Tail call implementations for CPS-style async functions
// These functions never return, so no need to preserve callee-saved registers

#define TAIL_CALL_0(fn) do { \
    static_assert(check_tail_call<decltype(fn)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    asm volatile("leave; jmpq *%0" : : "r"((void*)(fn)) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_1(fn, a1) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    asm volatile("leave; jmpq *%0" : : "r"((void*)(fn)), "r"(r1) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_2(fn, a1, a2) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    asm volatile("leave; jmpq *%0" : : "r"((void*)(fn)), "r"(r1), "r"(r2) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_3(fn, a1, a2, a3) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    asm volatile("leave; jmpq *%0" : : "r"((void*)(fn)), "r"(r1), "r"(r2), "r"(r3) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_4(fn, a1, a2, a3, a4) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    asm volatile("leave; jmpq *%0" : : "r"((void*)(fn)), "r"(r1), "r"(r2), "r"(r3), "r"(r4) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_5(fn, a1, a2, a3, a4, a5) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    asm volatile("leave; jmpq *%0" : : "r"((void*)(fn)), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_6(fn, a1, a2, a3, a4, a5, a6) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    register reg_t r6 asm("r9")  = (reg_t)(a6); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"(__fn), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_7(fn, a1, a2, a3, a4, a5, a6, a7) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    register reg_t r6 asm("r9")  = (reg_t)(a6); \
    register reg_t r7 asm("r10") = (reg_t)(a7); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"(__fn), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_8(fn, a1, a2, a3, a4, a5, a6, a7, a8) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    register reg_t r6 asm("r9")  = (reg_t)(a6); \
    register reg_t r7 asm("r10") = (reg_t)(a7); \
    register reg_t r8 asm("r11") = (reg_t)(a8); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"(__fn), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_9(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    register reg_t r6 asm("r9")  = (reg_t)(a6); \
    register reg_t r7 asm("r10") = (reg_t)(a7); \
    register reg_t r8 asm("r11") = (reg_t)(a8); \
    register reg_t r9 asm("r12") = (reg_t)(a9); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"(__fn), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_10(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9), decltype(a10)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    register reg_t r6 asm("r9")  = (reg_t)(a6); \
    register reg_t r7 asm("r10") = (reg_t)(a7); \
    register reg_t r8 asm("r11") = (reg_t)(a8); \
    register reg_t r9 asm("r12") = (reg_t)(a9); \
    register reg_t r10 asm("r13") = (reg_t)(a10); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"(__fn), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_11(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9), decltype(a10), decltype(a11)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    register reg_t r6 asm("r9")  = (reg_t)(a6); \
    register reg_t r7 asm("r10") = (reg_t)(a7); \
    register reg_t r8 asm("r11") = (reg_t)(a8); \
    register reg_t r9 asm("r12") = (reg_t)(a9); \
    register reg_t r10 asm("r13") = (reg_t)(a10); \
    register reg_t r11 asm("r14") = (reg_t)(a11); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"(__fn), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10), "r"(r11) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_12(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9), decltype(a10), decltype(a11), decltype(a12)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    register reg_t r6 asm("r9")  = (reg_t)(a6); \
    register reg_t r7 asm("r10") = (reg_t)(a7); \
    register reg_t r8 asm("r11") = (reg_t)(a8); \
    register reg_t r9 asm("r12") = (reg_t)(a9); \
    register reg_t r10 asm("r13") = (reg_t)(a10); \
    register reg_t r11 asm("r14") = (reg_t)(a11); \
    register reg_t r12 asm("r15") = (reg_t)(a12); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"(__fn), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10), "r"(r11), "r"(r12) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_13(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9), decltype(a10), decltype(a11), decltype(a12), decltype(a13)>(), "ib_tail_call: invalid function signature or argument constraints"); \
    register reg_t r1 asm("rdi") = (reg_t)(a1); \
    register reg_t r2 asm("rsi") = (reg_t)(a2); \
    register reg_t r3 asm("rdx") = (reg_t)(a3); \
    register reg_t r4 asm("rcx") = (reg_t)(a4); \
    register reg_t r5 asm("r8")  = (reg_t)(a5); \
    register reg_t r6 asm("r9")  = (reg_t)(a6); \
    register reg_t r7 asm("r10") = (reg_t)(a7); \
    register reg_t r8 asm("r11") = (reg_t)(a8); \
    register reg_t r9 asm("r12") = (reg_t)(a9); \
    register reg_t r10 asm("r13") = (reg_t)(a10); \
    register reg_t r11 asm("r14") = (reg_t)(a11); \
    register reg_t r12 asm("r15") = (reg_t)(a12); \
    register reg_t r13 asm("rbx") = (reg_t)(a13); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"(__fn), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10), "r"(r11), "r"(r12), "r"(r13) : "memory"); \
    __builtin_unreachable(); \
} while(0)

// Optimized tail call with strict validation
#define ib_tail_call(fn, ...) TAIL_CALL_DISPATCH(VA_NARGS(__VA_ARGS__), fn __VA_OPT__(,) __VA_ARGS__)

// Dispatch tail call based on argument count
#define TAIL_CALL_DISPATCH(n, ...) TAIL_CALL_DISPATCH_IMPL(n, __VA_ARGS__)
#define TAIL_CALL_DISPATCH_IMPL(n, ...) TAIL_CALL_DISPATCH_IMPL2(n, __VA_ARGS__)
#define TAIL_CALL_DISPATCH_IMPL2(n, ...) TAIL_CALL_##n(__VA_ARGS__)
