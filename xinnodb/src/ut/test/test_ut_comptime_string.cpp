#include <gtest/gtest.h>
#include "ut_comptime_string.hpp"

// Test compile-time string functionality
TEST(UtComptimeString, Basics) {
    constexpr ut_comptime_string s("hello");
    static_assert(s.size() == 6); // "hello" + null terminator
    static_assert(s.length() == 5);
    static_assert(!s.empty());

    EXPECT_STREQ(s.c_str(), "hello");
    EXPECT_EQ(s[0], 'h');
    EXPECT_EQ(s[4], 'o');
}

// Test string concatenation
TEST(UtComptimeString, Concatenation) {
    constexpr ut_comptime_string a("hello");
    constexpr ut_comptime_string b("world");
    constexpr auto result = a + " " + b;

    static_assert(result.size() == 12); // "hello world" + null
    static_assert(result.length() == 11);

    EXPECT_STREQ(result.c_str(), "hello world");
}

// Test empty strings
TEST(UtComptimeString, Empty) {
    constexpr ut_comptime_string empty("");
    static_assert(empty.size() == 1); // just null terminator
    static_assert(empty.length() == 0);
    static_assert(empty.empty());

    EXPECT_STREQ(empty.c_str(), "");
}

// Test single character strings
TEST(UtComptimeString, SingleChar) {
    constexpr ut_comptime_string c("x");
    static_assert(c.size() == 2); // 'x' + null terminator
    static_assert(c.length() == 1);
    static_assert(!c.empty());

    EXPECT_STREQ(c.c_str(), "x");
    EXPECT_EQ(c[0], 'x');
}

// Test string conversion to string_view
TEST(UtComptimeString, StringViewConversion) {
    constexpr ut_comptime_string s("test");
    std::string_view sv = s;

    EXPECT_EQ(sv.length(), 4);
    EXPECT_EQ(sv, "test");
    EXPECT_STREQ(sv.data(), "test");
}
