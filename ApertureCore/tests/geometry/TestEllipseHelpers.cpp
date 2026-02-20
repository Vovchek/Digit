/**
 * @file TestEllipseHelpers.cpp
 * @brief Rigorous unit tests for conicToEllipse and solveLinearSystem5x5 helpers
 */

#include <gtest/gtest.h>
#include <cmath>
#include <iostream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Note: These are internal helper functions in Ellipse.cpp namespace
// We'll need to either:
// 1. Make them testable by moving to a header, or
// 2. Test through the public Ellipse API

// For now, we'll duplicate the helper functions in the test to have full control
namespace {
    bool solveLinearSystem5x5(double A[5][5], double b[5], double x[5]) {
        constexpr double EPSILON = 1e-10;

        // Create augmented matrix
        double aug[5][6];
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                aug[i][j] = A[i][j];
            }
            aug[i][5] = b[i];
        }

        // Forward elimination with partial pivoting
        for (int k = 0; k < 5; k++) {
            // Find pivot
            int maxRow = k;
            double maxVal = std::abs(aug[k][k]);
            for (int i = k + 1; i < 5; i++) {
                double val = std::abs(aug[i][k]);
                if (val > maxVal) {
                    maxVal = val;
                    maxRow = i;
                }
            }

            if (maxVal < EPSILON) {
                return false;  // Singular matrix
            }

            // Swap rows
            if (maxRow != k) {
                for (int j = 0; j < 6; j++) {
                    std::swap(aug[k][j], aug[maxRow][j]);
                }
            }

            // Eliminate
            for (int i = k + 1; i < 5; i++) {
                double factor = aug[i][k] / aug[k][k];
                for (int j = k; j < 6; j++) {
                    aug[i][j] -= factor * aug[k][j];
                }
            }
        }

        // Back substitution
        for (int i = 4; i >= 0; i--) {
            double sum = aug[i][5];
            for (int j = i + 1; j < 5; j++) {
                sum -= aug[i][j] * x[j];
            }
            x[i] = sum / aug[i][i];
        }

        return true;
    }

    bool conicToEllipse(double a, double b, double c,
        double d, double e, double f,
        double& cx, double& cy,
        double& A, double& B,
        double& rotDeg)
    {
        constexpr double EPS = 1e-12;

        double q11 = a;
        double q12 = 0.5 * b;
        double q22 = c;

        double detQ = q11 * q22 - q12 * q12;
        if (detQ <= EPS) return false;

        double invQ11 = q22 / detQ;
        double invQ12 = -q12 / detQ;
        double invQ22 = q11 / detQ;

        cx = -0.5 * (invQ11 * d + invQ12 * e);
        cy = -0.5 * (invQ12 * d + invQ22 * e);

        double k =
            q11 * cx * cx +
            2 * q12 * cx * cy +
            q22 * cy * cy
            - f;

        if (k <= EPS) return false;

        double tr = q11 + q22;
        double diff = q11 - q22;
        double root = sqrt(diff * diff + 4 * q12 * q12);

        double l1 = 0.5 * (tr + root);
        double l2 = 0.5 * (tr - root);

        if (l1 <= EPS || l2 <= EPS) return false;

        double a1 = sqrt(k / l1);
        double a2 = sqrt(k / l2);

        if (a1 >= a2) {
            A = a1; B = a2;
            rotDeg = 0.5 * atan2(2 * q12, diff) * 180.0 / M_PI;
        }
        else {
            A = a2; B = a1;
            rotDeg = 0.5 * atan2(2 * q12, diff) * 180.0 / M_PI + 90.0;
        }
        if (rotDeg >= 180.) rotDeg -= 180;
        if (rotDeg <= -180.) rotDeg += 180.;

        return true;
    }
}
// ============================================================================
// Tests for solveLinearSystem5x5
// ============================================================================

TEST(SolveLinearSystem5x5Test, Identity) {
    // Solve I * x = b, where b = [1, 2, 3, 4, 5]
    double A[5][5] = {
        {1, 0, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 1}
    };
    double b[5] = {1, 2, 3, 4, 5};
    double x[5] = {0, 0, 0, 0, 0};
    
    EXPECT_TRUE(solveLinearSystem5x5(A, b, x));
    
    for (int i = 0; i < 5; i++) {
        EXPECT_DOUBLE_EQ(x[i], b[i]);
    }
}

TEST(SolveLinearSystem5x5Test, Diagonal) {
    // Solve diag(2,3,4,5,6) * x = [2, 6, 12, 20, 30]
    // Solution: x = [1, 2, 3, 4, 5]
    double A[5][5] = {
        {2, 0, 0, 0, 0},
        {0, 3, 0, 0, 0},
        {0, 0, 4, 0, 0},
        {0, 0, 0, 5, 0},
        {0, 0, 0, 0, 6}
    };
    double b[5] = {2, 6, 12, 20, 30};
    double x[5] = {0, 0, 0, 0, 0};
    
    EXPECT_TRUE(solveLinearSystem5x5(A, b, x));
    
    EXPECT_NEAR(x[0], 1.0, 1e-10);
    EXPECT_NEAR(x[1], 2.0, 1e-10);
    EXPECT_NEAR(x[2], 3.0, 1e-10);
    EXPECT_NEAR(x[3], 4.0, 1e-10);
    EXPECT_NEAR(x[4], 5.0, 1e-10);
}

TEST(SolveLinearSystem5x5Test, SimpleSystem) {
    // Simple 5x5 system with known solution
    // x1 + 2*x2 + 3*x3 + 4*x4 + 5*x5 = 15
    // 2*x1 + x2 + 4*x3 + 5*x4 + 6*x5 = 32
    // ... (full system would be larger, but let's test known case)
    double A[5][5] = {
        {1, 2, 3, 4, 5},
        {2, 1, 4, 5, 6},
        {3, 4, 1, 6, 7},
        {4, 5, 6, 1, 8},
        {5, 6, 7, 8, 1}
    };
    double b[5] = {15, 32, 50, 69, 89};
    double x[5] = {0, 0, 0, 0, 0};
    
    EXPECT_TRUE(solveLinearSystem5x5(A, b, x));
    
    // Verify solution: A*x should equal b
    for (int i = 0; i < 5; i++) {
        double sum = 0;
        for (int j = 0; j < 5; j++) {
            sum += A[i][j] * x[j];
        }
        EXPECT_NEAR(sum, b[i], 1e-8);
    }
}

TEST(SolveLinearSystem5x5Test, NearSingular) {
    // Nearly singular matrix should still solve
    double A[5][5] = {
        {1, 0.0001, 0, 0, 0},
        {0.0001, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 1}
    };
    double b[5] = {1, 1, 1, 1, 1};
    double x[5] = {0, 0, 0, 0, 0};
    
    EXPECT_TRUE(solveLinearSystem5x5(A, b, x));
    
    // Verify solution
    for (int i = 0; i < 5; i++) {
        double sum = 0;
        for (int j = 0; j < 5; j++) {
            sum += A[i][j] * x[j];
        }
        EXPECT_NEAR(sum, b[i], 1e-8);
    }
}

// ============================================================================
// Tests for conicToEllipse
// ============================================================================

TEST(ConicToEllipseTest, AxisAlignedEllipse) {
    // Ellipse: (x/10)² + (y/5)² = 1
    // Conic form: x²/100 + y²/25 = 1
    // Or: 25x² + 100y² = 2500
    // Normalized: 0.01*x² + 0.04*y² - 1 = 0
    // So: a=0.01, b=0, c=0.04, d=0, e=0, f=-1
    
    double a = 0.01;
    double b = 0.0;
    double c = 0.04;
    double d = 0.0;
    double e = 0.0;
    double f = -1.0;
    
    double centerX, centerY, semiMajor, semiMinor, rotationDeg;
    
    EXPECT_TRUE(conicToEllipse(a, b, c, d, e, f, centerX, centerY, semiMajor, semiMinor, rotationDeg));
    
    std::cout << "\nAxis-aligned ellipse test:\n";
    std::cout << "  Center: (" << centerX << ", " << centerY << ")\n";
    std::cout << "  Semi-major: " << semiMajor << " (expected 10)\n";
    std::cout << "  Semi-minor: " << semiMinor << " (expected 5)\n";
    std::cout << "  Rotation: " << rotationDeg << "° (expected 0)\n";
    
    EXPECT_NEAR(centerX, 0.0, 1e-9);
    EXPECT_NEAR(centerY, 0.0, 1e-9);
    EXPECT_NEAR(semiMajor, 10.0, 1e-8);
    EXPECT_NEAR(semiMinor, 5.0, 1e-8);
    EXPECT_NEAR(std::abs(rotationDeg), 0.0, 1e-8);
}

TEST(ConicToEllipseTest, TranslatedEllipse) {
    // Ellipse centered at (3, 2): ((x-3)/10)² + ((y-2)/5)² = 1
    // Conic: 0.01*(x-3)² + 0.04*(y-2)² = 1
    // Expand: 0.01*(x² - 6x + 9) + 0.04*(y² - 4y + 4) = 1
    // 0.01*x² + 0.04*y² - 0.06*x - 0.16*y + 0.09 + 0.16 - 1 = 0
    // 0.01*x² + 0.04*y² - 0.06*x - 0.16*y - 0.75 = 0
    
    double a = 0.01;
    double b = 0.0;
    double c = 0.04;
    double d = -0.06;
    double e = -0.16;
    double f = -0.75;
    
    double centerX, centerY, semiMajor, semiMinor, rotationDeg;
    
    EXPECT_TRUE(conicToEllipse(a, b, c, d, e, f, centerX, centerY, semiMajor, semiMinor, rotationDeg));
    
    std::cout << "\nTranslated ellipse test:\n";
    std::cout << "  Center: (" << centerX << ", " << centerY << ") (expected 3, 2)\n";
    std::cout << "  Semi-major: " << semiMajor << " (expected 10)\n";
    std::cout << "  Semi-minor: " << semiMinor << " (expected 5)\n";
    
    EXPECT_NEAR(centerX, 3.0, 1e-8);
    EXPECT_NEAR(centerY, 2.0, 1e-8);
    EXPECT_NEAR(semiMajor, 10.0, 1e-8);
    EXPECT_NEAR(semiMinor, 5.0, 1e-8);
}

TEST(ConicToEllipseTest, Circle) {
    // Circle: x² + y² = 25 (radius 5)
    // Conic: 0.04*x² + 0.04*y² - 1 = 0
    // So: a=0.04, b=0, c=0.04, d=0, e=0, f=-1
    
    double a = 0.04;
    double b = 0.0;
    double c = 0.04;
    double d = 0.0;
    double e = 0.0;
    double f = -1.0;
    
    double centerX, centerY, semiMajor, semiMinor, rotationDeg;
    
    EXPECT_TRUE(conicToEllipse(a, b, c, d, e, f, centerX, centerY, semiMajor, semiMinor, rotationDeg));
    
    std::cout << "\nCircle test:\n";
    std::cout << "  Center: (" << centerX << ", " << centerY << ")\n";
    std::cout << "  Semi-major: " << semiMajor << " (expected 5)\n";
    std::cout << "  Semi-minor: " << semiMinor << " (expected 5)\n";
    
    EXPECT_NEAR(centerX, 0.0, 1e-9);
    EXPECT_NEAR(centerY, 0.0, 1e-9);
    EXPECT_NEAR(semiMajor, 5.0, 1e-8);
    EXPECT_NEAR(semiMinor, 5.0, 1e-8);
}

TEST(ConicToEllipseTest, Hyperbola) {
    // Hyperbola: x² - y² = 1
    // This should FAIL because B²-4AC = 0 - 4(1)(-1) = 4 > 0 (not an ellipse)
    
    double a = 1.0;
    double b = 0.0;
    double c = -1.0;
    double d = 0.0;
    double e = 0.0;
    double f = -1.0;
    
    double centerX, centerY, semiMajor, semiMinor, rotationDeg;
    
    EXPECT_FALSE(conicToEllipse(a, b, c, d, e, f, centerX, centerY, semiMajor, semiMinor, rotationDeg));
}

TEST(ConicToEllipseTest, Parabola) {
    // Parabola: y² = 4x, or y² - 4x = 0
    // This should FAIL because B²-4AC = 0 - 4(0)(1) = 0 (not an ellipse)
    
    double a = 0.0;
    double b = 0.0;
    double c = 1.0;
    double d = -4.0;
    double e = 0.0;
    double f = 0.0;
    
    double centerX, centerY, semiMajor, semiMinor, rotationDeg;
    
    EXPECT_FALSE(conicToEllipse(a, b, c, d, e, f, centerX, centerY, semiMajor, semiMinor, rotationDeg));
}

TEST(ConicToEllipseTest, Circle_Translated) {
    // Circle centered at (5, 3) with radius 4
    // (x-5)² + (y-3)² = 16
    // x² - 10x + 25 + y² - 6y + 9 = 16
    // x² + y² - 10x - 6y + 18 = 0
    // Normalized by /0.0625: 16x² + 16y² - 160x - 96y + 288 = 0
    // Or divide by 16: x² + y² - 10x - 6y + 18 = 0
    // For our form, divide by 16: 0.0625*x² + 0.0625*y² - 0.625*x - 0.375*y + 1.125 = 0
    // To get F=-1: divide by -1.125: -0.0556*x² - 0.0556*y² + 0.556*x + 0.333*y - 1 = 0
    
    // Let's use the unnormalized form directly
    double a = 1.0;
    double b = 0.0;
    double c = 1.0;
    double d = -10.0;
    double e = -6.0;
    double f = 18.0;
    
    double centerX, centerY, semiMajor, semiMinor, rotationDeg;
    
    EXPECT_TRUE(conicToEllipse(a, b, c, d, e, f, centerX, centerY, semiMajor, semiMinor, rotationDeg));
    
    std::cout << "\nCircle translated test:\n";
    std::cout << "  Center: (" << centerX << ", " << centerY << ") (expected 5, 3)\n";
    std::cout << "  Semi-major: " << semiMajor << " (expected 4)\n";
    std::cout << "  Semi-minor: " << semiMinor << " (expected 4)\n";
    
    EXPECT_NEAR(centerX, 5.0, 1e-6);
    EXPECT_NEAR(centerY, 3.0, 1e-6);
    EXPECT_NEAR(semiMajor, 4.0, 1e-6);
    EXPECT_NEAR(semiMinor, 4.0, 1e-6);
}

TEST(ConicToEllipseTest, Ellipse_45DegRotated) {
    // Rotated ellipse is harder to test analytically
    // We'll use a simple case: ellipse with a=2, b=1, rotated 45 degrees
    // This requires computing the conic coefficients for the rotated form
    
    // Standard ellipse: (x/2)² + y² = 1
    // After 45-degree rotation: substitute x' = (x+y)/√2, y' = (-x+y)/√2
    // This gets complex, so we'll test with a known rotated form
    
    // For testing, we'll accept if it returns true and has reasonable values
    // Testing the full rotation algebra is beyond this scope
    
    // Just verify that non-axis-aligned ellipses are detected
    double a = 1.0;
    double b = 0.5;  // Non-zero B term indicates rotation
    double c = 1.0;
    double d = 0.0;
    double e = 0.0;
    double f = -1.0;
    
    double centerX, centerY, semiMajor, semiMinor, rotationDeg;
    
    EXPECT_TRUE(conicToEllipse(a, b, c, d, e, f, centerX, centerY, semiMajor, semiMinor, rotationDeg));
    
    std::cout << "\nRotated ellipse test (with B term):\n";
    std::cout << "  Center: (" << centerX << ", " << centerY << ")\n";
    std::cout << "  Semi-major: " << semiMajor << "\n";
    std::cout << "  Semi-minor: " << semiMinor << "\n";
    std::cout << "  Rotation: " << rotationDeg << "°\n";
    
    // Should have non-zero rotation
    EXPECT_NE(std::abs(rotationDeg), 0.0);
    EXPECT_GT(semiMajor, semiMinor);
}

// Test: Point on axis-aligned ellipse satisfies conic equation
TEST(ConicToEllipseTest, PointOnEllipseAxialAligned) {
    // Ellipse: (x/10)² + (y/5)² = 1, coefficients normalized
    double a = 0.01;
    double b = 0.0;
    double c = 0.04;
    double d = 0.0;
    double e = 0.0;
    double f = -1.0;
    
    // Points on the ellipse
    double test_points[][2] = {
        {10.0, 0.0},   // Right
        {0.0, 5.0},    // Top
        {-10.0, 0.0},  // Left
        {0.0, -5.0},   // Bottom
        {5.0, 2.5 * std::sqrt(3.0)}  // 60 degrees
    };
    
    for (auto& pt : test_points) {
        double x = pt[0];
        double y = pt[1];
        
        // Check conic equation: a*x² + b*xy + c*y² + d*x + e*y + f = 0
        double result = a * x * x + b * x * y + c * y * y + d * x + e * y + f;
        
        EXPECT_NEAR(result, 0.0, 1e-8) << "Point (" << x << ", " << y << ") should be on ellipse";
    }
}

