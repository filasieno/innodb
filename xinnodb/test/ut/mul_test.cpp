#include <gtest/gtest.h>
#include "ut.hpp"

TEST(Ut, MulSimple) {
    EXPECT_EQ(mul(2, 3), 6);
}

TEST(Ut, MulWithZero) {
    EXPECT_EQ(mul(0, 7), 0);
}


