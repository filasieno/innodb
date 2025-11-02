#include <gtest/gtest.h>
#include "ut_assert.hpp"

class AssertOutputCapture {
public:
    AssertOutputCapture() {
        old_buf = std::cout.rdbuf();
        std::cout.rdbuf(buffer.rdbuf());
    }

    ~AssertOutputCapture() {
        std::cout.flush();  // Make sure output is flushed before restoring
        std::cout.rdbuf(old_buf);
    }

    std::string getOutput() const {
        return buffer.str();
    }

private:
    std::stringstream buffer;
    std::streambuf* old_buf;
};

// Test that assertions pass when conditions are true (no output)
TEST(UtAssert, AssertPass_NoOutput) {
    AssertOutputCapture capture;

    // These should pass without any output
    IB_ASSERT(true);
    IB_ASSERT(1 + 1 == 2);
    IB_ASSERT(42 > 0);

    EXPECT_TRUE(capture.getOutput().empty());
}


TEST(UtAssert, Fail_NoArgs) {
    EXPECT_DEATH( { IB_FAIL("explicit fail"); } , "Fatal error: explicit fail" );
}

// Test assertion failure with no format string
TEST(UtAssert, AssertFail_NoFormat) {
    auto fn = []() { IB_ASSERT(false); };
    EXPECT_DEATH( fn (), "Assertion 'false' failed" );
}

// Test assertion failure with format string but no arguments
TEST(UtAssert, AssertFail_FormatNoArgs) {
    auto fn = []() { IB_ASSERT(false, "This should fail"); };
    EXPECT_DEATH( fn(), "Assertion 'false' failed: This should fail");
}

// Test assertion failure with format string and arguments
TEST(UtAssert, AssertFail_FormatWithArgs) {
    int value = 42;
    auto fn = [&]() { IB_ASSERT(value < 0, "Value {} should be negative, got {}", value, value); };
    EXPECT_DEATH( fn(), "Assertion 'value < 0' failed: Value 42 should be negative, got 42");
}

// Test IB_ASSERT_NOT_NULL macro
TEST(UtAssert, AssertNotNull_Pass) {
    int* ptr = new int(42);
    auto fn = [&]() { IB_ASSERT_NOT_NULL(ptr); };
    delete ptr;
    fn();
}

TEST(UtAssert, AssertNotNull_Fail) {
    int* ptr = nullptr;
    auto fn = [&]() { IB_ASSERT_NOT_NULL(ptr); };
    EXPECT_DEATH( fn(), "Assertion 'ptr != nullptr' failed");
}

TEST(UtAssert, AssertNotNull_FailWithFormat) {
    int* ptr = nullptr;
    auto fn = [&]() { IB_ASSERT_NOT_NULL(ptr, "Pointer was null in function {}", "test_function"); };
    EXPECT_DEATH( fn(), "Assertion 'ptr != nullptr' failed: Pointer was null in function test_function");
}

// Comparison macro pass cases (no output)
TEST(UtAssert, CompareMacros_Pass_NoOutput) {
    AssertOutputCapture capture;

    int a = 2, b = 2;
    IB_ASSERT_EQ(a, b);
    IB_ASSERT_NEQ(a, b + 1);
    IB_ASSERT_LT(1, 2);
    IB_ASSERT_GT(2, 1);
    IB_ASSERT_NLT(a, b); // >=
    IB_ASSERT_NGT(a, b); // <=

    EXPECT_TRUE(capture.getOutput().empty());
}

// Comparison macro failure cases with format
TEST(UtAssert, Compare_EQ_Fail_WithFormat) {
    int a = 1, b = 2;
    auto fn = [&](){ IB_ASSERT_EQ(a, b, "a {} b {}", a, b); };

    EXPECT_DEATH(fn(), "Assertion 'a == b' failed.*a 1 b 2");
}

TEST(UtAssert, Compare_NEQ_Fail_WithFormat) {
    int a = 2, b = 2;
    auto fn = [&]() { IB_ASSERT_NEQ(a, b, "expected a != b but both {}", a); };
    EXPECT_DEATH(fn(), "Assertion 'a != b' failed.*expected a != b but both 2");
}

TEST(UtAssert, Compare_LT_Fail_WithFormat) {
    int a = 3, b = 1;
    auto fn = [&](){ IB_ASSERT_LT(a, b, "a={} b={}", a, b); };
    EXPECT_DEATH(fn(), "Assertion 'a < b' failed.*a=3 b=1");
}

TEST(UtAssert, Compare_GT_Fail_WithFormat) {
    int a = 1, b = 2;
    auto fn = [&](){ IB_ASSERT_GT(a, b, "a={} b={}", a, b); };
    EXPECT_DEATH(fn(), "Assertion 'a > b' failed.*a=1 b=2");
}

TEST(UtAssert, Compare_NLT_Fail_WithFormat) {
    int a = 1, b = 2; // a >= b fails
    auto fn = [&](){ IB_ASSERT_NLT(a, b, "a={} b={}", a, b); };
    EXPECT_DEATH(fn(), "Assertion '!\\(a <= b\\)' failed.*a=1 b=2");
}

TEST(UtAssert, Compare_NGT_Fail_WithFormat) {
    int a = 2, b = 1; // a <= b fails
    auto fn = [&](){ IB_ASSERT_NGT(a, b, "a={} b={}", a, b); };
    EXPECT_DEATH(fn(), "Assertion '!\\(a >= b\\)' failed.*a=2 b=1");
}

TEST(UtAssert, Fail_WithArgs) {
    auto fn = []() { IB_FAIL("oops {} {}", 1, 2); };
    EXPECT_DEATH( fn(), "Fatal error: oops 1 2");
}

TEST(UtAssert, Unreachable_Death) {
    auto fn = []() { ib_unreachable(); };
    EXPECT_DEATH( fn(), "Unreachable code reached");
}

TEST(UtAssert, FatalError_Death_NoMsg) {
    auto fn = []() { IB_FAIL(); };
    EXPECT_DEATH( fn(), "Fatal error");
}

TEST(UtAssert, FatalError_Death_WithMsg) {
    auto fn = []() { IB_FAIL("Oops {} {}", 1, 2); };
    EXPECT_DEATH( fn(), "Fatal error: Oops 1 2");
}
