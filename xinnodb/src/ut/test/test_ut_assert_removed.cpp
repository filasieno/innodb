#include <gtest/gtest.h>

#define IB_ENABLE_ASSERT false
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

TEST(UtAssertDisabled, AssertDisabled_NoOutput) {
    AssertOutputCapture capture;

    // When assertions are disabled, these should be no-ops
    IB_ASSERT(false);
    IB_ASSERT(false, "This should not appear");

    IB_ASSERT_NOT_NULL(nullptr);
    IB_ASSERT_NOT_NULL(nullptr, "This should not appear");

    IB_ASSERT_EQ(2,1);
    IB_ASSERT_EQ(2,1, "This should not appear");

    IB_ASSERT_NEQ(2, 2);
    IB_ASSERT_NEQ(2, 2, "This should not appear");

    IB_ASSERT_GT(1,2)
    IB_ASSERT_GT(1,2, "This should not appear");

    IB_ASSERT_LT(2,1)
    IB_ASSERT_LT(2,1, "This should not appear");

    IB_ASSERT_NGT(2,1)
    IB_ASSERT_NGT(2,1, "This should not appear");

    IB_ASSERT_NLT(1,2)    
    IB_ASSERT_NLT(1,2, "This should not appear");

    EXPECT_TRUE(capture.getOutput().empty());
}


