#pragma once

#include "ut_comptime_string.hpp"
#include <source_location>
#include <print>

/// \defgroup assert Assertions
/// \brief A set of macros to perform assertions
/// \ingroup ut

#define IB_ENABLE_ASSERT

#if defined(IB_ENABLE_ASSERT)
    // Minimal counting: we only care if there's 1 arg (no fmt) or 2+ (with fmt)
    #define IB_GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
    #define IB_COUNT_ARGS(...) IB_GET_NTH_ARG(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

    // IB_ASSERT dispatch
    #define IB_ASSERT_DISPATCH_1(cond)                       ib_assert<#cond, "">((cond), std::source_location::current())
    #define IB_ASSERT_DISPATCH_2(cond, fmt, ...)             ib_assert<#cond, fmt>((cond), std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_DISPATCH_3                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_4                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_5                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_6                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_7                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_8                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_DISPATCH_9                             IB_ASSERT_DISPATCH_2
    #define IB_ASSERT_SELECT(count)                          IB_ASSERT_SELECT_I(count)
    #define IB_ASSERT_SELECT_I(count)                        IB_ASSERT_DISPATCH_##count
    #define IB_ASSERT(...)                                   IB_ASSERT_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

    // IB_ASSERT_NOT_NULL dispatch
    #define IB_ASSERT_NOT_NULL_DISPATCH_1(ptr)               ib_assert<"(ptr) != nullptr", "">((ptr) != nullptr, std::source_location::current())
    #define IB_ASSERT_NOT_NULL_DISPATCH_2(ptr, fmt, ...)     ib_assert<"(ptr) != nullptr", fmt>((ptr) != nullptr, std::source_location::current(), ##__VA_ARGS__)
    #define IB_ASSERT_NOT_NULL_DISPATCH_3                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_4                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_5                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_6                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_7                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_8                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_DISPATCH_9                    IB_ASSERT_NOT_NULL_DISPATCH_2
    #define IB_ASSERT_NOT_NULL_SELECT(count)                 IB_ASSERT_NOT_NULL_SELECT_I(count)
    #define IB_ASSERT_NOT_NULL_SELECT_I(count)               IB_ASSERT_NOT_NULL_DISPATCH_##count
    #define IB_ASSERT_NOT_NULL(...)                          IB_ASSERT_NOT_NULL_SELECT(IB_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)
#else
    #define IB_ASSERT(...)
    #define IB_ASSERT_NOT_NULL(...)
#endif

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
inline static void ib_assert(bool condition, const std::source_location loc, Args&&... args) noexcept
{
    if (condition) return; // Assertion passed, nothing to do

    static constexpr ut_comptime_string RED("\033[1;31m");
    static constexpr ut_comptime_string RESET("\033[0m");

    if constexpr (!user_fmt.empty()) {
        if constexpr (sizeof...(Args) == 0) {
            // Simple message without arguments
            static constexpr auto fmt = RED + "{}:{}: Assertion '{}' failed: {}" + RESET + "\n";
            auto message = std::format(
                static_cast<std::string_view>(fmt),
                loc.file_name(),
                (int)loc.line(),
                condition_str,
                static_cast<std::string_view>(user_fmt));
            #if !defined(IB_ASSERT_TEST_MODE)
                ut__assert_failed_func(message);
            #else
                std::cout << message;
            #endif

        } else {
            // Message with arguments - format user message first, then combine
            static constexpr auto fmt = RED + "{}:{}: Assertion '{}' failed: " + user_fmt + RESET + "\n";
            auto message = std::format(
                static_cast<std::string_view>(fmt),
                loc.file_name(),
                (int)loc.line(),
                condition_str,
                std::forward<Args>(args)...);

            #if !defined(IB_ASSERT_TEST_MODE)
                ut__assert_failed_func(message);
            #else
                std::cout << message;
            #endif
        }

    } else {
        static_assert(sizeof...(Args) == 0, "Expected no arguments");
        static constexpr auto fmt = RED + "{}:{}: Assertion '{}' failed\n" + RESET;
        auto message = std::format(
            static_cast<std::string_view>(fmt),
            loc.file_name(),
            (int)loc.line(),
            condition_str);

        #if !defined(IB_ASSERT_TEST_MODE)
            ut__assert_failed_func(message);
        #else
            std::cout << message;
        #endif
    }
}

