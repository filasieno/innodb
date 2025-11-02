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
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("Assertion 'false' failed This should fail") != std::string::npos);
}

// Test assertion failure with format string and arguments
TEST(UtAssert, AssertFail_FormatWithArgs) {
    AssertOutputCapture capture;

    int value = 42;
    IB_ASSERT(value < 0, "Value {} should be negative, got {}", value, value);

    std::string output = capture.getOutput();
    EXPECT_FALSE(output.empty());
    // Look for key parts of the message rather than exact match
    EXPECT_TRUE(output.find("Assertion 'value < 0' failed") != std::string::npos);
    EXPECT_TRUE(output.find("Value 42 should be negative, got 42") != std::string::npos);
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
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("Assertion '(ptr) != nullptr' failed") != std::string::npos);
}

TEST(UtAssert, AssertNotNull_FailWithFormat) {
    AssertOutputCapture capture;

    int* ptr = nullptr;
    IB_ASSERT_NOT_NULL(ptr, "Pointer was null in function {}", "test_function");

    std::string output = capture.getOutput();
    EXPECT_FALSE(output.empty());
    // Look for key parts of the message
    EXPECT_TRUE(output.find("Assertion '(ptr) != nullptr' failed") != std::string::npos);
    EXPECT_TRUE(output.find("Pointer was null in function test_function") != std::string::npos);
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
