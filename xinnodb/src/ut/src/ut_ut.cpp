#include "ut_assert.hpp"

#include <print>
#include <cstdio>

void ut__assert_failed_func(const std::string_view message) noexcept {
    std::fwrite(message.data(), 1, message.size(), stderr);
    std::fflush(stderr);
    std::abort();
}