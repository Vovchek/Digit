// This test file builds without precompiled headers. Disable PCH for this file in the
// Tests project (set "Precompiled Header" to "Not Using Precompiled Headers").
#include "stdafx.h"
#include "gtest/gtest.h"
#include <array>
#include <algorithm>
#include <cmath>

extern "C" double CloughTocher_Query(
	const double* xs,
	const double* ys,
	const double* zs,
	std::size_t n,
	double qx,
	double qy);

extern "C" int CloughTocher_DebugCoeffsAndBary(
	const double* xs,
	const double* ys,
	const double* zs,
	std::size_t n,
	double qx,
	double qy,
	double* out12,
	double* outL1,
	double* outL2,
	double* outL3);

extern "C" int CloughTocher_DebugSubPatchLocals(
	const double* xs,
	const double* ys,
	const double* zs,
	std::size_t n,
	double qx,
	double qy,
	int* outSubPatch,
	double* outL1,
	double* outL2,
	double* outL3,
	double* outAlpha,
	double* outBeta,
	double* outGamma);

static double analytic_z(double x, double y)
{
	// Use a smooth cubic polynomial to test C1 continuity
	return 1000.0 * (x*x*x - 3.0*x*y*y + y*y + x);
}

TEST(CloughTocher, C1ContinuityAcrossSharedEdge)
{
	// Square points (0,0),(1,0),(1,1),(0,1)
	const std::size_t n = 4;
	double xs[n] = {0.0, 1.0, 1.0, 0.0};
	double ys[n] = {0.0, 0.0, 1.0, 1.0};
	double zs[n];
	for (std::size_t i = 0; i < n; ++i) zs[i] = analytic_z(xs[i], ys[i]);

	// Use edge y = x (diagonal) as shared edge in typical triangulation; pick a point near center
	const double px = 0.45;
	const double py = 0.45;

	// normal to the diagonal (1,-1) normalized
	const double nx = 1.0 / std::sqrt(2.0);
	const double ny = -1.0 / std::sqrt(2.0);

	const double eps = 1e-6; // small offset in normalized XY (mm)

	// sample points slightly on either side of the diagonal
	const double p_minus_x = px - eps * nx;
	const double p_minus_y = py - eps * ny;
	const double p_plus_x  = px + eps * nx;
	const double p_plus_y  = py + eps * ny;

	// finite difference step to approximate gradient
	const double h = 1e-4;

	auto grad_approx = [&](double qx, double qy) {
		double zcx = CloughTocher_Query(xs, ys, zs, n, qx, qy);
		double zx = CloughTocher_Query(xs, ys, zs, n, qx + h, qy);
		double zy = CloughTocher_Query(xs, ys, zs, n, qx, qy + h);
		double dx = (zx - zcx) / h;
		double dy = (zy - zcx) / h;
		return std::pair<double,double>(dx, dy);
	};

	auto g_minus = grad_approx(p_minus_x, p_minus_y);
	auto g_plus  = grad_approx(p_plus_x, p_plus_y);

	// assert that gradient difference across the edge is small (C1 continuity)
	const double tol = 1e-1; // tolerance in microns/mm units due to interpolation error and finite differences
	EXPECT_NEAR(g_minus.first, g_plus.first, tol);
	EXPECT_NEAR(g_minus.second, g_plus.second, tol);
}

TEST(CloughTocher, BuildTriangleCubicCoeffs_AffinePlaneInvariants)
{
	// Single macro-triangle with exact affine plane z = 10*x + 20*y + 5
	const std::size_t n = 3;
	double xs[n] = {0.0, 1.0, 0.0};
	double ys[n] = {0.0, 0.0, 1.0};
	double zs[n] = {5.0, 15.0, 25.0};

	double coeffs[12] = {};
	double l1 = 0.0, l2 = 0.0, l3 = 0.0;
	ASSERT_EQ(1, CloughTocher_DebugCoeffsAndBary(xs, ys, zs, n, 0.2, 0.3, coeffs, &l1, &l2, &l3));

	// Corner controls must be a permutation of the input vertex heights.
	std::array<double, 3> corners = { coeffs[0], coeffs[5], coeffs[8] };
	std::sort(corners.begin(), corners.end());
	EXPECT_NEAR(corners[0], 5.0, 1e-12);
	EXPECT_NEAR(corners[1], 15.0, 1e-12);
	EXPECT_NEAR(corners[2], 25.0, 1e-12);

	// Linear precision along each macro edge: edge control points must lie at 1/3 and 2/3.
	EXPECT_NEAR(coeffs[1], (2.0 * coeffs[0] + coeffs[5]) / 3.0, 1e-12); // AB edge: b210
	EXPECT_NEAR(coeffs[3], (coeffs[0] + 2.0 * coeffs[5]) / 3.0, 1e-12); // AB edge: b120
	EXPECT_NEAR(coeffs[2], (2.0 * coeffs[0] + coeffs[8]) / 3.0, 1e-12); // AC edge: b201
	EXPECT_NEAR(coeffs[4], (coeffs[0] + 2.0 * coeffs[8]) / 3.0, 1e-12); // AC edge: b102
	EXPECT_NEAR(coeffs[6], (2.0 * coeffs[5] + coeffs[8]) / 3.0, 1e-12); // BC edge: b021
	EXPECT_NEAR(coeffs[7], (coeffs[5] + 2.0 * coeffs[8]) / 3.0, 1e-12); // BC edge: b012

	// Barycentric coordinates should be finite and sum to 1.
	EXPECT_TRUE(std::isfinite(l1));
	EXPECT_TRUE(std::isfinite(l2));
	EXPECT_TRUE(std::isfinite(l3));
	EXPECT_NEAR(l1 + l2 + l3, 1.0, 1e-12);
}

TEST(CloughTocher, QueryLocalCoordinates_AreConsistentWithBranchRules)
{
	const std::size_t n = 3;
	double xs[n] = {0.0, 1.0, 0.0};
	double ys[n] = {0.0, 0.0, 1.0};
	double zs[n] = {0.0, 1.0, 1.0};

	for (int iy = 1; iy <= 7; ++iy)
	{
		for (int ix = 1; ix <= 7; ++ix)
		{
			const double qx = ix / 10.0;
			const double qy = iy / 10.0;
			if (qx + qy >= 0.95)
				continue; // keep safely inside triangle

			int sub = 0;
			double l1 = 0.0, l2 = 0.0, l3 = 0.0;
			double a = 0.0, b = 0.0, g = 0.0;
			ASSERT_EQ(1, CloughTocher_DebugSubPatchLocals(xs, ys, zs, n, qx, qy, &sub, &l1, &l2, &l3, &a, &b, &g));

			EXPECT_NEAR(a + b + g, 1.0, 1e-12);
			EXPECT_GE(g, -1e-12);

			if (sub == 1)
			{
				EXPECT_LE(l3, l1 + 1e-12);
				EXPECT_LE(l3, l2 + 1e-12);
				EXPECT_NEAR(g, 3.0 * l3, 1e-12);
				EXPECT_NEAR(a, l1 - l3, 1e-12);
				EXPECT_NEAR(b, l2 - l3, 1e-12);
				// reconstruct macro barycentrics from local coords in sub-1
				EXPECT_NEAR(l1, a + g / 3.0, 1e-12);
				EXPECT_NEAR(l2, b + g / 3.0, 1e-12);
				EXPECT_NEAR(l3, g / 3.0, 1e-12);
			}
			else if (sub == 2)
			{
				EXPECT_LE(l1, l2 + 1e-12);
				EXPECT_LE(l1, l3 + 1e-12);
				EXPECT_NEAR(g, 3.0 * l1, 1e-12);
				EXPECT_NEAR(a, l2 - l1, 1e-12);
				EXPECT_NEAR(b, l3 - l1, 1e-12);
				// reconstruct macro barycentrics from local coords in sub-2
				EXPECT_NEAR(l1, g / 3.0, 1e-12);
				EXPECT_NEAR(l2, a + g / 3.0, 1e-12);
				EXPECT_NEAR(l3, b + g / 3.0, 1e-12);
			}
			else
			{
				EXPECT_EQ(sub, 3);
				EXPECT_LE(l2, l1 + 1e-12);
				EXPECT_LE(l2, l3 + 1e-12);
				EXPECT_NEAR(g, 3.0 * l2, 1e-12);
				EXPECT_NEAR(a, l3 - l2, 1e-12);
				EXPECT_NEAR(b, l1 - l2, 1e-12);
				// reconstruct macro barycentrics from local coords in sub-3
				EXPECT_NEAR(l1, b + g / 3.0, 1e-12);
				EXPECT_NEAR(l2, g / 3.0, 1e-12);
				EXPECT_NEAR(l3, a + g / 3.0, 1e-12);
			}
		}
	}
}
