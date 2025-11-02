#include "ut_assert.hpp"

#include <print>
#include <cstdio>

void ut__assert_failed_func(const std::string_view message) noexcept {
    std::print(stderr, "{}", message);
    std::fflush(stderr);
    std::abort();
}

[[noreturn]] void ib_unreachable(const std::source_location loc) noexcept {
    static constexpr ut_comptime_string RED("\033[1;31m");
    static constexpr ut_comptime_string RESET("\033[0m");
    static constexpr auto fmt = RED + "{}:{}: Unreachable code reached\n" + RESET;
    auto message = std::format(
        static_cast<std::string_view>(fmt),
        loc.file_name(),
        (int)loc.line()
    );
    std::print(stderr, "{}", message);
    std::fflush(stderr);
    std::abort();
}