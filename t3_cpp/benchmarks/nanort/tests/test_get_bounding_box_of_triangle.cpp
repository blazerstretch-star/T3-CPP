#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(BoundingBox, GetBoundingBoxOfTriangle) {
    float vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    unsigned int faces[] = {0, 1, 2};
    
    nanort::real3<float> bmin, bmax;
    nanort::GetBoundingBoxOfTriangle(&bmin, &bmax, vertices, faces, 0);
    
    EXPECT_FLOAT_EQ(bmin[0], 0.0f);
    EXPECT_FLOAT_EQ(bmin[1], 0.0f);
    EXPECT_FLOAT_EQ(bmin[2], 0.0f);
    EXPECT_FLOAT_EQ(bmax[0], 1.0f);
    EXPECT_FLOAT_EQ(bmax[1], 1.0f);
    EXPECT_FLOAT_EQ(bmax[2], 0.0f);
}
