#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(SafeMaxTest, BasicComparison) {
    EXPECT_FLOAT_EQ(nanort::safemax(1.0f, 2.0f), 2.0f);
    EXPECT_FLOAT_EQ(nanort::safemax(2.0f, 1.0f), 2.0f);
}

TEST(SafeMaxTest, NegativeValues) {
    EXPECT_FLOAT_EQ(nanort::safemax(-1.0f, 0.0f), 0.0f);
    EXPECT_FLOAT_EQ(nanort::safemax(-2.0f, -1.0f), -1.0f);
}

TEST(SafeMaxTest, EqualValues) {
    EXPECT_FLOAT_EQ(nanort::safemax(5.0f, 5.0f), 5.0f);
}
