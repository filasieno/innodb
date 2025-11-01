#include <gtest/gtest.h>
#include "io.hpp"

TEST(Io, DiffPositive) {
    EXPECT_EQ(diff(5, 3), 2);
}

TEST(Io, DiffNegative) {
    EXPECT_EQ(diff(3, 5), -2);
}


