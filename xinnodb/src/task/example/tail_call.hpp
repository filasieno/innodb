#pragma once

#include <type_traits>

// OPTION 1: With [[preserve_none]] - Maximum performance for CPS
#define IB_ASYNC [[noreturn]] __attribute__((preserve_none)) void

using reg_t = unsigned long long int;

// Combined compile-time validation: signature match, size check, no floats
template<typename Fn, typename... Args>
constexpr bool check_tail_call() {
    static_assert(std::is_invocable_v<Fn, Args...>, "co_do: Function signature doesn't match arguments");
    static_assert((... && (sizeof(Args) <= sizeof(reg_t))), "co_do: Arguments must be ≤ 8 bytes");
    static_assert((... && (!std::is_floating_point_v<Args>)), "co_do: Float/double not allowed (use XMM registers)");
    return true;
}

// Helper macros to count arguments (supports 0-13 arguments max)
#define VA_NARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(0 __VA_OPT__(,) __VA_ARGS__, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Tail call implementations for CPS-style async functions
// These functions never return, so no need to preserve callee-saved registers

#define TAIL_CALL_0(fn) do { \
    static_assert(check_tail_call<decltype(fn)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("leave; jmpq *%0" : : "r"((void*)(__fn)) : "memory"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_1(fn, a1) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%r12; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)) : "memory", "r12"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_2(fn, a1, a2) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)) : "memory", "rdi", "rsi"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_3(fn, a1, a2, a3) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)) : "memory", "rdi", "rsi", "rdx"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_4(fn, a1, a2, a3, a4) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)) : "memory", "rdi", "rsi", "rdx", "rcx"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_5(fn, a1, a2, a3, a4, a5) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_6(fn, a1, a2, a3, a4, a5, a6) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; movq %6, %%r9; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)), "r"((reg_t)(a6)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_7(fn, a1, a2, a3, a4, a5, a6, a7) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; movq %6, %%r9; movq %7, %%r10; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)), "r"((reg_t)(a6)), "r"((reg_t)(a7)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_8(fn, a1, a2, a3, a4, a5, a6, a7, a8) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; movq %6, %%r9; movq %7, %%r10; movq %8, %%r11; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)), "r"((reg_t)(a6)), "r"((reg_t)(a7)), "r"((reg_t)(a8)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_9(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; movq %6, %%r9; movq %7, %%r10; movq %8, %%r11; movq %9, %%r12; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)), "r"((reg_t)(a6)), "r"((reg_t)(a7)), "r"((reg_t)(a8)), "r"((reg_t)(a9)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11", "r12"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_10(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9), decltype(a10)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; movq %6, %%r9; movq %7, %%r10; movq %8, %%r11; movq %9, %%r12; movq %10, %%r13; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)), "r"((reg_t)(a6)), "r"((reg_t)(a7)), "r"((reg_t)(a8)), "r"((reg_t)(a9)), "r"((reg_t)(a10)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11", "r12", "r13"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_11(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9), decltype(a10), decltype(a11)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; movq %6, %%r9; movq %7, %%r10; movq %8, %%r11; movq %9, %%r12; movq %10, %%r13; movq %11, %%r14; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)), "r"((reg_t)(a6)), "r"((reg_t)(a7)), "r"((reg_t)(a8)), "r"((reg_t)(a9)), "r"((reg_t)(a10)), "r"((reg_t)(a11)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_12(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9), decltype(a10), decltype(a11), decltype(a12)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; movq %6, %%r9; movq %7, %%r10; movq %8, %%r11; movq %9, %%r12; movq %10, %%r13; movq %11, %%r14; movq %12, %%r15; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)), "r"((reg_t)(a6)), "r"((reg_t)(a7)), "r"((reg_t)(a8)), "r"((reg_t)(a9)), "r"((reg_t)(a10)), "r"((reg_t)(a11)), "r"((reg_t)(a12)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"); \
    __builtin_unreachable(); \
} while(0)

#define TAIL_CALL_13(fn, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) do { \
    static_assert(check_tail_call<decltype(fn), decltype(a1), decltype(a2), decltype(a3), decltype(a4), decltype(a5), decltype(a6), decltype(a7), decltype(a8), decltype(a9), decltype(a10), decltype(a11), decltype(a12), decltype(a13)>(), "co_do: invalid function signature or argument constraints"); \
    void* __fn = (void*)(fn); \
    asm volatile("movq %1, %%rdi; movq %2, %%rsi; movq %3, %%rdx; movq %4, %%rcx; movq %5, %%r8; movq %6, %%r9; movq %7, %%r10; movq %8, %%r11; movq %9, %%r12; movq %10, %%r13; movq %11, %%r14; movq %12, %%r15; movq %13, %%rbx; leave; jmpq *%0" : : "r"(__fn), "r"((reg_t)(a1)), "r"((reg_t)(a2)), "r"((reg_t)(a3)), "r"((reg_t)(a4)), "r"((reg_t)(a5)), "r"((reg_t)(a6)), "r"((reg_t)(a7)), "r"((reg_t)(a8)), "r"((reg_t)(a9)), "r"((reg_t)(a10)), "r"((reg_t)(a11)), "r"((reg_t)(a12)), "r"((reg_t)(a13)) : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rbx"); \
    __builtin_unreachable(); \
} while(0)

// Optimized tail call with strict validation
#define co_do(fn, ...) TAIL_CALL_DISPATCH(VA_NARGS(__VA_ARGS__), fn __VA_OPT__(,) __VA_ARGS__)

// Dispatch tail call based on argument count
#define TAIL_CALL_DISPATCH(n, ...) TAIL_CALL_DISPATCH_IMPL(n, __VA_ARGS__)
#define TAIL_CALL_DISPATCH_IMPL(n, ...) TAIL_CALL_DISPATCH_IMPL2(n, __VA_ARGS__)
#define TAIL_CALL_DISPATCH_IMPL2(n, ...) TAIL_CALL_##n(__VA_ARGS__)

#define co_if(cond, if_true, if_false, ...) \
   if (cond) { \
      co_do(if_true __VA_OPT__(,) __VA_ARGS__); \
   } else { \
      co_do(if_false __VA_OPT__(,) __VA_ARGS__); \
   }