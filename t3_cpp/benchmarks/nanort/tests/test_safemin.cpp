#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(SafeMinTest, BasicComparison) {
    EXPECT_FLOAT_EQ(nanort::safemin(1.0f, 2.0f), 1.0f);
    EXPECT_FLOAT_EQ(nanort::safemin(2.0f, 1.0f), 1.0f);
}

TEST(SafeMinTest, NegativeValues) {
    EXPECT_FLOAT_EQ(nanort::safemin(-1.0f, 0.0f), -1.0f);
    EXPECT_FLOAT_EQ(nanort::safemin(-2.0f, -1.0f), -2.0f);
}

TEST(SafeMinTest, EqualValues) {
    EXPECT_FLOAT_EQ(nanort::safemin(5.0f, 5.0f), 5.0f);
}
