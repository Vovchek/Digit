// This test file builds without precompiled headers. Disable PCH for this file in the
// Tests project (set "Precompiled Header" to "Not Using Precompiled Headers").
#include "stdafx.h"
#include "gtest/gtest.h"
#include <array>
#include <algorithm>
#include <cmath>

extern "C" double DeCasteljau_Query(
	const double* xs,
	const double* ys,
	const double* zs,
	std::size_t n,
	double qx,
	double qy);

extern "C" int DeCasteljau_CheckC0OnMacroEdge(
	const double* xs,
	const double* ys,
	const double* zs,
	std::size_t n,
	std::size_t tri1,
	int edge1,
	std::size_t tri2,
	int edge2,
	int N,
	double* outDifferences);

extern "C" int DeCasteljau_EvalInTriangle(
	const double* xs,
	const double* ys,
	const double* zs,
	std::size_t n,
	std::size_t tri,
	double qx,
	double qy,
	double* outZ);

static double analytic_z(double x, double y)
{
	// Use a smooth cubic polynomial to test C1 continuity
	return 1000.0 * (x*x*x - 3.0*x*y*y + y*y + x);
}

TEST(DeCasteljau, C1ContinuityAcrossSharedEdge)
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
		double zcx = DeCasteljau_Query(xs, ys, zs, n, qx, qy);
		double zx = DeCasteljau_Query(xs, ys, zs, n, qx + h, qy);
		double zy = DeCasteljau_Query(xs, ys, zs, n, qx, qy + h);
		double dx = (zx - zcx) / h;
		double dy = (zy - zcx) / h;
		return std::pair<double,double>(dx, dy);
	};

	auto g_minus = grad_approx(p_minus_x, p_minus_y);
	auto g_plus  = grad_approx(p_plus_x, p_plus_y);

	const double tol = 1.e-6; // tolerance in microns/mm units (relaxed)
	EXPECT_NEAR(g_minus.first, g_plus.first, tol);
	EXPECT_NEAR(g_minus.second, g_plus.second, tol);
}

TEST(DeCasteljau, LinearPrecisionOnAffinePlane)
{
	// Single macro-triangle with exact affine plane z = 10*x + 20*y + 5
	const std::size_t n = 3;
	double xs[n] = {0.0, 1.0, 0.0};
	double ys[n] = {0.0, 0.0, 1.0};
	double zs[n] = {5.0, 15.0, 25.0};

	// Test interpolation at several points; should match affine plane exactly
	for (int iy = 1; iy <= 5; ++iy)
	{
		for (int ix = 1; ix <= 5; ++ix)
		{
			const double qx = ix / 10.0;
			const double qy = iy / 10.0;
			if (qx + qy >= 0.95)
				continue; // keep safely inside triangle

			const double z_interp = DeCasteljau_Query(xs, ys, zs, n, qx, qy);
			const double z_exact = 5.0 + 10.0 * qx + 20.0 * qy;
			EXPECT_NEAR(z_interp, z_exact, 1e-10);
		}
	}
}

TEST(DeCasteljau, EvalInTriangleWithoutSearch)
{
	// Test that evalInTriangle can directly evaluate without triangle search.
	// This is a simpler test that just verifies the extern hook works.
	const std::size_t n = 3;
	double xs[n] = {0.0, 1.0, 0.0};
	double ys[n] = {0.0, 0.0, 1.0};
	double zs[n] = {0.0, 1.0, 1.0};

	// Test a point inside triangle 0 (first and only triangle in a 3-point cloud)
	const double qx = 0.2;
	const double qy = 0.2;

	double z_eval = std::numeric_limits<double>::quiet_NaN();
	int result = DeCasteljau_EvalInTriangle(xs, ys, zs, n, 0, qx, qy, &z_eval);

	// Should succeed and return a finite value
	EXPECT_EQ(result, 1);
	EXPECT_TRUE(std::isfinite(z_eval));

	// Also compare with public query API
	double z_query = DeCasteljau_Query(xs, ys, zs, n, qx, qy);
	EXPECT_NEAR(z_eval, z_query, 1e-10);
}

