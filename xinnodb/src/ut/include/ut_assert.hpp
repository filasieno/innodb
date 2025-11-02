#pragma once

#include "ut_comptime_string.hpp"
#include <source_location>
#include <cstdlib>
#include <print>

/// \defgroup assert Assertions
/// \brief A set of macros to perform assertions

/// \ingroup ut

#if !defined(IB_ENABLE_ASSERT)
/// \brief Enable assertions by default
#define IB_ENABLE_ASSERT true
#endif

#if IB_ENABLE_ASSERT == true
    // Minimal counting: we only care if there's 1 arg (no fmt) or 2+ (with fmt)
    #define IB_GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
    #define IB_COUNT_ARGS(...) IB_GET_NTH_ARG(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

    // IB_ASSERT dispatch
    #define IB_ASSERT_DISPATCH_1(cond)                       ut_assert<#cond, "">((cond), std::source_location::current())
    #define IB_ASSERT_DISPATCH_2(cond, fmt, ...)             ut_assert<#cond, fmt>((cond), std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_DISPATCH_3                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_4                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_5                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_6                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_7                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_8                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_9                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_SELECT(count)                          IB_ASSERT_SELECT_I(count)
    #define IB_ASSERT_SELECT_I(count)                        IB_ASSERT_DISPATCH_##count
    #define IB_ASSERT(...)                                   do { IB_ASSERT_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)

    // IB_ASSERT_NOT_NULL dispatch
    #define IB_ASSERT_NOT_NULL_DISPATCH_1(ptr)               ut_assert<"ptr != nullptr", "">((ptr) != nullptr, std::source_location::current())
    #define IB_ASSERT_NOT_NULL_DISPATCH_2(ptr, fmt, ...)     ut_assert<"ptr != nullptr", fmt>((ptr) != nullptr, std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_NOT_NULL_DISPATCH_3                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_4                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_5                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_6                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_7                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_8                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_9                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_SELECT(count)                 IB_ASSERT_NOT_NULL_SELECT_I(count)
    #define IB_ASSERT_NOT_NULL_SELECT_I(count)               IB_ASSERT_NOT_NULL_DISPATCH_##count
    #define IB_ASSERT_NOT_NULL(...)                          do { IB_ASSERT_NOT_NULL_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)

    // Binary comparison assertions (2 args => no fmt; 3+ args => fmt + args)
    // ==
    #define IB_ASSERT_EQ_DISPATCH_2(a, b)                    ut_assert<(#a " == " #b ), "">(((a) == (b)), std::source_location::current())
    #define IB_ASSERT_EQ_DISPATCH_3(a, b, fmt, ...)          ut_assert<(#a " == " #b ), fmt>(((a) == (b)), std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_EQ_DISPATCH_4                          IB_ASSERT_EQ_DISPATCH_3
    #define IB_ASSERT_EQ_DISPATCH_5                          IB_ASSERT_EQ_DISPATCH_3
    #define IB_ASSERT_EQ_DISPATCH_6                          IB_ASSERT_EQ_DISPATCH_3
    #define IB_ASSERT_EQ_DISPATCH_7                          IB_ASSERT_EQ_DISPATCH_3
    #define IB_ASSERT_EQ_DISPATCH_8                          IB_ASSERT_EQ_DISPATCH_3
    #define IB_ASSERT_EQ_DISPATCH_9                          IB_ASSERT_EQ_DISPATCH_3
    #define IB_ASSERT_EQ_SELECT(count)                       IB_ASSERT_EQ_SELECT_I(count)
    #define IB_ASSERT_EQ_SELECT_I(count)                     IB_ASSERT_EQ_DISPATCH_##count
    #define IB_ASSERT_EQ(...)                                do { IB_ASSERT_EQ_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)

    // !=
    #define IB_ASSERT_NEQ_DISPATCH_2(a, b)                   ut_assert<(#a " != " #b ), "">(((a) != (b)), std::source_location::current())
    #define IB_ASSERT_NEQ_DISPATCH_3(a, b, fmt, ...)         ut_assert<(#a " != " #b ), fmt>(((a) != (b)), std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_NEQ_DISPATCH_4                         IB_ASSERT_NEQ_DISPATCH_3
    #define IB_ASSERT_NEQ_DISPATCH_5                         IB_ASSERT_NEQ_DISPATCH_3
    #define IB_ASSERT_NEQ_DISPATCH_6                         IB_ASSERT_NEQ_DISPATCH_3
    #define IB_ASSERT_NEQ_DISPATCH_7                         IB_ASSERT_NEQ_DISPATCH_3
    #define IB_ASSERT_NEQ_DISPATCH_8                         IB_ASSERT_NEQ_DISPATCH_3
    #define IB_ASSERT_NEQ_DISPATCH_9                         IB_ASSERT_NEQ_DISPATCH_3
    #define IB_ASSERT_NEQ_SELECT(count)                      IB_ASSERT_NEQ_SELECT_I(count)
    #define IB_ASSERT_NEQ_SELECT_I(count)                    IB_ASSERT_NEQ_DISPATCH_##count
    #define IB_ASSERT_NEQ(...)                               do { IB_ASSERT_NEQ_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)

    // <
    #define IB_ASSERT_LT_DISPATCH_2(a, b)                    ut_assert<(#a " < " #b), "">(((a) < (b)), std::source_location::current())
    #define IB_ASSERT_LT_DISPATCH_3(a, b, fmt, ...)          ut_assert<(#a " < " #b), fmt>(((a) < (b)), std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_LT_DISPATCH_4                          IB_ASSERT_LT_DISPATCH_3
    #define IB_ASSERT_LT_DISPATCH_5                          IB_ASSERT_LT_DISPATCH_3
    #define IB_ASSERT_LT_DISPATCH_6                          IB_ASSERT_LT_DISPATCH_3
    #define IB_ASSERT_LT_DISPATCH_7                          IB_ASSERT_LT_DISPATCH_3
    #define IB_ASSERT_LT_DISPATCH_8                          IB_ASSERT_LT_DISPATCH_3
    #define IB_ASSERT_LT_DISPATCH_9                          IB_ASSERT_LT_DISPATCH_3
    #define IB_ASSERT_LT_SELECT(count)                       IB_ASSERT_LT_SELECT_I(count)
    #define IB_ASSERT_LT_SELECT_I(count)                     IB_ASSERT_LT_DISPATCH_##count
    #define IB_ASSERT_LT(...)                                do { IB_ASSERT_LT_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)

    // >
    #define IB_ASSERT_GT_DISPATCH_2(a, b)                    ut_assert<(#a " > " #b), "">(((a) > (b)), std::source_location::current())
    #define IB_ASSERT_GT_DISPATCH_3(a, b, fmt, ...)          ut_assert<(#a " > " #b), fmt>(((a) > (b)), std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_GT_DISPATCH_4                          IB_ASSERT_GT_DISPATCH_3
    #define IB_ASSERT_GT_DISPATCH_5                          IB_ASSERT_GT_DISPATCH_3
    #define IB_ASSERT_GT_DISPATCH_6                          IB_ASSERT_GT_DISPATCH_3
    #define IB_ASSERT_GT_DISPATCH_7                          IB_ASSERT_GT_DISPATCH_3
    #define IB_ASSERT_GT_DISPATCH_8                          IB_ASSERT_GT_DISPATCH_3
    #define IB_ASSERT_GT_DISPATCH_9                          IB_ASSERT_GT_DISPATCH_3
    #define IB_ASSERT_GT_SELECT(count)                       IB_ASSERT_GT_SELECT_I(count)
    #define IB_ASSERT_GT_SELECT_I(count)                     IB_ASSERT_GT_DISPATCH_##count
    #define IB_ASSERT_GT(...)                                do { IB_ASSERT_GT_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)

    // >= (not less than)
    #define IB_ASSERT_NLT_DISPATCH_2(a, b)                   ut_assert<("!(" #a " <= " #b ")"), "">(((a) >= (b)), std::source_location::current())
    #define IB_ASSERT_NLT_DISPATCH_3(a, b, fmt, ...)         ut_assert<("!(" #a " <= " #b ")"), fmt>(((a) >= (b)), std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_NLT_DISPATCH_4                         IB_ASSERT_NLT_DISPATCH_3
    #define IB_ASSERT_NLT_DISPATCH_5                         IB_ASSERT_NLT_DISPATCH_3
    #define IB_ASSERT_NLT_DISPATCH_6                         IB_ASSERT_NLT_DISPATCH_3
    #define IB_ASSERT_NLT_DISPATCH_7                         IB_ASSERT_NLT_DISPATCH_3
    #define IB_ASSERT_NLT_DISPATCH_8                         IB_ASSERT_NLT_DISPATCH_3
    #define IB_ASSERT_NLT_DISPATCH_9                         IB_ASSERT_NLT_DISPATCH_3
    #define IB_ASSERT_NLT_SELECT(count)                      IB_ASSERT_NLT_SELECT_I(count)
    #define IB_ASSERT_NLT_SELECT_I(count)                    IB_ASSERT_NLT_DISPATCH_##count
    #define IB_ASSERT_NLT(...)                               do { IB_ASSERT_NLT_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)

    // <= (not greater than)
    #define IB_ASSERT_NGT_DISPATCH_2(a, b)                   ut_assert<("!(" #a " >= " #b ")"), "">(((a) <= (b)), std::source_location::current())
    #define IB_ASSERT_NGT_DISPATCH_3(a, b, fmt, ...)         ut_assert<("!(" #a " >= " #b ")"), fmt>(((a) <= (b)), std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_NGT_DISPATCH_4                         IB_ASSERT_NGT_DISPATCH_3
    #define IB_ASSERT_NGT_DISPATCH_5                         IB_ASSERT_NGT_DISPATCH_3
    #define IB_ASSERT_NGT_DISPATCH_6                         IB_ASSERT_NGT_DISPATCH_3
    #define IB_ASSERT_NGT_DISPATCH_7                         IB_ASSERT_NGT_DISPATCH_3
    #define IB_ASSERT_NGT_DISPATCH_8                         IB_ASSERT_NGT_DISPATCH_3
    #define IB_ASSERT_NGT_DISPATCH_9                         IB_ASSERT_NGT_DISPATCH_3
    #define IB_ASSERT_NGT_SELECT(count)                      IB_ASSERT_NGT_SELECT_I(count)
    #define IB_ASSERT_NGT_SELECT_I(count)                    IB_ASSERT_NGT_DISPATCH_##count
    #define IB_ASSERT_NGT(...)                               do { IB_ASSERT_NGT_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)                        
#else
    #define IB_ASSERT(...)
    #define IB_ASSERT_NOT_NULL(...)
    #define IB_ASSERT_EQ(...)
    #define IB_ASSERT_NEQ(...)
    #define IB_ASSERT_GT(...)
    #define IB_ASSERT_LT(...)
    #define IB_ASSERT_NGT(...)
    #define IB_ASSERT_NLT(...)
#endif

// Explicit failure: IB_FAIL(fmt) or IB_FAIL(fmt, args...) -> ut_fatal_error
#define IB_FAIL_DISPATCH_0()         ut_fatal_error<>(std::source_location::current())
#define IB_FAIL_DISPATCH_1(fmt)      ut_fatal_error<fmt>(std::source_location::current())
#define IB_FAIL_DISPATCH_2(fmt, ...) ut_fatal_error<fmt>(std::source_location::current(), ##__VA_ARGS__)
#define IB_FAIL_DISPATCH_3           IB_FAIL_DISPATCH_2
#define IB_FAIL_DISPATCH_4           IB_FAIL_DISPATCH_2
#define IB_FAIL_DISPATCH_5           IB_FAIL_DISPATCH_2
#define IB_FAIL_DISPATCH_6           IB_FAIL_DISPATCH_2
#define IB_FAIL_DISPATCH_7           IB_FAIL_DISPATCH_2
#define IB_FAIL_DISPATCH_8           IB_FAIL_DISPATCH_2
#define IB_FAIL_DISPATCH_9           IB_FAIL_DISPATCH_2
#define IB_FAIL_SELECT(count)        IB_FAIL_SELECT_I(count)
#define IB_FAIL_SELECT_I(count)      IB_FAIL_DISPATCH_##count
#define IB_FAIL(...)                 do { IB_FAIL_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__); } while (0)

/// \brief Function to handle assertion failures
/// \param message The message to print
/// \ingroup assert
[[noreturn]] void ut__assert_failed_func(const std::string_view message) noexcept;


/// \brief Compile-time string formatting
/// \param condition_str The condition string
/// \param user_fmt The user format string
/// \param Args The argument types
/// \param condition The condition to check
/// \param loc The source location
/// \param args The arguments to format
/// \ingroup assert
template <ut_comptime_string condition_str, ut_comptime_string user_fmt = "", typename... Args>
requires ( (std::formattable<std::remove_cvref_t<Args>, char>) && ... )
inline static void ut_assert(bool condition, const std::source_location loc, Args&&... args) noexcept
{
    if (condition) return; // Assertion passed, nothing to do

    static constexpr ut_comptime_string RED("\033[1;31m");
    static constexpr ut_comptime_string RESET("\033[0m");

    if constexpr (!user_fmt.empty()) {
        static constexpr auto fmt = RED + "{}:{}: Assertion '{}' failed: " + user_fmt + RESET + "\n";
        if constexpr (sizeof...(Args) == 0) {
            // Simple message without arguments
            auto message = std::format(
                static_cast<std::string_view>(fmt),
                loc.file_name(),
                (int)loc.line(),
                condition_str
            );
            ut__assert_failed_func(message);
        } else {
            // Message with arguments - format user message first, then combine
            auto message = std::format(
                static_cast<std::string_view>(fmt),
                loc.file_name(),
                (int)loc.line(),
                condition_str,
                std::forward<Args>(args)...
            );
            ut__assert_failed_func(message);
        }

    } else {
        static_assert(sizeof...(Args) == 0, "Expected no arguments");
        static constexpr auto fmt = RED + "{}:{}: Assertion '{}' failed\n" + RESET;
        auto message = std::format(
            static_cast<std::string_view>(fmt),
            loc.file_name(),
            (int)loc.line(),
            condition_str
        );
        ut__assert_failed_func(message);
    }
}

/// \brief Fatal error emitter that always terminates (ignores test mode)
/// \param user_fmt The user format string (compile-time)
/// \param Args The argument types
/// \param loc The source location
/// \param args The arguments to format
/// \ingroup assert
template <ut_comptime_string user_fmt = "", typename... Args>
requires ( (std::formattable<std::remove_cvref_t<Args>, char>) && ... )
[[noreturn]] inline static void ut_fatal_error(const std::source_location loc, Args&&... args) noexcept
{
    static constexpr ut_comptime_string RED("\033[1;31m");
    static constexpr ut_comptime_string RESET("\033[0m");

    if constexpr (!user_fmt.empty()) {
        if constexpr (sizeof...(Args) == 0) {
            static constexpr auto fmt = RED + "{}:{}: Fatal error: {}" + RESET + "\n";
            auto message = std::format(
                static_cast<std::string_view>(fmt),
                loc.file_name(),
                (int)loc.line(),
                static_cast<std::string_view>(user_fmt));
            ut__assert_failed_func(message);
        } else {
            static constexpr auto fmt = RED + "{}:{}: Fatal error: " + user_fmt + RESET + "\n";
            auto message = std::format(
                static_cast<std::string_view>(fmt),
                loc.file_name(),
                (int)loc.line(),
                std::forward<Args>(args)...);
            ut__assert_failed_func(message);
        }
    } else {
        static constexpr auto fmt = RED + "{}:{}: Fatal error" + RESET + "\n";
        auto message = std::format(
            static_cast<std::string_view>(fmt),
            loc.file_name(),
            (int)loc.line());
        ut__assert_failed_func(message);
    }
}

/// \brief Unreachable code marker; always terminates
/// \ingroup assert
[[noreturn]] inline void ib_unreachable(const std::source_location loc = std::source_location::current()) noexcept
{
    static constexpr ut_comptime_string RED("\033[1;31m");
    static constexpr ut_comptime_string RESET("\033[0m");
    static constexpr auto fmt = RED + "{}:{}: Unreachable code reached\n" + RESET;
    auto message = std::format(
        static_cast<std::string_view>(fmt),
        loc.file_name(),
        (int)loc.line());

    ut__assert_failed_func(message);

    #if defined(__clang__) || defined(__GNUC__)
        __builtin_unreachable();
    #elif defined(_MSC_VER)
        __assume(false);
    #endif
}

