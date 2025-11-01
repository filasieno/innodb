#include <gtest/gtest.h>
#include "defs.hpp"
#include "io.hpp"

TEST(IO, SumTwoPositive2) {
    EXPECT_EQ(sum2(2, 3), 5);
}

TEST(IO, SumWithNegative2) {
    EXPECT_EQ(sum(-2, 3), 1);
}


