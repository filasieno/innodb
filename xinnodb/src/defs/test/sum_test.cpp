#include <gtest/gtest.h>
#include "defs.hpp"

TEST(Defs, SumTwoPositive) {
    EXPECT_EQ(sum(2, 3), 5);
}

TEST(Defs, SumWithNegative) {
    EXPECT_EQ(sum(-2, 3), 1);
}


