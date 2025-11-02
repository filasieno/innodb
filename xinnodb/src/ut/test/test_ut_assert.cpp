#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

// Enable test mode before including the header
#define IB_ASSERT_TEST_MODE 
#include "ut_assert.hpp"

// Test helper to capture std::cout output
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

// Test assertion failure with no format string
TEST(UtAssert, AssertFail_NoFormat) {
    AssertOutputCapture capture;

    IB_ASSERT(false);

    std::string output = capture.getOutput();
    std::print("output: {}", output);
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("Assertion 'false' failed") != std::string::npos);
    EXPECT_TRUE(output.find("\033[1;31m") != std::string::npos); // Red color
    EXPECT_TRUE(output.find("\033[0m") != std::string::npos);   // Reset color
}

// Test assertion failure with format string but no arguments
TEST(UtAssert, AssertFail_FormatNoArgs) {
    AssertOutputCapture capture;

    IB_ASSERT(false, "This should fail");

    std::string output = capture.getOutput();
    std::print("output: {}", output);
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("Assertion 'false' failed: This should fail") != std::string::npos);
}

// Test assertion failure with format string and arguments
TEST(UtAssert, AssertFail_FormatWithArgs) {
    AssertOutputCapture capture;

    int value = 42;
    IB_ASSERT(value < 0, "Value {} should be negative, got {}", value, value);

    std::string output = capture.getOutput();
    std::print("output: {}", output);
    EXPECT_FALSE(output.empty());
    // Look for key parts of the message rather than exact match
    EXPECT_TRUE(output.find("Assertion 'value < 0' failed: Value 42 should be negative, got 42") != std::string::npos);
}

// Test IB_ASSERT_NOT_NULL macro
TEST(UtAssert, AssertNotNull_Pass) {
    AssertOutputCapture capture;

    int* ptr = new int(42);
    IB_ASSERT_NOT_NULL(ptr);
    delete ptr;

    EXPECT_TRUE(capture.getOutput().empty());
}

TEST(UtAssert, AssertNotNull_Fail) {
    AssertOutputCapture capture;

    int* ptr = nullptr;
    IB_ASSERT_NOT_NULL(ptr);

    std::string output = capture.getOutput();
    std::print("output: {}", output);
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("Assertion '(ptr) != nullptr' failed") != std::string::npos);
}

TEST(UtAssert, AssertNotNull_FailWithFormat) {
    AssertOutputCapture capture;

    int* ptr = nullptr;
    IB_ASSERT_NOT_NULL(ptr, "Pointer was null in function {}", "test_function");

    std::string output = capture.getOutput();
    std::print("output: {}", output);
    EXPECT_FALSE(output.empty());
    // Look for key parts of the message
    EXPECT_TRUE(output.find("Assertion '(ptr) != nullptr' failed: Pointer was null in function test_function") != std::string::npos);    
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
    AssertOutputCapture capture;
    int a = 1, b = 2;
    IB_ASSERT_EQ(a, b, "a {} b {}", a, b);
    std::string out = capture.getOutput();
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("Assertion '(a) == (b)' failed"), std::string::npos);
    EXPECT_NE(out.find("a 1 b 2"), std::string::npos);
}

TEST(UtAssert, Compare_NEQ_Fail_WithFormat) {
    AssertOutputCapture capture;
    int a = 2, b = 2;
    IB_ASSERT_NEQ(a, b, "expected a != b but both {}", a);
    std::string out = capture.getOutput();
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("Assertion '(a) != (b)' failed"), std::string::npos);
    EXPECT_NE(out.find("expected a != b but both 2"), std::string::npos);
}

TEST(UtAssert, Compare_LT_Fail_WithFormat) {
    AssertOutputCapture capture;
    int a = 3, b = 1;
    IB_ASSERT_LT(a, b, "a={} b={}", a, b);
    std::string out = capture.getOutput();
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("Assertion '(a) < (b)' failed"), std::string::npos);
    EXPECT_NE(out.find("a=3 b=1"), std::string::npos);
}

TEST(UtAssert, Compare_GT_Fail_WithFormat) {
    AssertOutputCapture capture;
    int a = 1, b = 2;
    IB_ASSERT_GT(a, b, "a={} b={}", a, b);
    std::string out = capture.getOutput();
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("Assertion '(a) > (b)' failed"), std::string::npos);
    EXPECT_NE(out.find("a=1 b=2"), std::string::npos);
}

TEST(UtAssert, Compare_NLT_Fail_WithFormat) {
    AssertOutputCapture capture;
    int a = 1, b = 2; // a >= b fails
    IB_ASSERT_NLT(a, b, "a={} b={}", a, b);
    std::string out = capture.getOutput();
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("Assertion '(a) >= (b)' failed"), std::string::npos);
    EXPECT_NE(out.find("a=1 b=2"), std::string::npos);
}

TEST(UtAssert, Compare_NGT_Fail_WithFormat) {
    AssertOutputCapture capture;
    int a = 2, b = 1; // a <= b fails
    IB_ASSERT_NGT(a, b, "a={} b={}", a, b);
    std::string out = capture.getOutput();
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("Assertion '(a) <= (b)' failed"), std::string::npos);
    EXPECT_NE(out.find("a=2 b=1"), std::string::npos);
}

// IB_FAIL now routes to ut_fatal_error and must abort with message
TEST(UtAssert, Fail_NoArgs) {
    ASSERT_DEATH({ IB_FAIL("explicit fail"); }, "Fatal error: explicit fail");
}

TEST(UtAssert, Fail_WithArgs) {
    ASSERT_DEATH({ IB_FAIL("oops {} {}", 1, 2); }, "Fatal error: oops 1 2");
}

// Death tests: ib_unreachable and ut_fatal_error always abort
TEST(UtAssert, Unreachable_Death) {
    ASSERT_DEATH({ ib_unreachable(); }, "Unreachable code reached");
}

TEST(UtAssert, FatalError_Death_NoMsg) {
    ASSERT_DEATH({ IB_FAIL(); }, "Fatal error");
}

TEST(UtAssert, FatalError_Death_WithMsg) {
    ASSERT_DEATH({ IB_FAIL("Oops {} {}", 1, 2); }, "Fatal error: Oops 1 2");
}

// Test that disabled assertions do nothing
#ifndef IB_ENABLE_ASSERT
TEST(UtAssert, AssertDisabled_NoOutput) {
    AssertOutputCapture capture;

    // When assertions are disabled, these should be no-ops
    IB_ASSERT(false);
    IB_ASSERT(false, "This should not appear");
    IB_ASSERT_NOT_NULL(nullptr);

    EXPECT_TRUE(capture.getOutput().empty());
}
#endif
