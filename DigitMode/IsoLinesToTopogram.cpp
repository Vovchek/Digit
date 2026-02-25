#include <Eigen/Sparse>
#include <vector>
#include "ApertureCore/include/aperturecore/visibility/VisibilityMask.h"
#include "DigitMode\CFringeSegment.h"

// Forward declaration for CFringe

/**
 * @brief Wavefront reconstruction from fringe contours
 * 
 * Reconstructs a height map (topogram) from fringe line contours
 * using Poisson equation solving over a visibility mask.
 */
class WavefrontFromIsolines
{
public:

	struct Params
	{
		double nanValue = std::numeric_limits<double>::quiet_NaN();
		double cgTol    = 1e-6;
		int    cgIter   = 2000;
	};

	/**
	 * @brief Solve for height map from fringes
	 * @param fringes Vector of CFringe objects (number used as height)
	 * @param mask Visibility mask defining aperture region
	 * @param p Solver parameters
	 * @return Height map (topogram) - NaN outside aperture
	 */
	Eigen::MatrixXd solve(
		const std::vector<CFringeSegment>& fringes,
		const aperture::visibility::VisibilityMask& mask,
		const Params& p)
	{
		const int N = mask.width * mask.height;

		buildMask(mask);
		rasterizeFringes(fringes, mask);

		SpMat L = buildMaskedL(mask);

		Eigen::VectorXd w = solvePoisson(
			L, knownW, Eigen::VectorXd::Zero(N), p, mask);

		Eigen::VectorXd z = solvePoisson(
			L, knownZ, w, p, mask);

		return toImage(z, mask, p);
	}

private:

	typedef Eigen::SparseMatrix<double> SpMat;
	typedef Eigen::Triplet<double> T;

	std::vector<char> visible;
	std::vector<char> knownZ;
	std::vector<char> knownW;
	Eigen::VectorXd   zk;

	// ========================================================================
	// Mask Builder
	// ========================================================================

	void buildMask(const aperture::visibility::VisibilityMask& mask)
	{
		const int N = mask.width * mask.height;
		visible.assign(N, 0);
		knownZ.assign(N, 0);
		knownW.assign(N, 0);
		zk.resize(N);

		for (int y = 0; y < mask.height; y++) {
			for (int x = 0; x < mask.width; x++) {
				visible[id(x, y, mask)] = mask.IsVisible(x, y) ? 1 : 0;
			}
		}
	}

	// ========================================================================
	// Fringe Rasterization (Bresenham — single pixel thick)
	// ========================================================================

	/**
	 * @brief Rasterize a line segment on the height map
	 * @param x0, y0 Start point (pixel coords)
	 * @param x1, y1 End point (pixel coords)
	 * @param h Height value (fringe number)
	 * @param mask Visibility mask
	 */
	void drawLine(int x0, int y0, int x1, int y1,
		double h, const aperture::visibility::VisibilityMask& mask)
	{
		int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;

		while (true)
		{
			int i = id(x0, y0, mask);
			if (visible[i])
			{
				knownZ[i] = 1;
				zk[i] = h;
			}

			if (x0 == x1 && y0 == y1) break;
			int e2 = 2 * err;
			if (e2 >= dy) { err += dy; x0 += sx; }
			if (e2 <= dx) { err += dx; y0 += sy; }
		}
	}

	/**
	 * @brief Rasterize all fringe contours
	 * Extracts point sequences from each fringe and draws them
	 */
	void rasterizeFringes(
		const std::vector<CFringeSegment>& fringes,
		const aperture::visibility::VisibilityMask& mask)
	{
		for (const auto& fringe : fringes)
		{
			double height = static_cast<double>(fringe.GetNumber());  // Use fringe number as height
			
			// Get fringe contour points
			// Assuming CFringe has a method to get points or direct access
			
			// Rasterize line segments
			for (int i = 1; i < fringe.GetPointCount(); i++)
			{
				int x0 = static_cast<int>(fringe.GetPoint(i - 1).x);
				int y0 = static_cast<int>(fringe.GetPoint(i - 1).y);
				int x1 = static_cast<int>(fringe.GetPoint(i).x);
				int y1 = static_cast<int>(fringe.GetPoint(i).y);
				
				drawLine(x0, y0, x1, y1, height, mask);
			}
		}
	}

	// ========================================================================
	// Laplacian Builder (auto-Neumann on aperture edge)
	// ========================================================================

	SpMat buildMaskedL(const aperture::visibility::VisibilityMask& mask)
	{
		std::vector<T> t;

		for (int y = 0; y < mask.height; y++)
		{
			for (int x = 0; x < mask.width; x++)
			{
				int i = id(x, y, mask);
				if (!visible[i]) continue;

				int n = 0;

				auto add = [&](int nx, int ny)
				{
					if (nx >= 0 && nx < mask.width &&
						ny >= 0 && ny < mask.height)
					{
						int j = id(nx, ny, mask);
						if (visible[j])
						{
							t.emplace_back(i, j, 1);
							n++;
						}
					}
				};

				add(x + 1, y);
				add(x - 1, y);
				add(x, y + 1);
				add(x, y - 1);

				t.emplace_back(i, i, -n);
			}
		}

		const int N = mask.width * mask.height;
		SpMat L(N, N);
		L.setFromTriplets(t.begin(), t.end());
		return L;
	}

	// ========================================================================
	// Poisson Solver (Conjugate Gradient)
	// ========================================================================

	Eigen::VectorXd solvePoisson(
		SpMat& L,
		const std::vector<char>& known,
		const Eigen::VectorXd& rhs,
		const Params& p,
		const aperture::visibility::VisibilityMask& mask)
	{
		SpMat A = L;
		Eigen::VectorXd b = rhs;

		const int N = mask.width * mask.height;
		for (int i = 0; i < N; i++)
		{
			if (!visible[i] || known[i])
			{
				A.coeffRef(i, i) = 1;
				b[i] = known[i] ? zk[i] : 0;
			}
		}

		Eigen::ConjugateGradient<
			SpMat, Eigen::Lower | Eigen::Upper> cg;

		cg.setTolerance(p.cgTol);
		cg.setMaxIterations(p.cgIter);

		cg.compute(A);
		return cg.solve(b);
	}

	// ========================================================================
	// Output Conversion
	// ========================================================================

	Eigen::MatrixXd toImage(
		const Eigen::VectorXd& z,
		const aperture::visibility::VisibilityMask& mask,
		const Params& p)
	{
		Eigen::MatrixXd M(mask.height, mask.width);

		for (int y = 0; y < mask.height; y++)
		{
			for (int x = 0; x < mask.width; x++)
			{
				int i = id(x, y, mask);
				M(y, x) = visible[i] ?
					z[i] :
					p.nanValue;
			}
		}
		return M;
	}

	// ========================================================================
	// Index Helper
	// ========================================================================

	inline int id(int x, int y, const aperture::visibility::VisibilityMask& mask) const
	{
		return y * mask.width + x;
	}
};

// ✔ Usage Example
//
// aperture::visibility::VisibilityMask mask(1000, 1000);
// // ... populate mask with aperture visibility ...
//
// std::vector<CFringe> fringes = /* ... */;
//
// WavefrontFromIsolines wf;
// WavefrontFromIsolines::Params p;
// p.cgTol = 1e-6;
// p.cgIter = 2000;
//
// Eigen::MatrixXd topogram = wf.solve(frings, mask, p);
//
