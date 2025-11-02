#include <gtest/gtest.h>

#include "ut_dlink.hpp" // IWYU pragma: keep

struct Data {
    struct ut_dlink node;
    int             value;
};

TEST(DLinkTest, BasicOperations) {
    Data d1{.node = {}, .value = 100};
    Data d2{.node = {}, .value = 200};
    Data d3{.node = {}, .value = 300};

    ut_dlink_init(&d1.node);
    ut_dlink_init(&d2.node);
    ut_dlink_init(&d3.node);

    EXPECT_TRUE(ut_dlink_is_detached(&d1.node));
    EXPECT_TRUE(ut_dlink_is_detached(&d2.node));
    EXPECT_TRUE(ut_dlink_is_detached(&d3.node));

    ut_dlink_enqueue(&d1.node, &d2.node);
    EXPECT_FALSE(ut_dlink_is_detached(&d1.node));
    EXPECT_FALSE(ut_dlink_is_detached(&d2.node));
    EXPECT_EQ(d1.node.next, &d2.node);
    EXPECT_EQ(d1.node.prev, &d2.node);
    EXPECT_EQ(d2.node.prev, &d1.node);
    EXPECT_EQ(d2.node.next, &d1.node);

    ut_dlink_enqueue(&d2.node, &d3.node);
    EXPECT_EQ(d3.node.prev, &d2.node);
    EXPECT_EQ(d3.node.next, &d1.node);
    EXPECT_EQ(d2.node.next, &d3.node);
    EXPECT_EQ(d1.node.prev, &d3.node);
}