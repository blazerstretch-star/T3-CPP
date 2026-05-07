#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: Result is always in [0, 1]
TEST(PowerHeuristicTest, BoundedRange) {
    Float result1 = PowerHeuristic(1, 0.5f, 1, 0.5f);
    Float result2 = PowerHeuristic(2, 0.8f, 1, 0.2f);
    Float result3 = PowerHeuristic(1, 0.1f, 3, 0.9f);
    
    EXPECT_GE(result1, 0.0f);
    EXPECT_LE(result1, 1.0f);
    EXPECT_GE(result2, 0.0f);
    EXPECT_LE(result2, 1.0f);
    EXPECT_GE(result3, 0.0f);
    EXPECT_LE(result3, 1.0f);
}

// Property 2: Equal weights produce 0.5
TEST(PowerHeuristicTest, EqualWeights) {
    Float result = PowerHeuristic(1, 0.5f, 1, 0.5f);
    EXPECT_NEAR(result, 0.5f, 1e-6f);
    
    Float result2 = PowerHeuristic(2, 0.3f, 2, 0.3f);
    EXPECT_NEAR(result2, 0.5f, 1e-6f);
}

// Property 3: Complementary weights sum to 1
TEST(PowerHeuristicTest, ComplementarySumToOne) {
    int nf = 2, ng = 3;
    Float fPdf = 0.6f, gPdf = 0.4f;
    
    Float wf = PowerHeuristic(nf, fPdf, ng, gPdf);
    Float wg = PowerHeuristic(ng, gPdf, nf, fPdf);
    
    EXPECT_NEAR(wf + wg, 1.0f, 1e-6f);
}

// Property 4: Dominant strategy gets higher weight
TEST(PowerHeuristicTest, DominantStrategy) {
    Float result = PowerHeuristic(2, 0.8f, 1, 0.2f);
    EXPECT_GT(result, 0.5f);
    
    Float result2 = PowerHeuristic(1, 0.2f, 2, 0.8f);
    EXPECT_LT(result2, 0.5f);
}

// Property 5: Zero PDF in second strategy gives weight 1
TEST(PowerHeuristicTest, ZeroSecondPDF) {
    Float result = PowerHeuristic(1, 0.5f, 1, 0.0f);
    EXPECT_NEAR(result, 1.0f, 1e-6f);
}

// Property 6: Zero PDF in first strategy gives weight 0
TEST(PowerHeuristicTest, ZeroFirstPDF) {
    Float result = PowerHeuristic(1, 0.0f, 1, 0.5f);
    EXPECT_NEAR(result, 0.0f, 1e-6f);
}

// Property 7: Power heuristic is more aggressive than balance heuristic
TEST(PowerHeuristicTest, MoreAggressiveThanBalance) {
    int nf = 3, ng = 1;
    Float fPdf = 0.7f, gPdf = 0.3f;
    
    Float power_weight = PowerHeuristic(nf, fPdf, ng, gPdf);
    Float balance_weight = BalanceHeuristic(nf, fPdf, ng, gPdf);
    
    // Power heuristic should give more weight to dominant strategy
    EXPECT_GT(power_weight, balance_weight);
}

// Property 8: Scaling both PDFs doesn't change result
TEST(PowerHeuristicTest, ScaleInvariance) {
    Float result1 = PowerHeuristic(2, 0.3f, 3, 0.4f);
    Float result2 = PowerHeuristic(2, 0.6f, 3, 0.8f);  // 2x scaled
    
    EXPECT_NEAR(result1, result2, 1e-6f);
}

// Property 9: Known result verification
TEST(PowerHeuristicTest, KnownResult) {
    // nf=1, fPdf=0.6, ng=1, gPdf=0.6
    // f = 1*0.6 = 0.6, g = 1*0.6 = 0.6
    // Result = 0.36 / (0.36 + 0.36) = 0.5
    Float result = PowerHeuristic(1, 0.6f, 1, 0.6f);
    EXPECT_NEAR(result, 0.5f, 1e-6f);
}

// Property 10: Monotonicity with respect to first PDF
TEST(PowerHeuristicTest, MonotonicityFirstPDF) {
    int nf = 1, ng = 1;
    Float gPdf = 0.5f;
    
    Float prev_weight = 0.0f;
    for (Float fPdf = 0.1f; fPdf <= 0.9f; fPdf += 0.1f) {
        Float weight = PowerHeuristic(nf, fPdf, ng, gPdf);
        EXPECT_GE(weight, prev_weight - 1e-6f);
        prev_weight = weight;
    }
}
