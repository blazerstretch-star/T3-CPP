#include <gtest/gtest.h>
#include "../src/nanort_functions.h"
#include <cmath>

using namespace nanort;

// ============================================================================
// PROPERTY-BASED TESTS: Verify mathematical invariants
// ============================================================================

// PROPERTY 1: Ordering - when hit, tmin ≤ tmax
TEST(IntersectRayAABBTest, Property_Ordering) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    real3<float> ray_org(0.0f, 0.0f, -5.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    if (hit) {
        EXPECT_LE(tmin, tmax) << "When ray hits, tmin should be ≤ tmax";
    }
}

// PROPERTY 2: Range constraint - when hit, min_t ≤ tmin ≤ max_t
TEST(IntersectRayAABBTest, Property_RangeConstraint) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    real3<float> ray_org(0.0f, 0.0f, -5.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    float min_t = 0.0f;
    float max_t = 1000.0f;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, min_t, max_t,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    if (hit) {
        EXPECT_GE(tmin, min_t - 1e-5f) << "tmin should be ≥ min_t";
        EXPECT_LE(tmin, max_t + 1e-5f) << "tmin should be ≤ max_t";
    }
}

// PROPERTY 3: No hit outside range - if tmax < min_t or tmin > max_t, no hit
TEST(IntersectRayAABBTest, Property_NoHitOutsideRange) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    real3<float> ray_org(0.0f, 0.0f, -5.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    // Ray would hit at t=4 to t=6, but we restrict range to [0, 3]
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 3.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_FALSE(hit) << "Should not hit when intersection is outside [min_t, max_t]";
}

// PROPERTY 4: Ray origin inside box - tmin should be ≤ 0
TEST(IntersectRayAABBTest, Property_OriginInsideBox) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    // Ray origin inside box
    real3<float> ray_org(0.0f, 0.0f, 0.0f);
    real3<float> ray_dir(1.0f, 0.0f, 0.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, -1000.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit) << "Ray starting inside box should hit";
    if (hit) {
        EXPECT_LE(tmin, 0.0f + 1e-5f) << "When origin inside box, tmin should be ≤ 0";
    }
}

// PROPERTY 5: Deterministic - same input produces same output
TEST(IntersectRayAABBTest, Property_Deterministic) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    real3<float> ray_org(0.0f, 0.0f, -5.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin1, tmax1, tmin2, tmax2;
    
    bool hit1 = IntersectRayAABB(&tmin1, &tmax1, 0.0f, 1000.0f,
                                bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    bool hit2 = IntersectRayAABB(&tmin2, &tmax2, 0.0f, 1000.0f,
                                bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_EQ(hit1, hit2) << "Same input should produce same hit result";
    if (hit1 && hit2) {
        EXPECT_FLOAT_EQ(tmin1, tmin2) << "Same input should produce same tmin";
        EXPECT_FLOAT_EQ(tmax1, tmax2) << "Same input should produce same tmax";
    }
}

// ============================================================================
// EDGE CASE TESTS: Boundary conditions
// ============================================================================

// EDGE CASE 1: Ray hits box from outside
TEST(IntersectRayAABBTest, EdgeCase_RayHitsFromOutside) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    real3<float> ray_org(0.0f, 0.0f, -5.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit) << "Ray should hit box";
    EXPECT_GT(tmin, 0.0f) << "Ray from outside should have positive tmin";
    EXPECT_LE(tmin, tmax) << "tmin should be ≤ tmax";
    
    // Ray at z=-5, box from z=-1 to z=1
    // Entry at z=-1: t = (-1 - (-5)) / 1 = 4
    // Exit at z=1: t = (1 - (-5)) / 1 = 6
    EXPECT_NEAR(tmin, 4.0f, 1e-3f) << "Entry point should be at t=4";
}

// EDGE CASE 2: Ray misses box
TEST(IntersectRayAABBTest, EdgeCase_RayMisses) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    // Ray parallel to box, offset in Y
    real3<float> ray_org(0.0f, 5.0f, -5.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_FALSE(hit) << "Ray should miss box";
}

// EDGE CASE 3: Ray origin inside box
TEST(IntersectRayAABBTest, EdgeCase_OriginInsideBox) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    real3<float> ray_org(0.0f, 0.0f, 0.0f);  // Inside box
    real3<float> ray_dir(1.0f, 0.0f, 0.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, -1000.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit) << "Ray starting inside should hit";
    EXPECT_LE(tmin, 0.0f + 1e-5f) << "tmin should be ≤ 0 when origin inside";
    EXPECT_GT(tmax, 0.0f) << "tmax should be positive (exit point)";
}

// EDGE CASE 4: Ray parallel to box face
TEST(IntersectRayAABBTest, EdgeCase_ParallelToFace) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    // Ray parallel to XY plane, at z=0 (inside box in Z)
    real3<float> ray_org(-5.0f, 0.0f, 0.0f);
    real3<float> ray_dir(1.0f, 0.0f, 0.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit) << "Ray parallel to face but intersecting should hit";
}

// EDGE CASE 5: Ray grazing box edge
TEST(IntersectRayAABBTest, EdgeCase_GrazingEdge) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    // Ray grazing the edge at y=1, z=1
    real3<float> ray_org(-5.0f, 1.0f, 1.0f);
    real3<float> ray_dir(1.0f, 0.0f, 0.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    // Grazing is acceptable as hit or miss depending on epsilon handling
    // Just verify no crash and valid output
    if (hit) {
        EXPECT_LE(tmin, tmax);
    }
}

// EDGE CASE 6: Ray with negative direction
TEST(IntersectRayAABBTest, EdgeCase_NegativeDirection) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    // Ray pointing backwards
    real3<float> ray_org(0.0f, 0.0f, 5.0f);
    real3<float> ray_dir(0.0f, 0.0f, -1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 1};  // Negative Z direction
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit) << "Ray with negative direction should still hit";
    EXPECT_GT(tmin, 0.0f) << "Should have positive t value";
}

// EDGE CASE 7: Degenerate box (point)
TEST(IntersectRayAABBTest, EdgeCase_DegenerateBox) {
    float bmin[3] = {0.0f, 0.0f, 0.0f};
    float bmax[3] = {0.0f, 0.0f, 0.0f};  // Point box
    
    real3<float> ray_org(-5.0f, 0.0f, 0.0f);
    real3<float> ray_dir(1.0f, 0.0f, 0.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    // Degenerate box behavior is implementation-defined
    // Just verify no crash
    if (hit) {
        EXPECT_LE(tmin, tmax);
    }
}

// EDGE CASE 8: Ray with zero component in direction
TEST(IntersectRayAABBTest, EdgeCase_ZeroDirectionComponent) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    // Ray with zero Y and Z components (parallel to X axis)
    real3<float> ray_org(-5.0f, 0.0f, 0.0f);
    real3<float> ray_dir(1.0f, 0.0f, 0.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit) << "Ray parallel to axis should hit if aligned";
    
    // Test ray parallel but offset
    real3<float> ray_org2(-5.0f, 2.0f, 0.0f);  // Outside in Y
    bool hit2 = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                                bmin, bmax, ray_org2, ray_inv_dir, ray_dir_sign);
    
    EXPECT_FALSE(hit2) << "Ray parallel but offset should miss";
}

// EDGE CASE 9: Very large box
TEST(IntersectRayAABBTest, EdgeCase_LargeBox) {
    float bmin[3] = {-1e10f, -1e10f, -1e10f};
    float bmax[3] = {1e10f, 1e10f, 1e10f};
    
    real3<float> ray_org(0.0f, 0.0f, -5.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1e15f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit) << "Should hit large box";
    EXPECT_FALSE(std::isnan(tmin)) << "Should not produce NaN";
    EXPECT_FALSE(std::isinf(tmin)) << "Should not produce Inf";
}

// EDGE CASE 10: Very small box
TEST(IntersectRayAABBTest, EdgeCase_SmallBox) {
    float bmin[3] = {-1e-6f, -1e-6f, -1e-6f};
    float bmax[3] = {1e-6f, 1e-6f, 1e-6f};
    
    real3<float> ray_org(0.0f, 0.0f, -1.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 10.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    // Small box may or may not be hit depending on precision
    // Just verify no crash and valid output
    if (hit) {
        EXPECT_LE(tmin, tmax);
        EXPECT_FALSE(std::isnan(tmin));
    }
}

// EDGE CASE 11: Ray at box corner
TEST(IntersectRayAABBTest, EdgeCase_RayAtCorner) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    // Ray starting at corner
    real3<float> ray_org(1.0f, 1.0f, 1.0f);
    real3<float> ray_dir(1.0f, 1.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, -1.0f, 10.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    // Corner case is implementation-defined
    // Just verify no crash
    if (hit) {
        EXPECT_LE(tmin, tmax);
    }
}

// ============================================================================
// KNOWN VALUE TESTS: Hand-calculated results
// ============================================================================

TEST(IntersectRayAABBTest, KnownValue_CenteredRay) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    real3<float> ray_org(0.0f, 0.0f, -5.0f);
    real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    real3<float> ray_inv_dir = vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                               bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit);
    // Entry at z=-1: t = 4, Exit at z=1: t = 6
    EXPECT_NEAR(tmin, 4.0f, 1e-3f);
    // Note: Implementation may use epsilon factor (1.00000024f)
    EXPECT_NEAR(tmax, 6.0f, 1e-2f);
}
