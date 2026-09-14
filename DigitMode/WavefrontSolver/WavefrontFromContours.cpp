#include "DigitMode/WavefrontSolver/WavefrontFromContours.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <ctime>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <unordered_set>
#include <deque>
#include "./delaunator-cpp/delaunator-header-only.hpp"

namespace
{
	constexpr double kCircleEps = 1e-9;

	double sqr(double v)
	{
		return v * v;
	}

// Clough-Tocher C1 cubic interpolator based on Delaunay triangulation
class CloughTocherInterpolator
{
	public:
		// Accepts samples in original units: X (mm), Y (mm), Z (microns)
		CloughTocherInterpolator(std::vector<XyzSample> samples)
			: points_orig_(std::move(samples))
		{
			normalizePoints();
			buildTriangulation();
			computeTriangleGradients();
			computeVertexGradients();
			coeffs_cache_.resize(triangulation_ ? triangulation_->triangles.size() / 3u : 0u);
		}

		bool debugGetCoeffsAndBary(double x, double y, std::array<double, 12>& coeffs, double& l1, double& l2, double& l3) const
		{
			if (points_norm_.empty() || !triangulation_)
				return false;

			const double nx = normalizeX(x);
			const double ny = normalizeY(y);
			std::size_t tri = locateContainingTriangle(nx, ny);
			if (tri == delaunator::INVALID_INDEX)
				return false;

			auto& opt = coeffs_cache_[tri];
			if (!opt.has_value())
			{
				std::vector<double> local;
				if (!buildTriangleCubicCoeffs(tri, local))
					return false;
				opt = std::move(local);
			}
			const auto& data = *opt;
			for (std::size_t i = 0; i < 12u; ++i)
				coeffs[i] = data[i];

			const auto& tris = triangulation_->triangles;
			const std::size_t ia = tris[3 * tri];
			const std::size_t ib = tris[3 * tri + 1];
			const std::size_t ic = tris[3 * tri + 2];
			const auto& A = points_norm_[ia];
			const auto& B = points_norm_[ib];
			const auto& C = points_norm_[ic];
			return barycentricCoords(A.x, A.y, B.x, B.y, C.x, C.y, nx, ny, l1, l2, l3);
		}

		bool debugGetSubPatchLocalCoords(double x, double y, int& subPatch, double& l1, double& l2, double& l3, double& alpha, double& beta, double& gamma) const
		{
			std::array<double, 12> coeffs{};
			if (!debugGetCoeffsAndBary(x, y, coeffs, l1, l2, l3))
				return false;

			if (l3 <= l1 && l3 <= l2)
			{
				subPatch = 1;
				gamma = 3.0 * l3;
				alpha = l1 - l3;
				beta = l2 - l3;
			}
			else if (l1 <= l2 && l1 <= l3)
			{
				subPatch = 2;
				gamma = 3.0 * l1;
				alpha = l2 - l1;
				beta = l3 - l1;
			}
			else
			{
				subPatch = 3;
				gamma = 3.0 * l2;
				alpha = l3 - l2;
				beta = l1 - l2;
			}

			return true;
		}

		// Query in original units (X mm, Y mm). Returns interpolated Z.
		double query(double x, double y) const
		{
			if (points_norm_.empty() || !triangulation_)
				return std::numeric_limits<double>::quiet_NaN();

			const double nx = normalizeX(x);
			const double ny = normalizeY(y);

			// Locate containing triangle
			std::size_t tri = locateContainingTriangle(nx, ny);
			if (tri == delaunator::INVALID_INDEX) {
				return std::numeric_limits<double>::quiet_NaN();
			}

			// Compute or fetch cached cubic coefficients for this triangle
			auto& opt = coeffs_cache_[tri];
			if (!opt.has_value()) {
				std::vector<double> coeffs;
				if (!buildTriangleCubicCoeffs(tri, coeffs)) {
					return interpolatePlane(tri, nx, ny);
				}
				opt = std::move(coeffs);
			}

			const std::vector<double>& data = *opt; // 10 values

			// --- Barycentric coordinates of (nx, ny) in macro-triangle ---
			double l1, l2, l3;
			const auto& tris = triangulation_->triangles;
			const std::size_t ia = tris[3 * tri];
			const std::size_t ib = tris[3 * tri + 1];
			const std::size_t ic = tris[3 * tri + 2];
			const auto& A = points_norm_[ia];
			const auto& B = points_norm_[ib];
			const auto& C = points_norm_[ic];
			if (!barycentricCoords(A.x, A.y, B.x, B.y, C.x, C.y, nx, ny, l1, l2, l3)) {
				return interpolatePlane(tri, nx, ny);
			}

			// --- Macro control points ---
			const double b300 = data[0];
			const double b210 = data[1];
			const double b201 = data[2];
			const double b120 = data[3];
			const double b102 = data[4];
			const double b030 = data[5];
			const double b021 = data[6];
			const double b012 = data[7];
			const double b003 = data[8];
			const double b111 = data[9];

			// ============================================================
			// de Casteljau split at centroid (u,v,w) = (1/3, 1/3, 1/3)
			// ============================================================
			//
			// Level 1: degree 3 -> 2  (6 points)
			const double L1_200 = (b300 + b210 + b201) / 3.0; // near V1
			const double L1_020 = (b030 + b120 + b021) / 3.0; // near V2
			const double L1_002 = (b003 + b102 + b012) / 3.0; // near V3
			const double L1_110 = (b210 + b120 + b111) / 3.0; // edge V1-V2 side
			const double L1_101 = (b201 + b102 + b111) / 3.0; // edge V1-V3 side
			const double L1_011 = (b021 + b012 + b111) / 3.0; // edge V2-V3 side

			// Level 2: degree 2 -> 1  (3 points)
			const double L2_100 = (L1_200 + L1_110 + L1_101) / 3.0; // toward V1
			const double L2_010 = (L1_110 + L1_020 + L1_011) / 3.0; // toward V2
			const double L2_001 = (L1_101 + L1_011 + L1_002) / 3.0; // toward V3

			// Level 3: degree 1 -> 0  (1 point = centroid value of surface)
			const double G = (L2_100 + L2_010 + L2_001) / 3.0;

			// ============================================================
			// Select sub-patch and compute local barycentric coordinates
			// ============================================================
			double alpha, beta, gamma;
			double c300, c030, c003;
			double c210, c120, c201, c021, c102, c012, c111;

			// Shared by all sub-patches
			c003 = G;

			if (l3 <= l1 && l3 <= l2) {
				// Sub-patch S1 = (V1, V2, G)
				gamma = 3.0 * l3;
				alpha = l1 - l3;
				beta = l2 - l3;

				c300 = b300;   // V1
				c030 = b030;   // V2
				c210 = b210;   // edge V1-V2, near V1
				c120 = b120;   // edge V1-V2, near V2
				c201 = L1_200; // edge V1-G, 1/3 from V1
				c102 = L2_100; // edge V1-G, 2/3 from V1
				c021 = L1_020; // edge V2-G, 1/3 from V2
				c012 = L2_010; // edge V2-G, 2/3 from V2
				c111 = L1_110; // interior
			}
			else if (l1 <= l2 && l1 <= l3) {
				// Sub-patch S2 = (V2, V3, G)
				gamma = 3.0 * l1;
				alpha = l2 - l1;
				beta = l3 - l1;

				c300 = b030;   // V2
				c030 = b003;   // V3
				c210 = b021;   // edge V2-V3, near V2
				c120 = b012;   // edge V2-V3, near V3
				c201 = L1_020; // edge V2-G, 1/3 from V2
				c102 = L2_010; // edge V2-G, 2/3 from V2
				c021 = L1_002; // edge V3-G, 1/3 from V3
				c012 = L2_001; // edge V3-G, 2/3 from V3
				c111 = L1_011; // interior
			}
			else {
				// Sub-patch S3 = (V3, V1, G)
				gamma = 3.0 * l2;
				alpha = l3 - l2;
				beta = l1 - l2;

				c300 = b003;   // V3
				c030 = b300;   // V1
				c210 = b102;   // edge V3-V1, near V3
				c120 = b201;   // edge V3-V1, near V1
				c201 = L1_002; // edge V3-G, 1/3 from V3
				c102 = L2_001; // edge V3-G, 2/3 from V3
				c021 = L1_200; // edge V1-G, 1/3 from V1
				c012 = L2_100; // edge V1-G, 2/3 from V1
				c111 = L1_101; // interior
			}

			// ============================================================
			// Unrolled Bernstein-Bézier cubic evaluation (only z)
			// ============================================================
			const double bu = alpha, bv = beta, bw = gamma;
			const double bu2 = bu * bu, bv2 = bv * bv, bw2 = bw * bw;
			const double bu3 = bu2 * bu, bv3 = bv2 * bv, bw3 = bw2 * bw;

			const double interpolated_z_norm =
				bu3 * c300
				+ 3.0 * bu2 * bv * c210
				+ 3.0 * bu * bv2 * c120
				+ bv3 * c030
				+ 3.0 * bu2 * bw * c201
				+ 6.0 * bu * bv * bw * c111
				+ 3.0 * bv2 * bw * c021
				+ 3.0 * bu * bw2 * c102
				+ 3.0 * bv * bw2 * c012
				+ bw3 * c003;

			return denormalizeZ(interpolated_z_norm);
		}

	private:
		std::unique_ptr<delaunator::Delaunator> triangulation_;
		std::vector<XyzSample> points_orig_;
		std::vector<XyzSample> points_norm_;
		// per-triangle gradients (dz/dx,dz/dy) in normalized coords
		std::vector<std::pair<double,double>> tri_gradients_;
		// per-vertex averaged gradients
		std::vector<std::pair<double,double>> vert_gradients_;
		mutable std::vector<std::optional<std::vector<double>>> coeffs_cache_;

		// normalization extents
		double x_min_ = 0.0, x_max_ = 1.0, x_range_ = 1.0;
		double y_min_ = 0.0, y_max_ = 1.0, y_range_ = 1.0;
		double z_min_ = 0.0, z_max_ = 1.0, z_range_ = 1.0;

		void normalizePoints()
		{
			if (points_orig_.empty()) return;
			x_min_ = x_max_ = points_orig_[0].x;
			y_min_ = y_max_ = points_orig_[0].y;
			z_min_ = z_max_ = points_orig_[0].z;
			for (const auto& p : points_orig_)
			{
				if (p.x < x_min_) x_min_ = p.x; if (p.x > x_max_) x_max_ = p.x;
				if (p.y < y_min_) y_min_ = p.y; if (p.y > y_max_) y_max_ = p.y;
				if (p.z < z_min_) z_min_ = p.z; if (p.z > z_max_) z_max_ = p.z;
			}
			x_range_ = (x_max_ - x_min_) > 0.0 ? (x_max_ - x_min_) : 1.0;
			y_range_ = (y_max_ - y_min_) > 0.0 ? (y_max_ - y_min_) : 1.0;
			z_range_ = (z_max_ - z_min_) > 0.0 ? (z_max_ - z_min_) : 1.0;

			points_norm_.clear(); points_norm_.reserve(points_orig_.size());
			for (const auto& p : points_orig_)
			{
				XyzSample np;
				np.x = normalizeX(p.x); /* (p.x - x_min_) / x_range_; */
				np.y = normalizeY(p.y); /* (p.y - y_min_) / y_range_; */
				// keep Z in original units (microns)
				np.z = p.z;
				points_norm_.push_back(np);
			}
		}

		double normalizeX(double x) const noexcept { return x;/*(x - x_min_) / x_range_;*/ }
		double normalizeY(double y) const noexcept { return y;/*(y - y_min_) / y_range_;*/ }
		double denormalizeZ(double z_norm) const noexcept { return z_norm; /* Z stored in original units */ }

		void buildTriangulation()
		{
			if (points_norm_.empty())
			{
				triangulation_.reset();
				return;
			}

			std::vector<double> coords;
			coords.reserve(points_norm_.size() * 2u);
			for (const auto& p : points_norm_)
			{
				coords.push_back(p.x);
				coords.push_back(p.y);
			}

			triangulation_ = std::make_unique<delaunator::Delaunator>(coords);
		}

		void computeTriangleGradients()
		{
			tri_gradients_.clear();
			if (!triangulation_) return;
			const auto& tri = triangulation_->triangles;
			const std::size_t triCount = tri.size() / 3u;
			tri_gradients_.resize(triCount, {0.0, 0.0});

			for (std::size_t t = 0; t < triCount; ++t)
			{
				const std::size_t ia = tri[3*t];
				const std::size_t ib = tri[3*t+1];
				const std::size_t ic = tri[3*t+2];
				const auto& A = points_norm_[ia];
				const auto& B = points_norm_[ib];
				const auto& C = points_norm_[ic];
				// solve [ [Bx-Ax, By-Ay], [Cx-Ax, Cy-Ay] ] * [a;b] = [Bz-Az, Cz-Az]
				const double m00 = B.x - A.x; const double m01 = B.y - A.y;
				const double m10 = C.x - A.x; const double m11 = C.y - A.y;
				const double rhs0 = B.z - A.z; const double rhs1 = C.z - A.z;
				const double det = m00 * m11 - m01 * m10;
				if (std::fabs(det) < 1e-15)
				{
					tri_gradients_[t] = {0.0, 0.0};
				}
				else
				{
					const double a = ( rhs0 * m11 - m01 * rhs1) / det; // dz/dx
					const double b = ( m00 * rhs1 - rhs0 * m10) / det; // dz/dy
					tri_gradients_[t] = {a, b};
				}
			}
		}

		void computeVertexGradients()
		{
			vert_gradients_.clear();
			vert_gradients_.resize(points_norm_.size(), { 0.0, 0.0 });
			if (!triangulation_) return;

			const auto& tri = triangulation_->triangles;
			const std::size_t triCount = tri.size() / 3u;
			std::vector<double> weightSum(points_norm_.size(), 0.0);

			for (std::size_t t = 0; t < triCount; ++t)
			{
				const std::size_t ia = tri[3 * t];
				const std::size_t ib = tri[3 * t + 1];
				const std::size_t ic = tri[3 * t + 2];
				const auto& A = points_norm_[ia];
				const auto& B = points_norm_[ib];
				const auto& C = points_norm_[ic];

				// Calculate area as the stability metric
				const double area = std::abs((B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x)) * 0.5;
				if (area < 1e-9) continue; // Skip degenerate or collapsed sliver triangles

				const auto g = tri_gradients_[t];

				// Apply a highly robust inverse-area weight or standard area weight
				// To give smooth results on irregular meshes, add an epsilon to prevent small triangle domination
				const double w = area;

				vert_gradients_[ia].first += g.first * w; vert_gradients_[ia].second += g.second * w; weightSum[ia] += w;
				vert_gradients_[ib].first += g.first * w; vert_gradients_[ib].second += g.second * w; weightSum[ib] += w;
				vert_gradients_[ic].first += g.first * w; vert_gradients_[ic].second += g.second * w; weightSum[ic] += w;
			}

			for (std::size_t i = 0; i < vert_gradients_.size(); ++i)
			{
				if (weightSum[i] > 1e-12)
				{
					vert_gradients_[i].first /= weightSum[i];
					vert_gradients_[i].second /= weightSum[i];
				}
				else
				{
					vert_gradients_[i] = { 0.0, 0.0 };
				}
			}
		}

		// Locate triangle containing (nx,ny) by walking triangles (start from cached triangle if any)
		mutable std::size_t cachedTriangle_ = delaunator::INVALID_INDEX;
		std::size_t locateContainingTriangle(double nx, double ny) const
		{
			if (!triangulation_) return delaunator::INVALID_INDEX;
			const auto& tri = triangulation_->triangles;
			const auto& half = triangulation_->halfedges;
			const std::size_t triCount = tri.size() / 3u;

			std::size_t t = (cachedTriangle_ != delaunator::INVALID_INDEX) ? cachedTriangle_ : 0u;
			// cap iterations
			for (std::size_t iter = 0; iter < triCount; ++iter)
			{
				const std::size_t ia = tri[3*t];
				const std::size_t ib = tri[3*t+1];
				const std::size_t ic = tri[3*t+2];
				const auto& A = points_norm_[ia];
				const auto& B = points_norm_[ib];
				const auto& C = points_norm_[ic];
				double l1, l2, l3;
				if (barycentricCoords(A.x,A.y,B.x,B.y,C.x,C.y,nx,ny,l1,l2,l3))
				{
					if (l1 >= -1e-12 && l2 >= -1e-12 && l3 >= -1e-12)
					{
						cachedTriangle_ = t; return t;
					}
					// choose adjacent triangle opposite the most negative barycentric
					// vertex 0 -> opposite edge index 1, vertex 1 -> opposite edge index 2, vertex 2 -> opposite edge index 0
					std::size_t smallest = 0;
					if (l2 < l1 && l2 < l3) smallest = 1;
					else if (l3 < l1 && l3 < l2) smallest = 2;
					const std::size_t edgeIndex = (smallest + 1) % 3; // mapping to edge opposite the vertex
					const std::size_t e = 3 * t + edgeIndex;
					const std::size_t opp = half[e];
					if (opp == delaunator::INVALID_INDEX) break;
					t = opp / 3u; continue;
				}
				else
				{
					// degenerate triangle: advance
					t = (t + 1) % triCount;
				}
			}
			return delaunator::INVALID_INDEX;
		}

		// compute barycentric
		static bool barycentricCoords(double x1,double y1,double x2,double y2,double x3,double y3,double x,double y,
									  double& l1,double& l2,double& l3)
		{
			const double det = (y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3);
			if (std::fabs(det) < 1e-18) return false;
			l1 = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / det;
			l2 = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / det;
			l3 = 1.0 - l1 - l2;
			return true;
		}

		// find closest vertex by simple linear scan (could be optimized)
		std::size_t findClosestVertex(double nx, double ny) const
		{
			double best = std::numeric_limits<double>::infinity();
			std::size_t besti = 0u;
			for (std::size_t i = 0; i < points_norm_.size(); ++i)
			{
				const double dx = points_norm_[i].x - nx;
				const double dy = points_norm_[i].y - ny;
				const double d2 = dx*dx + dy*dy;
				if (d2 < best) { best = d2; besti = i; }
			}
			return besti;
		}

		// Build cubic Bézier control values for the macro-triangle.
		// outCoeffs layout (size 10):
		//   [0]=b300 (V1)         [1]=b210 (V1 side of edge V1-V2)
		//   [2]=b201 (V1 side of edge V1-V3)   [3]=b120 (V2 side of edge V1-V2)
		//   [4]=b102 (V3 side of edge V1-V3)   [5]=b030 (V2)
		//   [6]=b021 (V2 side of edge V2-V3)   [7]=b012 (V3 side of edge V2-V3)
		//   [8]=b003 (V3)         [9]=b111 (interior, plane-reproducing choice)
		//
		// NOTE: This is a *macro-patch* for de Casteljau subdivision, NOT Farin's
		//       Clough-Tocher construction. Interior b111 is chosen as the average
		//       of the six edge controls, which guarantees that a planar input is
		//       reproduced exactly (cubic precision on planar data).
		bool buildTriangleCubicCoeffs(std::size_t triIndex, std::vector<double>& outCoeffs) const
		{
			const auto& tri = triangulation_->triangles;
			const std::size_t ia = tri[3 * triIndex];
			const std::size_t ib = tri[3 * triIndex + 1];
			const std::size_t ic = tri[3 * triIndex + 2];
			const auto& A = points_norm_[ia];
			const auto& B = points_norm_[ib];
			const auto& C = points_norm_[ic];

			const double z1 = A.z;
			const double z2 = B.z;
			const double z3 = C.z;

			auto g1 = vert_gradients_[ia];
			auto g2 = vert_gradients_[ib];
			auto g3 = vert_gradients_[ic];

			// --- Corner controls ---
			const double b300 = z1;
			const double b030 = z2;
			const double b003 = z3;

			// --- Edge controls via Hermite tangents (1/3 along each edge) ---
			const double b210 = z1 + (1.0 / 3.0) * (g1.first * (B.x - A.x) + g1.second * (B.y - A.y));
			const double b201 = z1 + (1.0 / 3.0) * (g1.first * (C.x - A.x) + g1.second * (C.y - A.y));

			const double b120 = z2 + (1.0 / 3.0) * (g2.first * (A.x - B.x) + g2.second * (A.y - B.y));
			const double b021 = z2 + (1.0 / 3.0) * (g2.first * (C.x - B.x) + g2.second * (C.y - B.y));

			const double b102 = z3 + (1.0 / 3.0) * (g3.first * (A.x - C.x) + g3.second * (A.y - C.y));
			const double b012 = z3 + (1.0 / 3.0) * (g3.first * (B.x - C.x) + g3.second * (B.y - C.y));

			// --- Interior control: average of edge points (plane-reproducing) ---
			const double b111 = (1.0 / 6.0) * (b210 + b120 + b021 + b012 + b102 + b201);

			outCoeffs.resize(10);
			outCoeffs[0] = b300;
			outCoeffs[1] = b210;
			outCoeffs[2] = b201;
			outCoeffs[3] = b120;
			outCoeffs[4] = b102;
			outCoeffs[5] = b030;
			outCoeffs[6] = b021;
			outCoeffs[7] = b012;
			outCoeffs[8] = b003;
			outCoeffs[9] = b111;

			return true;
		}

		// fallback planar interpolation: compute plane from triangle and evaluate
		double interpolatePlane(std::size_t triIndex, double x, double y) const
		{
			const auto& tri = triangulation_->triangles;
			const std::size_t ia = tri[3*triIndex];
			const std::size_t ib = tri[3*triIndex+1];
			const std::size_t ic = tri[3*triIndex+2];
			const auto& A = points_norm_[ia];
			const auto& B = points_norm_[ib];
			const auto& C = points_norm_[ic];
			const double m00 = B.x - A.x; const double m01 = B.y - A.y;
			const double m10 = C.x - A.x; const double m11 = C.y - A.y;
			const double rhs0 = B.z - A.z; const double rhs1 = C.z - A.z;
			const double det = m00 * m11 - m01 * m10;
			if (std::fabs(det) < 1e-15) return denormalizeZ(A.z);
			const double a = ( rhs0 * m11 - m01 * rhs1) / det; // dz/dx
			const double b = ( m00 * rhs1 - rhs0 * m10) / det; // dz/dy
			const double z = A.z + a * (x - A.x) + b * (y - A.y);
			return denormalizeZ(z);
		}
	};

	// Expose C-style helpers for unit tests.
	extern "C" double CloughTocher_Query(
		const double* xs,
		const double* ys,
		const double* zs,
		std::size_t n,
		double qx,
		double qy)
	{
		if (!xs || !ys || !zs || n == 0) return std::numeric_limits<double>::quiet_NaN();
		std::vector<XyzSample> samples;
		samples.reserve(n);
		for (std::size_t i = 0; i < n; ++i)
			samples.push_back(XyzSample{ xs[i], ys[i], zs[i] });
		CloughTocherInterpolator interp(std::move(samples));
		return interp.query(qx, qy);
	}

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
		double* outL3)
	{
		if (!xs || !ys || !zs || !out12 || !outL1 || !outL2 || !outL3 || n == 0)
			return 0;

		std::vector<XyzSample> samples;
		samples.reserve(n);
		for (std::size_t i = 0; i < n; ++i)
			samples.push_back(XyzSample{ xs[i], ys[i], zs[i] });

		CloughTocherInterpolator interp(std::move(samples));
		std::array<double, 12> coeffs{};
		double l1 = 0.0, l2 = 0.0, l3 = 0.0;
		if (!interp.debugGetCoeffsAndBary(qx, qy, coeffs, l1, l2, l3))
			return 0;

		for (std::size_t i = 0; i < 12u; ++i)
			out12[i] = coeffs[i];
		*outL1 = l1;
		*outL2 = l2;
		*outL3 = l3;
		return 1;
	}

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
		double* outGamma)
	{
		if (!xs || !ys || !zs || !outSubPatch || !outL1 || !outL2 || !outL3 || !outAlpha || !outBeta || !outGamma || n == 0)
			return 0;

		std::vector<XyzSample> samples;
		samples.reserve(n);
		for (std::size_t i = 0; i < n; ++i)
			samples.push_back(XyzSample{ xs[i], ys[i], zs[i] });

		CloughTocherInterpolator interp(std::move(samples));
		int subPatch = 0;
		double l1 = 0.0, l2 = 0.0, l3 = 0.0;
		double alpha = 0.0, beta = 0.0, gamma = 0.0;
		if (!interp.debugGetSubPatchLocalCoords(qx, qy, subPatch, l1, l2, l3, alpha, beta, gamma))
			return 0;

		*outSubPatch = subPatch;
		*outL1 = l1;
		*outL2 = l2;
		*outL3 = l3;
		*outAlpha = alpha;
		*outBeta = beta;
		*outGamma = gamma;
		return 1;
	}

	double distanceSquared(const WavefrontPrimitivePoint& a, const WavefrontPrimitivePoint& b)
	{
		return sqr(a.x - b.x) + sqr(a.y - b.y);
	}

	bool containsPoint(const WavefrontBoundingCircle& circle, const WavefrontPrimitivePoint& p)
	{
		if (!circle.valid)
			return false;
		return distanceSquared(circle.center, p) <= sqr(circle.radius + kCircleEps);
	}

	WavefrontBoundingCircle circleFromOnePoint(const WavefrontPrimitivePoint& p)
	{
		WavefrontBoundingCircle c;
		c.center = p;
		c.radius = 0.0;
		c.valid = true;
		return c;
	}

	WavefrontBoundingCircle circleFromTwoPoints(const WavefrontPrimitivePoint& a, const WavefrontPrimitivePoint& b)
	{
		WavefrontBoundingCircle c;
		c.center = { (a.x + b.x) * 0.5, (a.y + b.y) * 0.5 };
		c.radius = std::sqrt(distanceSquared(a, b)) * 0.5;
		c.valid = true;
		return c;
	}

	WavefrontBoundingCircle circleFromThreePoints(
		const WavefrontPrimitivePoint& a,
		const WavefrontPrimitivePoint& b,
		const WavefrontPrimitivePoint& c)
	{
		double d = 2.0 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
		if (std::abs(d) < kCircleEps)
		{
			WavefrontBoundingCircle ab = circleFromTwoPoints(a, b);
			WavefrontBoundingCircle ac = circleFromTwoPoints(a, c);
			WavefrontBoundingCircle bc = circleFromTwoPoints(b, c);
			WavefrontBoundingCircle best = ab;
			if (ac.radius > best.radius) best = ac;
			if (bc.radius > best.radius) best = bc;
			return best;
		}

		double ax2ay2 = sqr(a.x) + sqr(a.y);
		double bx2by2 = sqr(b.x) + sqr(b.y);
		double cx2cy2 = sqr(c.x) + sqr(c.y);

		WavefrontBoundingCircle circle;
		circle.center.x = (ax2ay2 * (b.y - c.y) + bx2by2 * (c.y - a.y) + cx2cy2 * (a.y - b.y)) / d;
		circle.center.y = (ax2ay2 * (c.x - b.x) + bx2by2 * (a.x - c.x) + cx2cy2 * (b.x - a.x)) / d;
		circle.radius = std::sqrt(distanceSquared(circle.center, a));
		circle.valid = true;
		return circle;
	}

	WavefrontBoundingCircle welzl(
		std::vector<WavefrontPrimitivePoint>& points,
		std::vector<WavefrontPrimitivePoint>& boundary,
		int n)
	{
		if (n == 0 || boundary.size() == 3)
		{
			if (boundary.empty())
				return {};
			if (boundary.size() == 1)
				return circleFromOnePoint(boundary[0]);
			if (boundary.size() == 2)
				return circleFromTwoPoints(boundary[0], boundary[1]);
			return circleFromThreePoints(boundary[0], boundary[1], boundary[2]);
		}

		const WavefrontPrimitivePoint p = points[static_cast<size_t>(n - 1)];
		WavefrontBoundingCircle d = welzl(points, boundary, n - 1);
		if (containsPoint(d, p))
			return d;

		boundary.push_back(p);
		WavefrontBoundingCircle result = welzl(points, boundary, n - 1);
		boundary.pop_back();
		return result;
	}
}

// Undefine conflicting MFC macros so standard library min/max remain usable here.
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace
{
	class BarycentricInterpolator
	{
	public:
		explicit BarycentricInterpolator(std::vector<XyzSample> samples)
			: points_(std::move(samples))
		{
			buildDelaunay();
		}

		double query(double x, double y) const
		{
			if (points_.empty())
				return std::numeric_limits<double>::quiet_NaN();

			if (triangulation_)
			{
				const std::size_t triangleCount = triangulation_->triangles.size() / 3u;
				if (visitedTriangle_ != delaunator::INVALID_INDEX && visitedTriangle_ < triangleCount)
				{
					if (const auto z = interpolate(x, y, visitedTriangle_))
						return *z;

					for (const std::size_t adjacentTriangle : trianglesAdjacentToTriangle(visitedTriangle_))
					{
						if (adjacentTriangle == delaunator::INVALID_INDEX)
							continue;

						if (const auto z = interpolate(x, y, adjacentTriangle))
						{
							visitedTriangle_ = adjacentTriangle;
							return *z;
						}
					}
				}

				for (std::size_t t = 0; t < triangleCount; ++t)
				{
					if (const auto z = interpolate(x, y, t))
					{
						visitedTriangle_ = t;
						return *z;
					}
				}
			}

			std::size_t nearest = 0u;
			double bestDist2 = distanceSquared(points_[0], x, y);
			for (std::size_t i = 1; i < points_.size(); ++i)
			{
				const double d2 = distanceSquared(points_[i], x, y);
				if (d2 < bestDist2)
				{
					bestDist2 = d2;
					nearest = i;
				}
			}

			return points_[nearest].z;
		}

	private:
		static constexpr double kPositionEps = 1e-9;

		std::vector<XyzSample> points_;
		std::unique_ptr<delaunator::Delaunator> triangulation_;
		mutable std::size_t visitedTriangle_ = delaunator::INVALID_INDEX;

		static double distanceSquared(const XyzSample& p, double x, double y) noexcept
		{
			const double dx = p.x - x;
			const double dy = p.y - y;
			return dx * dx + dy * dy;
		}

		std::array<std::size_t, 3u> trianglesAdjacentToTriangle(std::size_t triangleIndex) const noexcept
		{
			std::array<std::size_t, 3u> adjacentTriangles{
				delaunator::INVALID_INDEX,
				delaunator::INVALID_INDEX,
				delaunator::INVALID_INDEX };
			if (!triangulation_)
				return adjacentTriangles;

			const std::size_t edgeOffset = triangleIndex * 3u;
			for (std::size_t edgeIndex = 0; edgeIndex < adjacentTriangles.size(); ++edgeIndex)
			{
				const std::size_t halfedgeIndex = triangulation_->halfedges[edgeOffset + edgeIndex];
				if (halfedgeIndex != delaunator::INVALID_INDEX)
					adjacentTriangles[edgeIndex] = halfedgeIndex / 3u;
			}

			return adjacentTriangles;
		}

		std::optional<double> interpolate(double x, double y, std::size_t triangleIndex) const
		{
			if (!triangulation_)
				return std::nullopt;

			const std::size_t triangleOffset = triangleIndex * 3u;
			if (triangleOffset + 2u >= triangulation_->triangles.size())
				return std::nullopt;

			const std::size_t ia = triangulation_->triangles[triangleOffset];
			const std::size_t ib = triangulation_->triangles[triangleOffset + 1u];
			const std::size_t ic = triangulation_->triangles[triangleOffset + 2u];

			const XyzSample& a = points_[ia];
			const XyzSample& b = points_[ib];
			const XyzSample& c = points_[ic];
			const double denom = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
			if (std::fabs(denom) <= kPositionEps)
				return std::nullopt;

			const double w1 = ((b.y - c.y) * (x - c.x) + (c.x - b.x) * (y - c.y)) / denom;
			const double w2 = ((c.y - a.y) * (x - c.x) + (a.x - c.x) * (y - c.y)) / denom;
			const double w3 = 1.0 - w1 - w2;
			if (w1 < -kPositionEps || w2 < -kPositionEps || w3 < -kPositionEps)
				return std::nullopt;

			return w1 * a.z + w2 * b.z + w3 * c.z;
		}

		void buildDelaunay()
		{
			visitedTriangle_ = delaunator::INVALID_INDEX;
			if (points_.empty())
			{
				triangulation_.reset();
				return;
			}

			std::vector<double> coords;
			coords.reserve(points_.size() * 2u);
			for (const auto& p : points_)
			{
				coords.push_back(p.x);
				coords.push_back(p.y);
			}

			triangulation_ = std::make_unique<delaunator::Delaunator>(coords);
		}

	};
}

// Local weighted quadric interpolator using Delaunay graph neighborhoods.
namespace
{
	class LocalQuadricInterpolator
	{
	public:
		explicit LocalQuadricInterpolator(std::vector<XyzSample> samples, std::size_t K = 16u)
			: points_(std::move(samples)), K_((std::max)(K, 6u)), searchPool_((std::max)(4u * ((std::max)(K, 6u) + 1u), 32u))
		{
			buildTriangulation();
			buildAdjacency();
			lastVertex_ = points_.empty() ? delaunator::INVALID_INDEX : 0u;
			visitedMark_.assign(points_.size(), 0u);
			queue_.reserve(points_.size());
			candidateVertices_.reserve(searchPool_);
			distanceBuffer_.reserve(searchPool_);
		}

		double query(double x, double y) const
		{
			if (points_.empty())
				return std::numeric_limits<double>::quiet_NaN();
			if (points_.size() == 1u)
				return points_[0].z;

			const std::size_t seed = findClosestVertex(x, y);
			if (seed == delaunator::INVALID_INDEX)
				return std::numeric_limits<double>::quiet_NaN();

			gatherNeighborhood(seed, searchPool_);
			if (candidateVertices_.empty())
				return points_[seed].z;

			distanceBuffer_.clear();
			distanceBuffer_.reserve(candidateVertices_.size());
			for (std::size_t vi : candidateVertices_)
			{
				const auto& p = points_[vi];
				const double dx = p.x - x;
				const double dy = p.y - y;
				distanceBuffer_.push_back(std::make_pair(dx * dx + dy * dy, vi));
			}

			const std::size_t selectCount = (std::min)(distanceBuffer_.size(), K_ + 1u);
			if (selectCount == 0u)
				return points_[seed].z;

			std::partial_sort(distanceBuffer_.begin(), distanceBuffer_.begin() + static_cast<std::ptrdiff_t>(selectCount), distanceBuffer_.end(),
				[](const auto& a, const auto& b) { return a.first < b.first; });

			if (distanceBuffer_[0].first <= 1e-24)
				return points_[distanceBuffer_[0].second].z;

			const std::size_t fitCount = (std::min)(K_, selectCount);
			if (fitCount == 0u)
				return points_[distanceBuffer_[0].second].z;

			const std::size_t rIndex = (selectCount > fitCount) ? fitCount : (fitCount - 1u);
			const double R = std::sqrt(distanceBuffer_[rIndex].first);
			if (!(R > 0.0))
				return points_[distanceBuffer_[0].second].z;

			double normal[6][7] = {};
			double weightSum = 0.0;
			double weightedZ = 0.0;
			std::size_t used = 0u;

			for (std::size_t i = 0; i < fitCount; ++i)
			{
				const auto [d2, vi] = distanceBuffer_[i];
				if (d2 <= 1e-24)
					return points_[vi].z;

				const double d = std::sqrt(d2);
				const double t = ((std::max)(0.0, R - d)) / (R * d);
				const double w = t * t * t;
				if (!(w > 0.0))
					continue;

				const double xi = points_[vi].x;
				const double yi = points_[vi].y;
				const double zi = points_[vi].z;
				const double phi[6] = { 1.0, xi, yi, xi * xi, yi * yi, xi * yi };

				for (int r = 0; r < 6; ++r)
				{
					for (int c = 0; c < 6; ++c)
						normal[r][c] += w * phi[r] * phi[c];
					normal[r][6] += w * phi[r] * zi;
				}

				weightSum += w;
				weightedZ += w * zi;
				++used;
			}

			if (used < 6u || !(weightSum > 0.0))
				return points_[distanceBuffer_[0].second].z;

			double coeffs[6] = {};
			if (!solve6x6(normal, coeffs))
				return weightedZ / weightSum;

			return coeffs[0]
				+ coeffs[1] * x
				+ coeffs[2] * y
				+ coeffs[3] * x * x
				+ coeffs[4] * y * y
				+ coeffs[5] * x * y;
		}

	private:
		std::vector<XyzSample> points_;
		std::unique_ptr<delaunator::Delaunator> triangulation_;
		std::vector<std::vector<std::size_t>> adjacency_;
		std::size_t K_ = 16u;
		std::size_t searchPool_ = 64u;
		mutable std::size_t lastVertex_ = delaunator::INVALID_INDEX;
		mutable std::size_t visitToken_ = 1u;
		mutable std::vector<std::size_t> visitedMark_;
		mutable std::vector<std::size_t> queue_;
		mutable std::vector<std::size_t> candidateVertices_;
		mutable std::vector<std::pair<double, std::size_t>> distanceBuffer_;

		void buildTriangulation()
		{
			if (points_.empty())
			{
				triangulation_.reset();
				return;
			}

			std::vector<double> coords;
			coords.reserve(points_.size() * 2u);
			for (const auto& p : points_)
			{
				coords.push_back(p.x);
				coords.push_back(p.y);
			}
			triangulation_ = std::make_unique<delaunator::Delaunator>(coords);
		}

		void buildAdjacency()
		{
			adjacency_.assign(points_.size(), {});
			if (!triangulation_)
				return;

			std::vector<std::unordered_set<std::size_t>> tmp(points_.size());
			const auto& tri = triangulation_->triangles;
			const std::size_t triCount = tri.size() / 3u;
			for (std::size_t t = 0; t < triCount; ++t)
			{
				const std::size_t a = tri[3u * t];
				const std::size_t b = tri[3u * t + 1u];
				const std::size_t c = tri[3u * t + 2u];
				tmp[a].insert(b); tmp[a].insert(c);
				tmp[b].insert(a); tmp[b].insert(c);
				tmp[c].insert(a); tmp[c].insert(b);
			}

			for (std::size_t i = 0; i < tmp.size(); ++i)
			{
				adjacency_[i].reserve(tmp[i].size());
				for (std::size_t v : tmp[i])
					adjacency_[i].push_back(v);
			}
		}

		std::size_t findClosestVertex(double x, double y) const
		{
			if (points_.empty())
				return delaunator::INVALID_INDEX;

			auto dist2 = [&](std::size_t idx) noexcept
			{
				const auto& p = points_[idx];
				const double dx = p.x - x;
				const double dy = p.y - y;
				return dx * dx + dy * dy;
			};

			std::size_t current = (lastVertex_ != delaunator::INVALID_INDEX) ? lastVertex_ : 0u;
			double best = dist2(current);
			bool moved = true;
			while (moved)
			{
				moved = false;
				for (std::size_t nb : adjacency_[current])
				{
					const double d = dist2(nb);
					if (d + 1e-18 < best)
					{
						best = d;
						current = nb;
						moved = true;
					}
				}
			}
			lastVertex_ = current;
			return current;
		}

		void gatherNeighborhood(std::size_t seed, std::size_t limit) const
		{
			candidateVertices_.clear();
			if (seed == delaunator::INVALID_INDEX || points_.empty())
				return;

			if (++visitToken_ == 0u)
			{
				visitToken_ = 1u;
				std::fill(visitedMark_.begin(), visitedMark_.end(), 0u);
			}

			queue_.clear();
			queue_.push_back(seed);
			visitedMark_[seed] = visitToken_;

			std::size_t head = 0u;
			while (head < queue_.size() && candidateVertices_.size() < limit)
			{
				const std::size_t v = queue_[head++];
				candidateVertices_.push_back(v);
				for (std::size_t nb : adjacency_[v])
				{
					if (visitedMark_[nb] == visitToken_)
						continue;
					visitedMark_[nb] = visitToken_;
					queue_.push_back(nb);
				}
			}
		}

		static bool solve6x6(double a[6][7], double x[6])
		{
			for (int col = 0; col < 6; ++col)
			{
				int pivot = col;
				double maxAbs = std::fabs(a[col][col]);
				for (int r = col + 1; r < 6; ++r)
				{
					const double v = std::fabs(a[r][col]);
					if (v > maxAbs)
					{
						maxAbs = v;
						pivot = r;
					}
				}
				if (maxAbs < 1e-14)
					return false;

				if (pivot != col)
				{
					for (int c = col; c <= 6; ++c)
						std::swap(a[col][c], a[pivot][c]);
				}

				const double diag = a[col][col];
				for (int r = col + 1; r < 6; ++r)
				{
					const double factor = a[r][col] / diag;
					if (std::fabs(factor) < 1e-18)
						continue;
					for (int c = col; c <= 6; ++c)
						a[r][c] -= factor * a[col][c];
				}
			}

			for (int r = 5; r >= 0; --r)
			{
				double rhs = a[r][6];
				for (int c = r + 1; c < 6; ++c)
					rhs -= a[r][c] * x[c];
				if (std::fabs(a[r][r]) < 1e-14)
					return false;
				x[r] = rhs / a[r][r];
			}
			return true;
		}
	};
}

// Akima-style bivariate C1 interpolator over Delaunay triangles.
namespace
{
	class AkimaBivariateInterpolator
	{
	public:
		explicit AkimaBivariateInterpolator(std::vector<XyzSample> samples)
			: points_(std::move(samples))
		{
			buildTriangulation();
			buildAdjacencyAndIncidence();
			computeTriangleSlopes();
			computeVertexGradientsAkima();
			buildTrianglePatches();
			cachedTriangle_ = trianglePatches_.empty() ? delaunator::INVALID_INDEX : 0u;
		}

		double query(double x, double y) const
		{
			if (points_.empty() || !triangulation_ || trianglePatches_.empty())
				return std::numeric_limits<double>::quiet_NaN();

			const std::size_t tri = locateContainingTriangle(x, y);
			if (tri == delaunator::INVALID_INDEX)
				return std::numeric_limits<double>::quiet_NaN();

			const auto& p = trianglePatches_[tri];
			double l1 = 0.0, l2 = 0.0, l3 = 0.0;
			if (!barycentricCoords(
				points_[p.ia].x, points_[p.ia].y,
				points_[p.ib].x, points_[p.ib].y,
				points_[p.ic].x, points_[p.ic].y,
				x, y, l1, l2, l3))
			{
				return std::numeric_limits<double>::quiet_NaN();
			}

			const double l11 = l1 * l1;
			const double l22 = l2 * l2;
			const double l33 = l3 * l3;
			const double l111 = l11 * l1;
			const double l222 = l22 * l2;
			const double l333 = l33 * l3;

			return p.b300 * l111
				+ 3.0 * p.b210 * l11 * l2
				+ 3.0 * p.b201 * l11 * l3
				+ 3.0 * p.b120 * l1 * l22
				+ 6.0 * p.b111 * l1 * l2 * l3
				+ 3.0 * p.b102 * l1 * l33
				+ p.b030 * l222
				+ 3.0 * p.b021 * l22 * l3
				+ 3.0 * p.b012 * l2 * l33
				+ p.b003 * l333;
		}

	private:
		struct TriPatch
		{
			std::size_t ia = 0u;
			std::size_t ib = 0u;
			std::size_t ic = 0u;
			double b300 = 0.0;
			double b210 = 0.0;
			double b201 = 0.0;
			double b120 = 0.0;
			double b111 = 0.0;
			double b102 = 0.0;
			double b030 = 0.0;
			double b021 = 0.0;
			double b012 = 0.0;
			double b003 = 0.0;
		};

		std::vector<XyzSample> points_;
		std::unique_ptr<delaunator::Delaunator> triangulation_;
		std::vector<std::vector<std::size_t>> adjacency_;
		std::vector<std::vector<std::size_t>> incidentTriangles_;
		std::vector<std::pair<double, double>> triangleSlopes_;
		std::vector<double> triangleAreas_;
		std::vector<std::pair<double, double>> vertexGradients_;
		std::vector<TriPatch> trianglePatches_;
		mutable std::size_t cachedTriangle_ = delaunator::INVALID_INDEX;

		void buildTriangulation()
		{
			if (points_.empty())
			{
				triangulation_.reset();
				return;
			}

			std::vector<double> coords;
			coords.reserve(points_.size() * 2u);
			for (const auto& p : points_)
			{
				coords.push_back(p.x);
				coords.push_back(p.y);
			}
			triangulation_ = std::make_unique<delaunator::Delaunator>(coords);
		}

		void buildAdjacencyAndIncidence()
		{
			adjacency_.assign(points_.size(), {});
			incidentTriangles_.assign(points_.size(), {});
			if (!triangulation_)
				return;

			std::vector<std::unordered_set<std::size_t>> tmpAdj(points_.size());
			const auto& tri = triangulation_->triangles;
			const std::size_t triCount = tri.size() / 3u;
			for (std::size_t t = 0; t < triCount; ++t)
			{
				const std::size_t a = tri[3u * t];
				const std::size_t b = tri[3u * t + 1u];
				const std::size_t c = tri[3u * t + 2u];

				tmpAdj[a].insert(b); tmpAdj[a].insert(c);
				tmpAdj[b].insert(a); tmpAdj[b].insert(c);
				tmpAdj[c].insert(a); tmpAdj[c].insert(b);

				incidentTriangles_[a].push_back(t);
				incidentTriangles_[b].push_back(t);
				incidentTriangles_[c].push_back(t);
			}

			for (std::size_t i = 0; i < tmpAdj.size(); ++i)
			{
				adjacency_[i].reserve(tmpAdj[i].size());
				for (std::size_t nb : tmpAdj[i])
					adjacency_[i].push_back(nb);
			}
		}

		void computeTriangleSlopes()
		{
			triangleSlopes_.clear();
			triangleAreas_.clear();
			if (!triangulation_)
				return;

			const auto& tri = triangulation_->triangles;
			const std::size_t triCount = tri.size() / 3u;
			triangleSlopes_.resize(triCount, { 0.0, 0.0 });
			triangleAreas_.resize(triCount, 0.0);

			for (std::size_t t = 0; t < triCount; ++t)
			{
				const auto& A = points_[tri[3u * t]];
				const auto& B = points_[tri[3u * t + 1u]];
				const auto& C = points_[tri[3u * t + 2u]];

				const double ux = B.x - A.x;
				const double uy = B.y - A.y;
				const double vx = C.x - A.x;
				const double vy = C.y - A.y;
				const double det = ux * vy - uy * vx;
				triangleAreas_[t] = std::abs(det) * 0.5;

				if (std::fabs(det) < 1e-18)
				{
					triangleSlopes_[t] = { 0.0, 0.0 };
					continue;
				}

				const double rhs0 = B.z - A.z;
				const double rhs1 = C.z - A.z;
				const double dzdx = (rhs0 * vy - uy * rhs1) / det;
				const double dzdy = (ux * rhs1 - rhs0 * vx) / det;
				triangleSlopes_[t] = { dzdx, dzdy };
			}
		}

		void computeVertexGradientsAkima()
		{
			vertexGradients_.assign(points_.size(), { 0.0, 0.0 });
			if (!triangulation_)
				return;

			for (std::size_t v = 0; v < points_.size(); ++v)
			{
				const auto& inc = incidentTriangles_[v];
				if (inc.empty())
					continue;
				if (inc.size() == 1u)
				{
					vertexGradients_[v] = triangleSlopes_[inc[0]];
					continue;
				}

				double wx = 0.0;
				double wy = 0.0;
				double wsum = 0.0;
				for (std::size_t i = 0; i < inc.size(); ++i)
				{
					const std::size_t ti = inc[i];
					const auto si = triangleSlopes_[ti];

					double rough = 0.0;
					for (std::size_t j = 0; j < inc.size(); ++j)
					{
						if (i == j) continue;
						const auto sj = triangleSlopes_[inc[j]];
						const double dx = si.first - sj.first;
						const double dy = si.second - sj.second;
						rough += std::sqrt(dx * dx + dy * dy);
					}
					rough /= static_cast<double>((std::max)(std::size_t{ 1u }, inc.size() - 1u));

					const auto& tri = triangulation_->triangles;
					const auto& A = points_[tri[3u * ti]];
					const auto& B = points_[tri[3u * ti + 1u]];
					const auto& C = points_[tri[3u * ti + 2u]];
					const double cx = (A.x + B.x + C.x) / 3.0;
					const double cy = (A.y + B.y + C.y) / 3.0;
					const double ddx = cx - points_[v].x;
					const double ddy = cy - points_[v].y;
					const double d = std::sqrt(ddx * ddx + ddy * ddy);

					const double area = (std::max)(triangleAreas_[ti], 1e-18);
					const double w = area / (1e-12 + rough + 0.25 * d);
					wx += w * si.first;
					wy += w * si.second;
					wsum += w;
				}

				if (wsum > 0.0)
					vertexGradients_[v] = { wx / wsum, wy / wsum };
				else
				{
					double sx = 0.0, sy = 0.0;
					for (std::size_t ti : inc)
					{
						sx += triangleSlopes_[ti].first;
						sy += triangleSlopes_[ti].second;
					}
					const double inv = 1.0 / static_cast<double>(inc.size());
					vertexGradients_[v] = { sx * inv, sy * inv };
				}
			}
		}

		void buildTrianglePatches()
		{
			trianglePatches_.clear();
			if (!triangulation_)
				return;

			const auto& tri = triangulation_->triangles;
			const std::size_t triCount = tri.size() / 3u;
			trianglePatches_.resize(triCount);

			for (std::size_t t = 0; t < triCount; ++t)
			{
				const std::size_t ia = tri[3u * t];
				const std::size_t ib = tri[3u * t + 1u];
				const std::size_t ic = tri[3u * t + 2u];
				const auto& A = points_[ia];
				const auto& B = points_[ib];
				const auto& C = points_[ic];
				const auto gA = vertexGradients_[ia];
				const auto gB = vertexGradients_[ib];
				const auto gC = vertexGradients_[ic];

				const double b300 = A.z;
				const double b030 = B.z;
				const double b003 = C.z;
				const double b210 = A.z + (1.0 / 3.0) * (gA.first * (B.x - A.x) + gA.second * (B.y - A.y));
				const double b201 = A.z + (1.0 / 3.0) * (gA.first * (C.x - A.x) + gA.second * (C.y - A.y));
				const double b120 = B.z + (1.0 / 3.0) * (gB.first * (A.x - B.x) + gB.second * (A.y - B.y));
				const double b021 = B.z + (1.0 / 3.0) * (gB.first * (C.x - B.x) + gB.second * (C.y - B.y));
				const double b102 = C.z + (1.0 / 3.0) * (gC.first * (A.x - C.x) + gC.second * (A.y - C.y));
				const double b012 = C.z + (1.0 / 3.0) * (gC.first * (B.x - C.x) + gC.second * (B.y - C.y));

				const double r12 = 0.25 * (b210 + b120) + (1.0 / 6.0) * (b201 + b021) - (1.0 / 12.0) * (b300 + b030);
				const double r23 = 0.25 * (b021 + b012) + (1.0 / 6.0) * (b120 + b102) - (1.0 / 12.0) * (b030 + b003);
				const double r31 = 0.25 * (b102 + b201) + (1.0 / 6.0) * (b012 + b210) - (1.0 / 12.0) * (b003 + b300);
				const double b111 = (r12 + r23 + r31) / 3.0;

				trianglePatches_[t] = TriPatch{ ia, ib, ic, b300, b210, b201, b120, b111, b102, b030, b021, b012, b003 };
			}
		}

		std::size_t locateContainingTriangle(double x, double y) const
		{
			if (!triangulation_)
				return delaunator::INVALID_INDEX;

			const auto& tri = triangulation_->triangles;
			const auto& half = triangulation_->halfedges;
			const std::size_t triCount = tri.size() / 3u;
			if (triCount == 0u)
				return delaunator::INVALID_INDEX;

			std::size_t t = (cachedTriangle_ != delaunator::INVALID_INDEX && cachedTriangle_ < triCount) ? cachedTriangle_ : 0u;
			for (std::size_t iter = 0; iter < triCount; ++iter)
			{
				const std::size_t ia = tri[3u * t];
				const std::size_t ib = tri[3u * t + 1u];
				const std::size_t ic = tri[3u * t + 2u];
				double l1 = 0.0, l2 = 0.0, l3 = 0.0;
				if (!barycentricCoords(points_[ia].x, points_[ia].y, points_[ib].x, points_[ib].y, points_[ic].x, points_[ic].y, x, y, l1, l2, l3))
				{
					t = (t + 1u) % triCount;
					continue;
				}

				if (l1 >= -1e-12 && l2 >= -1e-12 && l3 >= -1e-12)
				{
					cachedTriangle_ = t;
					return t;
				}

				std::size_t smallest = 0u;
				if (l2 < l1 && l2 < l3) smallest = 1u;
				else if (l3 < l1 && l3 < l2) smallest = 2u;
				const std::size_t edgeIndex = (smallest + 1u) % 3u;
				const std::size_t he = half[3u * t + edgeIndex];
				if (he == delaunator::INVALID_INDEX)
					break;
				t = he / 3u;
			}

			return delaunator::INVALID_INDEX;
		}

		static bool barycentricCoords(
			double x1, double y1,
			double x2, double y2,
			double x3, double y3,
			double x, double y,
			double& l1, double& l2, double& l3)
		{
			const double det = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
			if (std::fabs(det) < 1e-18)
				return false;
			l1 = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / det;
			l2 = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / det;
			l3 = 1.0 - l1 - l2;
			return true;
		}
	};
}

// Inverse Distance Weighting interpolator using Delaunay connectivity to gather neighbors
namespace
{
	class IDWInterpolator
	{
	public:
		// samples: X (mm), Y (mm), Z (microns)
		explicit IDWInterpolator(std::vector<XyzSample> samples, std::size_t K = 12u)
			: points_orig_(std::move(samples)), K_(K)
		{
			normalizePoints();
			buildTriangulation();
			buildAdjacency();
			lastVertex_ = points_norm_.empty() ? delaunator::INVALID_INDEX : 0u;
		}

		// Query in original units (X mm, Y mm). Returns interpolated Z in microns.
		double query(double x, double y) const
		{
			if (points_norm_.empty())
				return std::numeric_limits<double>::quiet_NaN();

			const double nx = normalizeX(x);
			const double ny = normalizeY(y);

			// Find closest vertex via graph walking (greedy descent)
			std::size_t seed = findClosestVertex(nx, ny);

			// BFS gather K neighbors starting from seed
			const std::vector<std::size_t> neighbors = gatherKNeighbors(seed, K_);
			if (neighbors.empty())
				return std::numeric_limits<double>::quiet_NaN();

			// Compute distances in normalized space and check for exact hit
			std::vector<double> distances;
			distances.reserve(neighbors.size());
			double maxD = 0.0;
			for (std::size_t vi : neighbors)
			{
				const auto& p = points_norm_[vi];
				const double dx = p.x - nx;
				const double dy = p.y - ny;
				const double d = std::sqrt(dx * dx + dy * dy);
				if (d < 1e-9)
				{
					// exact hit: return denormalized Z
					return denormalizeZ(p.z);
				}
				distances.push_back(d);
				if (d > maxD) maxD = d;
			}

			if (maxD <= 0.0)
			{
				// all at same position - average Z
				double sumZ = 0.0;
				for (std::size_t vi : neighbors)
					sumZ += denormalizeZ(points_norm_[vi].z);
				return sumZ / static_cast<double>(neighbors.size());
			}

			// compute weights and weighted average in normalized Z space
			double weightSum = 0.0;
			double weightedZ = 0.0;
			for (std::size_t i = 0; i < neighbors.size(); ++i)
			{
				double d = distances[i];
				// w_i = ((R - d_i) / (R * d_i))^2
				double w = ( (maxD - d) / (maxD * d) );
				w = w * w;
				const double z_norm = points_norm_[neighbors[i]].z;
				weightedZ += w * z_norm;
				weightSum += w;
			}

			if (weightSum <= 0.0)
				return std::numeric_limits<double>::quiet_NaN();

			const double z_norm_final = weightedZ / weightSum;
			return denormalizeZ(z_norm_final);
		}

	private:
		std::vector<XyzSample> points_orig_;
		// normalized points [0,1]
		std::vector<XyzSample> points_norm_;
		std::unique_ptr<delaunator::Delaunator> triangulation_;
		std::vector<std::vector<std::size_t>> adjacency_;
		std::size_t K_ = 12u;
		mutable std::size_t lastVertex_ = delaunator::INVALID_INDEX;

		// normalization extents
		double x_min_ = 0.0, x_max_ = 1.0, x_range_ = 1.0;
		double y_min_ = 0.0, y_max_ = 1.0, y_range_ = 1.0;
		double z_min_ = 0.0, z_max_ = 1.0, z_range_ = 1.0;

		void normalizePoints()
		{
			if (points_orig_.empty()) return;
			x_min_ = x_max_ = points_orig_[0].x;
			y_min_ = y_max_ = points_orig_[0].y;
			z_min_ = z_max_ = points_orig_[0].z;

			for (const auto& p : points_orig_)
			{
				if (p.x < x_min_) x_min_ = p.x;
				if (p.x > x_max_) x_max_ = p.x;
				if (p.y < y_min_) y_min_ = p.y;
				if (p.y > y_max_) y_max_ = p.y;
				if (p.z < z_min_) z_min_ = p.z;
				if (p.z > z_max_) z_max_ = p.z;
			}

			x_range_ = (x_max_ - x_min_) > 0.0 ? (x_max_ - x_min_) : 1.0;
			y_range_ = (y_max_ - y_min_) > 0.0 ? (y_max_ - y_min_) : 1.0;
			z_range_ = (z_max_ - z_min_) > 0.0 ? (z_max_ - z_min_) : 1.0;

			points_norm_.clear();
			points_norm_.reserve(points_orig_.size());
			for (const auto& p : points_orig_)
			{
				XyzSample np;
				np.x = (p.x - x_min_) / x_range_;
				np.y = (p.y - y_min_) / y_range_;
				np.z = (p.z - z_min_) / z_range_;
				points_norm_.push_back(np);
			}
		}

		double normalizeX(double x) const noexcept { return (x - x_min_) / x_range_; }
		double normalizeY(double y) const noexcept { return (y - y_min_) / y_range_; }
		double denormalizeZ(double z_norm) const noexcept { return z_norm * z_range_ + z_min_; }

		void buildTriangulation()
		{
			if (points_norm_.empty())
			{
				triangulation_.reset();
				return;
			}

			std::vector<double> coords;
			coords.reserve(points_norm_.size() * 2u);
			for (const auto& p : points_norm_)
			{
				coords.push_back(p.x);
				coords.push_back(p.y);
			}

			triangulation_ = std::make_unique<delaunator::Delaunator>(coords);
		}

		void buildAdjacency()
		{
			adjacency_.clear();
			adjacency_.resize(points_norm_.size());
			if (!triangulation_) return;

			const auto& tri = triangulation_->triangles;
			const std::size_t triCount = tri.size() / 3u;
			// temporary use unordered_set per vertex to avoid duplicates
			std::vector<std::unordered_set<std::size_t>> neighbors(points_norm_.size());
			for (std::size_t t = 0; t < triCount; ++t)
			{
				const std::size_t a = tri[3u * t];
				const std::size_t b = tri[3u * t + 1u];
				const std::size_t c = tri[3u * t + 2u];
				neighbors[a].insert(b); neighbors[a].insert(c);
				neighbors[b].insert(a); neighbors[b].insert(c);
				neighbors[c].insert(a); neighbors[c].insert(b);
			}

			for (std::size_t i = 0; i < neighbors.size(); ++i)
			{
				adjacency_[i].reserve(neighbors[i].size());
				for (auto v : neighbors[i]) adjacency_[i].push_back(v);
			}
		}

		// Greedy graph-walking to find a local nearest vertex on adjacency graph
		std::size_t findClosestVertex(double nx, double ny) const
		{
			if (points_norm_.empty()) return delaunator::INVALID_INDEX;

			std::size_t current = (lastVertex_ != delaunator::INVALID_INDEX) ? lastVertex_ : 0u;
			auto dist = [&](std::size_t idx) noexcept {
				const auto& p = points_norm_[idx];
				const double dx = p.x - nx;
				const double dy = p.y - ny;
				return dx * dx + dy * dy;
			};

			double best = dist(current);
			bool moved = true;
			while (moved)
			{
				moved = false;
				for (std::size_t nb : adjacency_[current])
				{
					double dnb = dist(nb);
					if (dnb + 1e-15 < best)
					{
						best = dnb;
						current = nb;
						moved = true;
					}
				}
			}

			lastVertex_ = current;
			return current;
		}

		// BFS gather exactly K (or fewer if not available) unique vertices starting from seed
		std::vector<std::size_t> gatherKNeighbors(std::size_t seed, std::size_t K) const
		{
			std::vector<std::size_t> result;
			if (seed == delaunator::INVALID_INDEX || points_norm_.empty()) return result;

			result.reserve(K);
			std::vector<char> visited(points_norm_.size(), 0);
			std::deque<std::size_t> q;
			q.push_back(seed);
			visited[seed] = 1;

			while (!q.empty() && result.size() < K)
			{
				std::size_t v = q.front(); q.pop_front();
				result.push_back(v);
				for (std::size_t nb : adjacency_[v])
				{
					if (!visited[nb])
					{
						visited[nb] = 1;
						q.push_back(nb);
					}
				}
			}

			return result;
		}
	};
}

std::pair<std::vector<char>, std::vector<double>>
WavefrontFromContoursContext::rasterize(const std::vector<char>& mask) const
{
	const auto& visibilityMask = input_.visibilityMask_;
	const auto& fringeSegments = input_.fringeSegments_;
	
	// Use pre-resolved output dimensions from constructor
	int outWidth = input_.outWidth_;
	int outHeight = input_.outHeight_;

	// Allocate result with output dimensions
	std::vector<double> zk(outHeight * outWidth, std::numeric_limits<double>::quiet_NaN());
	std::vector<char> knownZ(outHeight * outWidth, 0);

	// Lambda to draw a line using Bresenham's algorithm
	// Respects output bounds and visibility mask
	auto drawLine = [&](int x0, int y0, int x1, int y1, double h)
	{
		// Cohen-Sutherland line clipping to ensure rasterization stays within bounds
		const int xMin = 0, xMax = outWidth - 1;
		const int yMin = 0, yMax = outHeight - 1;
		
		// Compute outcode for point (x, y)
		auto computeCode = [=](int x, int y) -> int {
			int code = 0;
			if (x < xMin) code |= 1;      // left
			if (x > xMax) code |= 2;      // right
			if (y < yMin) code |= 4;      // bottom
			if (y > yMax) code |= 8;      // top
			return code;
		};
		
		int code0 = computeCode(x0, y0);
		int code1 = computeCode(x1, y1);
		
		// If both endpoints are outside on the same side, skip
		if ((code0 & code1) != 0) {
			return;  // Line completely outside
		}
		
		// Clip endpoints to bounds
		while ((code0 | code1) != 0) {
			if ((code0 & code1) != 0) return;  // Completely outside
			
			int codeOut = code0 != 0 ? code0 : code1;
			int x, y;
			
			// Find intersection of line with edge
			if (codeOut & 1) {  // left
				x = xMin;
				y = y0 + (y1 - y0) * (xMin - x0) / (x1 - x0);
			} else if (codeOut & 2) {  // right
				x = xMax;
				y = y0 + (y1 - y0) * (xMax - x0) / (x1 - x0);
			} else if (codeOut & 4) {  // bottom
				y = yMin;
				x = x0 + (x1 - x0) * (yMin - y0) / (y1 - y0);
			} else {  // top
				y = yMax;
				x = x0 + (x1 - x0) * (yMax - y0) / (y1 - y0);
			}
			
			if (codeOut == code0) {
				x0 = x; y0 = y;
				code0 = computeCode(x0, y0);
			} else {
				x1 = x; y1 = y;
				code1 = computeCode(x1, y1);
			}
		}
		
		// Rasterize clipped line using Bresenham's algorithm
		int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;

		while (true)
		{
			// Bounds check (should never fail after clipping, but defensive)
			if (x0 >= 0 && x0 < outWidth && y0 >= 0 && y0 < outHeight)
			{
				int outIndex = y0 * outWidth + x0;
				if (mask[outIndex])  // Check visibility
				{
					knownZ[outIndex] = 1;
					zk[outIndex] = h;
				}
			}

			if (x0 == x1 && y0 == y1) break;
			int e2 = 2 * err;
			if (e2 >= dy) { err += dy; x0 += sx; }
			if (e2 <= dx) { err += dx; y0 += sy; }
		}
	};

	// Draw lines for each fringe segment
	for (const auto& fringe : fringeSegments)
	{
		double fringeValue = fringe.GetNumber() / input_.scaleFactor_;
		int pointCount = fringe.GetPointCount();

		// Draw lines connecting consecutive points in the fringe
		for (int i = 0; i < pointCount - 1; ++i)
		{
			CDPoint p0 = fringe.GetPoint(i);
			CDPoint p1 = fringe.GetPoint(i + 1);

			// Handle coordinate system conversion
			p0.y = convertY(p0.y);
			p1.y = convertY(p1.y);

			// Convert to output pixel coordinates
			double u0 = xToOutput(p0.x);
			double v0 = yToOutput(p0.y);
			double u1 = xToOutput(p1.x);
			double v1 = yToOutput(p1.y);

			int px0 = static_cast<int>(u0);
			int py0 = static_cast<int>(v0);
			int px1 = static_cast<int>(u1);
			int py1 = static_cast<int>(v1);

			// Draw the line segment
			drawLine(px0, py0, px1, py1, fringeValue);
		}
	}

	return {knownZ, zk};
}

std::vector<WavefrontFromContoursContext::FringeCrossing>
WavefrontFromContoursContext::findFringeCrossings(double worldY) const
{
	std::vector<FringeCrossing> crossings;
	const auto& fringeSegments = input_.fringeSegments_;

	constexpr double eps = 1e-12;

	for (const auto& fringe : fringeSegments)
	{
		double fringeValue = fringe.GetNumber();
		int pointCount = fringe.GetPointCount();

		for (int i = 0; i < pointCount - 1; ++i)
		{
			CDPoint p0 = fringe.GetPoint(i);
			CDPoint p1 = fringe.GetPoint(i + 1);

			double y0 = p0.y;
			double y1 = p1.y;

			// ------------------------------------------------------------
			// Case 1: Horizontal segment
			// ------------------------------------------------------------
			if (std::abs(y1 - y0) < eps)
			{
				if (std::abs(worldY - y0) < eps)
				{
					double centerX = 0.5 * (p0.x + p1.x);
					crossings.push_back({ centerX, fringeValue });
				}
				continue;
			}

			// ------------------------------------------------------------
			// Case 2: Non-horizontal segment
			// Use half-open rule: [ymin, ymax)
			// ------------------------------------------------------------
			double ymin = (std::min)(y0, y1);
			double ymax = (std::max)(y0, y1);

			if (worldY >= ymin - eps && worldY < ymax - eps)
			{
				double t = (worldY - y0) / (y1 - y0);
				double crossX = p0.x + t * (p1.x - p0.x);

				crossings.push_back({ crossX, fringeValue });
			}
		}
	}

	std::sort(crossings.begin(), crossings.end());
	return crossings;
}

std::ostream& operator<<(std::ostream& os, const WavefrontFromContoursResult& result)
{
	os << "MatrixXd(" << result.rows_ << " x " << result.cols_ << ")\n";

	// Set formatting for floating point
	std::streamsize prevPrecision = os.precision();
	os.precision(6);
	os << std::fixed;

	for (int y = 0; y < result.rows_; ++y)
	{
		for (int x = 0; x < result.cols_; ++x)
		{
			int idx = y * result.cols_ + x;
			double val = result.data_[idx];
			
			if (std::isnan(val))
			{
				// os << std::setw(1) << '_';  // 
				os << std::setw(6) << "NaN";
			}
			else if (std::isinf(val))
			{
				os << std::setw(6) << (val > 0 ? "Inf" : "-Inf");
			}
			else
			{
				//os << std::setw(1) << static_cast<int>(val)%10;
				os << std::setw(6) << val;
			}

			if (x < result.cols_ - 1)
				os << " ";
		}
		os << "\n";
	}

	// Restore formatting
	os.precision(prevPrecision);
	os.unsetf(std::ios_base::fixed);

	return os;
}

	
bool WavefrontFromContoursResult::saveMtrMatrix(std::ostream& os) const
{
	if (data_.empty() || rows_ == 0 || cols_ == 0)
		return false;

	// Calculate matrix size and normalization parameters
	auto boundingCircle = getBoundingCircle();
	if (!boundingCircle.valid) return false;
	size_t sizeMatrix = static_cast<size_t>(boundingCircle.radius * 2); // Use bounding circle diameter
	sizeMatrix |= 1;  // Ensure odd size for symmetry
	double ratio = 2.0 / (sizeMatrix - 1);  // Scale factor to fit largest dimension into [-1, 1]
	int xc = static_cast<int>(boundingCircle.center.x);  // Center column index
	int yc = static_cast<int>(boundingCircle.center.y);  // Center row index

	// Write header
	std::streamsize oldPrec = os.precision();
	os << std::fixed << std::setprecision(4);
	
	os << "Title=\n";
	std::time_t t = std::time(nullptr);  // Get current time
	std::tm* local = std::localtime(&t);  // Convert to local time

	// Format time
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", local);
	os << "Date=" << buffer << "\n";
	std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
	os << "Time=" << buffer << "\n";

	if(getScaleFactor() != 1.0) os << "ScaleFactor=" << getScaleFactor() << "\n";
	if(getFiScan() != 0.0) os << "FiScan=" << getFiScan() << "\n";
	
	os << "Units=WAV\n\n";
	// NB:essentially Size = 1./delta - not nesserery eq to rows_ or cols_.
	// Setting a wrong Size value breaks WinFringe calculations
	os << "Size=" << sizeMatrix << "\n"; 
	os << "[MATRIX]\n";

	// Format settings
	const int pairsPerLine = 6;
	const std::string indent(7, ' ');  // 7 spaces to align with first Z value

	std::ios_base::fmtflags oldFlags = os.flags();

	// Iterate through rows
	for (int row = 0; row < rows_; ++row)
	{
		// Calculate Y coordinate at the center of this row
		double y_norm = (row - yc) * ratio;  // Normalize to [-1, 1]
		if (y_norm > 1.0 || y_norm < -1.0) continue; // going out of -1..1 range kills WinFringe - skip such rows entirely

		bool firstLineOfRow = true;
		int pairCount = 0;
		size_t rowCount = 0;

		// Write all columns for this row
		for (int col = 0; col < cols_; ++col)
		{
			int idx = row * cols_ + col;
			double z = data_[idx];
			if (std::isnan(z) || std::isinf(z))
				continue;  // Skip NaN/Inf values entirely for MTR output

			// Calculate X coordinate at the center of this column
			double x_norm = (col - xc) * ratio;  // Normalize to [-1, 1]
			if (x_norm > 1.0 || x_norm < -1.0) continue; // going out of -1..1 range kills WinFringe

			// Start new output line if needed
			if (pairCount == 0)
			{
				if (firstLineOfRow)
				{
					os << y_norm;  // First line starts with normalized Y
					firstLineOfRow = false;
				}
				else
				{
					os << "\n" << indent;  // Continuation line with indentation
				}
			}

			// Write (Z, X) pair
			os << " " << z << " " << x_norm;
			pairCount++;
			rowCount++;

			// Move to next text line after 6 pairs (if more columns remain)
			if (pairCount == pairsPerLine && col < cols_ - 1)
			{
				pairCount = 0;
			}
		}

		// End of matrix row with E tag
		if(rowCount != 0) os << " E\n";
	}

	os << "END\n";

	// Restore formatting
	os.flags(oldFlags);
	os.precision(oldPrec);

	return true;
}

// Solver implementation
WavefrontFromContoursResult WavefrontFromContoursSolver_Bilinear::solve(const WavefrontFromContoursContext& ctx) const
{
	auto mask = ctx.buildMask();
	auto [knownZ, zk] = ctx.rasterize(mask);

	int outWidth = 0, outHeight = 0;
	getContextDimensions(ctx, outWidth, outHeight);
	
	// Perform bilinear interpolation to fill unknown values in visible regions
	performBilinearInterpolation(zk, knownZ, mask, outHeight, outWidth);

	// Convert bounds to output coordinate system
	aperture::Bounds outputBounds = ctx.convertBounds(ctx.input_.bounds_);

	// Populate result
	WavefrontFromContoursResult result;
	result.setMatrixData(zk.data(), outHeight, outWidth);
	result.setBounds(outputBounds);
	result.setCoordinateSystem(ctx.input_.outputCoordType_);
	result.setBoundingCircle(ctx.computeMaskBoundingCircle(mask));
	result.setScaleFactor(1.0); // reterize() scales due to it
	result.setFiScan(ctx.input_.fiScan_);

	return result;
}

// WavefrontFromContoursResult::setMatrixData implementation
void WavefrontFromContoursResult::setMatrixData(const double* data, int rows, int cols)
{
	rows_ = rows;
	cols_ = cols;
	data_.assign(data, data + rows * cols);
}

// ============================================================================
// WavefrontFromContoursSolver_HorizontalLinear Implementation
// ============================================================================

double WavefrontFromContoursSolver_HorizontalLinear::interpolateAtX(
	const std::vector<WavefrontFromContoursContext::FringeCrossing>& crossings,
	double worldX) const
{
	if (crossings.empty())
		return std::numeric_limits<double>::quiet_NaN();
	
	// Find the two crossings that bracket worldX
	// If worldX is before first crossing or after last, extrapolate or return NaN
	
	// Find first crossing at or after worldX
	auto it = std::lower_bound(crossings.begin(), crossings.end(), 
		WavefrontFromContoursContext::FringeCrossing{worldX, 0.0});
	
	// If worldX is before all crossings, use first crossing value (extrapolate)
	if (it == crossings.begin())
	{
		return crossings.front().fringeValue;
	}
	
	// If worldX is after all crossings, use last crossing value (extrapolate)
	if (it == crossings.end())
	{
		return crossings.back().fringeValue;
	}
	
	// worldX is between two crossings - interpolate
	const WavefrontFromContoursContext::FringeCrossing& right = *it;
	const WavefrontFromContoursContext::FringeCrossing& left = *(it - 1);
	
	// Linear interpolation
	double dx = right.x - left.x;
	if (std::abs(dx) < 1e-10)
	{
		// Crossings are at same X - average the values
		return (left.fringeValue + right.fringeValue) / 2.0;
	}
	
	double t = (worldX - left.x) / dx;
	return left.fringeValue + t * (right.fringeValue - left.fringeValue);
}

WavefrontFromContoursResult WavefrontFromContoursSolver_HorizontalLinear::solve(
	const WavefrontFromContoursContext& ctx) const
{
	// Get output dimensions
	int outWidth = 0, outHeight = 0;
	getContextDimensions(ctx, outWidth, outHeight);
	
	// Build visibility mask
	auto mask = ctx.buildMask();
	
	// Allocate output matrix
	std::vector<double> zk(outHeight * outWidth, std::numeric_limits<double>::quiet_NaN());
	
	const auto& bounds = ctx.input_.bounds_;
	const double reverseScale = 1.0 / ctx.input_.scaleFactor_;
	
	// Process each row
	for (int row = 0; row < outHeight; ++row)
	{
		// Calculate world Y coordinate for this row (center of pixel)
		double worldY = ctx.yToInput(static_cast<double>(row));
		
		// Find all fringe crossings at this Y
		auto crossings = ctx.findFringeCrossings(worldY);
		
		if (crossings.empty())
			continue; // No fringes at this Y, leave as NaN
		
		// Process each column in this row
		for (int col = 0; col < outWidth; ++col)
		{
			int idx = row * outWidth + col;
			
			// Skip if not visible
			if (!mask[idx])
				continue;
			
			// Calculate world X coordinate for this column (center of pixel)
			double worldX = ctx.xToInput(static_cast<double>(col));
			
			// Interpolate Z value at this X
			double z = interpolateAtX(crossings, worldX);
			zk[idx] = z * reverseScale;
		}
	}
	
	// Convert bounds to output coordinate system
	aperture::Bounds outputBounds = ctx.convertBounds(ctx.input_.bounds_);
	
	// Populate result
	WavefrontFromContoursResult result;
	result.setMatrixData(zk.data(), outHeight, outWidth);
	result.setBounds(outputBounds);
	result.setCoordinateSystem(ctx.input_.outputCoordType_);
	result.setBoundingCircle(ctx.computeMaskBoundingCircle(mask));
	result.setScaleFactor(1.0); // already scaled
	result.setFiScan(ctx.input_.fiScan_);

	return result;
}

// ============================================================================
// WavefrontFromContoursSolver_HorizontalSpline Implementation
// ============================================================================

WavefrontFromContoursSolver_HorizontalSpline::SplineCoefficients
WavefrontFromContoursSolver_HorizontalSpline::buildSpline(
	const std::vector<WavefrontFromContoursContext::FringeCrossing>& crossings) const
{
	SplineCoefficients spline;
	int n = static_cast<int>(crossings.size());
	
	if (n == 0)
		return spline;
	
	// Extract x and y values
	spline.x.resize(n);
	spline.a.resize(n);
	for (int i = 0; i < n; ++i)
	{
		spline.x[i] = crossings[i].x;
		spline.a[i] = crossings[i].fringeValue;
	}
	
	if (n == 1)
	{
		// Single point - constant interpolation
		spline.b.resize(1, 0.0);
		spline.c.resize(1, 0.0);
		spline.d.resize(1, 0.0);
		return spline;
	}
	
	if (n == 2)
	{
		// Two points - linear interpolation
		spline.b.resize(1);
		spline.c.resize(1, 0.0);
		spline.d.resize(1, 0.0);
		double h = spline.x[1] - spline.x[0];
		if (std::abs(h) > 1e-10)
			spline.b[0] = (spline.a[1] - spline.a[0]) / h;
		else
			spline.b[0] = 0.0;
		return spline;
	}
	
	// Natural cubic spline for n >= 3
	// Build tridiagonal system for second derivatives
	std::vector<double> h(n - 1);  // Step sizes
	for (int i = 0; i < n - 1; ++i)
		h[i] = spline.x[i + 1] - spline.x[i];
	
	// Tridiagonal system: alpha = RHS
	std::vector<double> alpha(n);
	for (int i = 1; i < n - 1; ++i)
	{
		alpha[i] = 3.0 * ((spline.a[i + 1] - spline.a[i]) / h[i] - 
		                   (spline.a[i] - spline.a[i - 1]) / h[i - 1]);
	}
	
	// Solve tridiagonal system using Thomas algorithm
	std::vector<double> l(n, 1.0);
	std::vector<double> mu(n, 0.0);
	std::vector<double> z(n, 0.0);
	
	for (int i = 1; i < n - 1; ++i)
	{
		l[i] = 2.0 * (spline.x[i + 1] - spline.x[i - 1]) - h[i - 1] * mu[i - 1];
		if (std::abs(l[i]) < 1e-10)
			l[i] = 1e-10; // Avoid division by zero
		mu[i] = h[i] / l[i];
		z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
	}
	
	// Back substitution
	spline.c.resize(n, 0.0);
	spline.b.resize(n - 1);
	spline.d.resize(n - 1);
	
	// Natural spline boundary conditions: c[0] = c[n-1] = 0
	for (int j = n - 2; j >= 0; --j)
	{
		spline.c[j] = z[j] - mu[j] * spline.c[j + 1];
		spline.b[j] = (spline.a[j + 1] - spline.a[j]) / h[j] - 
		               h[j] * (spline.c[j + 1] + 2.0 * spline.c[j]) / 3.0;
		spline.d[j] = (spline.c[j + 1] - spline.c[j]) / (3.0 * h[j]);
	}
	
	return spline;
}

double WavefrontFromContoursSolver_HorizontalSpline::SplineCoefficients::evaluate(double xi) const
{
	int n = static_cast<int>(x.size());
	
	if (n == 0)
		return std::numeric_limits<double>::quiet_NaN();
	
	if (n == 1)
		return a[0]; // Constant
	
	// Find interval: x[i] <= xi < x[i+1]
	// Handle extrapolation
	if (xi <= x[0])
	{
		// Extrapolate using first segment
		double dx = xi - x[0];
		return a[0] + b[0] * dx + c[0] * dx * dx + d[0] * dx * dx * dx;
	}
	
	if (xi >= x[n - 1])
	{
		// Extrapolate using last segment
		int i = n - 2;
		double dx = xi - x[i];
		return a[i] + b[i] * dx + c[i] * dx * dx + d[i] * dx * dx * dx;
	}
	
	// Binary search for interval
	int i = 0;
	int j = n - 1;
	while (j - i > 1)
	{
		int k = (i + j) / 2;
		if (xi < x[k])
			j = k;
		else
			i = k;
	}
	
	// Evaluate cubic polynomial at xi
	double dx = xi - x[i];
	return a[i] + b[i] * dx + c[i] * dx * dx + d[i] * dx * dx * dx;
}

double WavefrontFromContoursSolver_HorizontalSpline::interpolateAtX(
	const std::vector<WavefrontFromContoursContext::FringeCrossing>& crossings,
	double worldX) const
{
	if (crossings.empty())
		return std::numeric_limits<double>::quiet_NaN();
	
	// Build cubic spline
	SplineCoefficients spline = buildSpline(crossings);
	
	// Evaluate at worldX
	return spline.evaluate(worldX);
}

WavefrontFromContoursResult WavefrontFromContoursSolver_HorizontalSpline::solve(
	const WavefrontFromContoursContext& ctx) const
{
	// Get output dimensions
	int outWidth = 0, outHeight = 0;
	getContextDimensions(ctx, outWidth, outHeight);
	
	// Build visibility mask
	auto mask = ctx.buildMask();
	
	// Allocate output matrix
	std::vector<double> zk(outHeight * outWidth, std::numeric_limits<double>::quiet_NaN());
	
	const auto& bounds = ctx.input_.bounds_;
	const double reverseScale = 1.0 / ctx.input_.scaleFactor_;

	// Process each row
	for (int row = 0; row < outHeight; ++row)
	{
		// Calculate world Y coordinate for this row (center of pixel)
		double worldY = ctx.yToInput(static_cast<double>(row));
		
		// Find all fringe crossings at this Y
		auto crossings = ctx.findFringeCrossings(worldY);
		
		if (crossings.size() < 2)
			continue; // Need at least 2 points for meaningful spline
		
		// Process each column in this row
		for (int col = 0; col < outWidth; ++col)
		{
			int idx = row * outWidth + col;
			
			// Skip if not visible
			if (!mask[idx])
				continue;
			
			// Calculate world X coordinate for this column (center of pixel)
			double worldX = ctx.xToInput(static_cast<double>(col));
			
			// Interpolate Z value at this X using cubic spline
			double z = interpolateAtX(crossings, worldX);
			zk[idx] = z * reverseScale;
		}
	}
	
	// Convert bounds to output coordinate system
	aperture::Bounds outputBounds = ctx.convertBounds(ctx.input_.bounds_);
	
	// Populate result
	WavefrontFromContoursResult result;
	result.setMatrixData(zk.data(), outHeight, outWidth);
	result.setBounds(outputBounds);
	result.setCoordinateSystem(ctx.input_.outputCoordType_);
	result.setBoundingCircle(ctx.computeMaskBoundingCircle(mask));
	result.setScaleFactor(1.0); // already scaled here
	result.setFiScan(ctx.input_.fiScan_);

	return result;
}

std::vector<XyzSample> WavefrontFromContoursSolver_Delaunay::prepareSamples(
	const WavefrontFromContoursContext& ctx) const
{
	const double safeScaleFactor = std::abs(ctx.input_.scaleFactor_) > 1e-12 ? ctx.input_.scaleFactor_ : 1.0;
	std::vector<XyzSample> samples;
	std::size_t sampleEstimate = 0u;
	for (const auto& fringe : ctx.input_.fringeSegments_)
		sampleEstimate += fringe.GetPointCount() > 0 ? static_cast<std::size_t>(fringe.GetPointCount()) : 0u;
	samples.reserve(sampleEstimate);

	for (const auto& fringe : ctx.input_.fringeSegments_)
	{
		const double z = fringe.GetNumber() / safeScaleFactor;
		const int pointCount = fringe.GetPointCount();
		for (int i = 0; i < pointCount; ++i)
		{
			CDPoint p = fringe.GetPoint(i);
			if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(z))
				continue;
			samples.push_back(XyzSample{p.x, p.y, z});
		}
	}

	if (samples.size() > 1u)
	{
		struct SampleAggregate
		{
			double x = 0.0;
			double y = 0.0;
			double z = 0.0;
			std::size_t count = 0u;
		};

		std::sort(samples.begin(), samples.end(), [](const XyzSample& a, const XyzSample& b)
		{
			if (a.x != b.x) return a.x < b.x;
			if (a.y != b.y) return a.y < b.y;
			return a.z < b.z;
		});

		std::vector<SampleAggregate> merged;
		merged.reserve(samples.size());
		for (const auto& sample : samples)
		{
			if (!merged.empty() && merged.back().x == sample.x && merged.back().y == sample.y)
			{
				merged.back().z += sample.z;
				++merged.back().count;
			}
			else
			{
				merged.push_back(SampleAggregate{sample.x, sample.y, sample.z, 1u});
			}
		}

		std::vector<XyzSample> uniqueSamples;
		uniqueSamples.reserve(merged.size());
		for (const auto& sample : merged)
		{
			uniqueSamples.push_back(XyzSample{
				sample.x,
				sample.y,
				sample.z / static_cast<double>(sample.count)
			});
		}
		samples.swap(uniqueSamples);
	}

	return samples;
}

WavefrontFromContoursResult WavefrontFromContoursSolver_Delaunay::solve(
	const WavefrontFromContoursContext& ctx) const
{
	int outWidth = 0;
	int outHeight = 0;
	getContextDimensions(ctx, outWidth, outHeight);

	auto mask = ctx.buildMask();
	std::vector<double> zk(static_cast<std::size_t>(outWidth) * static_cast<std::size_t>(outHeight),
		std::numeric_limits<double>::quiet_NaN());

	std::vector<XyzSample> samples = prepareSamples(ctx); // also scales Z by 1/scaleFactor
	
	//LocalQuadricInterpolator interpolator(std::move(samples));
	CloughTocherInterpolator interpolator(std::move(samples));
	//AkimaBivariateInterpolator interpolator(std::move(samples));
	//BarycentricInterpolator interpolator(std::move(samples));
	//IDWInterpolator interpolator(std::move(samples));

	for (int row = 0; row < outHeight; ++row)
	{
		const double worldY = ctx.yToInput(static_cast<double>(row));
		const std::size_t rowOffset = static_cast<std::size_t>(row) * static_cast<std::size_t>(outWidth);

		for (int col = 0; col < outWidth; ++col)
		{
			const std::size_t idx = rowOffset + static_cast<std::size_t>(col);
			if (!mask[idx])
				continue;

			const double worldX = ctx.xToInput(static_cast<double>(col));
			const double z = interpolator.query(worldX, worldY);
			if (std::isfinite(z))
				zk[idx] = z; // samples were scaled in prepareSamples() by 1/scaleFactor, so no need to scale here
		}
	}

	aperture::Bounds outputBounds = ctx.convertBounds(ctx.input_.bounds_);
	WavefrontFromContoursResult result;
	result.setMatrixData(zk.data(), outHeight, outWidth);
	result.setBounds(outputBounds);
	result.setCoordinateSystem(ctx.input_.outputCoordType_);
	result.setBoundingCircle(ctx.computeMaskBoundingCircle(mask));
	result.setScaleFactor(1.0);
	result.setFiScan(ctx.input_.fiScan_);
	return result;
}

WavefrontBoundingCircle WavefrontFromContoursContext::computeMaskBoundingCircle(const std::vector<char>& mask) const
{
	const int outWidth = input_.outWidth_;
	const int outHeight = input_.outHeight_;
	if (mask.empty() || outWidth <= 0 || outHeight <= 0)
		return {};

	std::vector<WavefrontPrimitivePoint> points;
	points.reserve(static_cast<size_t>(outHeight) * 2u);

	for (int row = 0; row < outHeight; ++row)
	{
		const int rowOffset = row * outWidth;
		int firstVisible = -1;
		int lastVisible = -1;

		for (int col = 0; col < outWidth; ++col)
		{
			if (mask[rowOffset + col] == 0)
				continue;
			if (firstVisible < 0)
				firstVisible = col;
			lastVisible = col;
		}

		if (firstVisible < 0)
			continue;

		points.push_back({ static_cast<double>(firstVisible), static_cast<double>(row) });
		if (lastVisible != firstVisible)
		{
			points.push_back({ static_cast<double>(lastVisible), static_cast<double>(row) });
		}
	}

	if (points.empty())
		return {};

	std::mt19937 rng(0xD16D1234u);
	std::shuffle(points.begin(), points.end(), rng);

	std::vector<WavefrontPrimitivePoint> boundary;
	boundary.reserve(3u);
	return welzl(points, boundary, static_cast<int>(points.size()));
}
