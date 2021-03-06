//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see LICENSE.md for more details.
/*
 *  \addtogroup Tests
 *  \brief Ordering unittests.
 */

#include <pycpp/misc/ordering.h>
#include <gtest/gtest.h>

PYCPP_USING_NAMESPACE

// TESTS
// -----


TEST(ordering, not_equal_to)
{
    EXPECT_TRUE(ordering::not_equal_to(1, 3));
    EXPECT_FALSE(ordering::not_equal_to(1, 1));
}


TEST(ordering, less_equal)
{
    EXPECT_TRUE(ordering::less_equal(1, 3));
    EXPECT_TRUE(ordering::less_equal(1, 1));
    EXPECT_FALSE(ordering::less_equal(1, 0));
}


TEST(ordering, greater)
{
    EXPECT_FALSE(ordering::greater(1, 3));
    EXPECT_FALSE(ordering::greater(1, 1));
    EXPECT_TRUE(ordering::greater(1, 0));
}


TEST(ordering, greater_equal)
{
    EXPECT_FALSE(ordering::greater_equal(1, 3));
    EXPECT_TRUE(ordering::greater_equal(1, 1));
    EXPECT_TRUE(ordering::greater_equal(1, 0));
}
