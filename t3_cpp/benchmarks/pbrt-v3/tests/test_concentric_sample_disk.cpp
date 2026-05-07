#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: All samples lie inside or on unit disk
TEST(ConcentricSampleDiskTest, InsideUnitDisk) {
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.1f) {
        for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.1f) {
            Point2f u(u_x, u_y);
            Point2f result = ConcentricSampleDisk(u);
            Float radius = std::sqrt(result.x * result.x + result.y * result.y);
            EXPECT_LE(radius, 1.0f + 1e-5f);
        }
    }
}

// Property 2: Center of square maps to center of disk
TEST(ConcentricSampleDiskTest, CenterMapping) {
    Point2f u(0.5f, 0.5f);
    Point2f result = ConcentricSampleDisk(u);
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
}

// Property 3: Corners map to edge of disk
TEST(ConcentricSampleDiskTest, CornerMapping) {
    Point2f corners[] = {
        Point2f(0.0f, 0.0f), Point2f(1.0f, 0.0f),
        Point2f(0.0f, 1.0f), Point2f(1.0f, 1.0f)
    };
    
    for (const auto& u : corners) {
        Point2f result = ConcentricSampleDisk(u);
        Float radius = std::sqrt(result.x * result.x + result.y * result.y);
        EXPECT_NEAR(radius, 1.0f, 1e-5f);
    }
}

// Property 4: Symmetry - opposite corners map to opposite points
TEST(ConcentricSampleDiskTest, OppositeCorners) {
    Point2f u1(0.0f, 0.0f);
    Point2f u2(1.0f, 1.0f);
    
    Point2f result1 = ConcentricSampleDisk(u1);
    Point2f result2 = ConcentricSampleDisk(u2);
    
    // Should be approximately opposite
    EXPECT_NEAR(result1.x, -result2.x, 1e-5f);
    EXPECT_NEAR(result1.y, -result2.y, 1e-5f);
}

// Property 5: Edge samples have radius = 1
TEST(ConcentricSampleDiskTest, EdgeSamples) {
    Point2f edge_samples[] = {
        Point2f(1.0f, 0.5f), Point2f(0.0f, 0.5f),
        Point2f(0.5f, 1.0f), Point2f(0.5f, 0.0f)
    };
    
    for (const auto& u : edge_samples) {
        Point2f result = ConcentricSampleDisk(u);
        Float radius = std::sqrt(result.x * result.x + result.y * result.y);
        EXPECT_NEAR(radius, 1.0f, 1e-5f);
    }
}

// Property 6: Radial monotonicity - moving away from center increases radius
TEST(ConcentricSampleDiskTest, RadialMonotonicity) {
    Point2f center(0.5f, 0.5f);
    
    for (Float t = 0.0f; t <= 1.0f; t += 0.2f) {
        Point2f u(0.5f + t * 0.5f, 0.5f);
        Point2f result = ConcentricSampleDisk(u);
        Float radius = std::sqrt(result.x * result.x + result.y * result.y);
        
        // Radius should increase with t
        EXPECT_GE(radius, t - 1e-5f);
    }
}

// Property 7: Continuous mapping (nearby inputs map to nearby outputs)
TEST(ConcentricSampleDiskTest, Continuity) {
    Point2f u1(0.5f, 0.6f);
    Point2f u2(0.5f, 0.61f);
    
    Point2f result1 = ConcentricSampleDisk(u1);
    Point2f result2 = ConcentricSampleDisk(u2);
    
    Float distance = std::sqrt((result1.x - result2.x) * (result1.x - result2.x) +
                               (result1.y - result2.y) * (result1.y - result2.y));
    
    // Small input change should produce small output change
    EXPECT_LT(distance, 0.1f);
}

// Property 8: Four-fold symmetry
TEST(ConcentricSampleDiskTest, FourFoldSymmetry) {
    Point2f u(0.7f, 0.5f);
    Point2f result = ConcentricSampleDisk(u);
    
    // Sample should be on positive x-axis side
    EXPECT_GT(result.x, 0.0f);
    
    // Radius should be consistent
    Float radius = std::sqrt(result.x * result.x + result.y * result.y);
    EXPECT_LE(radius, 1.0f + 1e-5f);
}

// Property 9: Coverage - samples should cover full disk
TEST(ConcentricSampleDiskTest, DiskCoverage) {
    bool has_positive_x = false;
    bool has_negative_x = false;
    bool has_positive_y = false;
    bool has_negative_y = false;
    
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.2f) {
        for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.2f) {
            Point2f u(u_x, u_y);
            Point2f result = ConcentricSampleDisk(u);
            
            if (result.x > 0.1f) has_positive_x = true;
            if (result.x < -0.1f) has_negative_x = true;
            if (result.y > 0.1f) has_positive_y = true;
            if (result.y < -0.1f) has_negative_y = true;
        }
    }
    
    EXPECT_TRUE(has_positive_x);
    EXPECT_TRUE(has_negative_x);
    EXPECT_TRUE(has_positive_y);
    EXPECT_TRUE(has_negative_y);
}

// Property 10: Non-degenerate mapping (different inputs produce different outputs)
TEST(ConcentricSampleDiskTest, NonDegenerate) {
    Point2f u1(0.3f, 0.3f);
    Point2f u2(0.7f, 0.7f);
    
    Point2f result1 = ConcentricSampleDisk(u1);
    Point2f result2 = ConcentricSampleDisk(u2);
    
    // Results should be different
    Float distance = std::sqrt((result1.x - result2.x) * (result1.x - result2.x) +
                               (result1.y - result2.y) * (result1.y - result2.y));
    EXPECT_GT(distance, 0.1f);
}
