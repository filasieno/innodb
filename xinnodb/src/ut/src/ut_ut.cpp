#include "ut_assert.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <cstdio>

void ut__assert_failed_func(const std::string_view message) noexcept {
    fmt::fprintf(stderr, message);
    std::fflush(stderr);
    __builtin_trap();
}

[[noreturn]] void ib_unreachable(const std::source_location loc) noexcept {
    static constexpr ut_comptime_string RED("\033[1;31m");
    static constexpr ut_comptime_string RESET("\033[0m");
    static constexpr auto fmt = RED + "{}:{}: Unreachable code reached\n" + RESET;
    auto message = fmt::format(
        static_cast<std::string_view>(fmt),
        loc.file_name(),
        (int)loc.line()
    );
    fmt::fprintf(stderr, message);
    std::fflush(stderr);
    __builtin_trap();
}