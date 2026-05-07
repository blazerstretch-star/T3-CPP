#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(Utilities, SafeMinMax) {
    EXPECT_FLOAT_EQ(nanort::safemin(1.0f, 2.0f), 1.0f);
    EXPECT_FLOAT_EQ(nanort::safemax(1.0f, 2.0f), 2.0f);
    
    EXPECT_FLOAT_EQ(nanort::safemin(-1.0f, 0.0f), -1.0f);
    EXPECT_FLOAT_EQ(nanort::safemax(-1.0f, 0.0f), 0.0f);
}
