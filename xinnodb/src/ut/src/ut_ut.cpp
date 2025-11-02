#include "ut_assert.hpp"

#include <print>

void ut__assert_failed_func(const std::string_view message) noexcept {
    std::print("{}", message);
    std::fflush(stdout);
    std::abort();
}