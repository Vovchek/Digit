#include <Eigen/Sparse>
#include <vector>

// Isoline Type (convert from Cfringe)
struct IsoLine
{
	std::vector<Eigen::Vector2d> pts; // pixel coords
	double height;
};

// Reconstruction Class
class WavefrontFromIsolines
{
public:

	struct Params
	{
		int W, H;
		double nanValue = std::numeric_limits<double>::quiet_NaN();
		double cgTol = 1e-6;
		int    cgIter = 2000;
	};

	template<class VisibleFn>
	Eigen::MatrixXd solve(
		const std::vector<IsoLine>& isolines,
		VisibleFn isVisible,
		const Params& p)
	{
		const int N = p.W * p.H;

		buildMask(isVisible, p);
		rasterizeIsolines(isolines, p);

		SpMat L = buildMaskedL(p);

		Eigen::VectorXd w = solvePoisson(
			L, knownW, zeroVec(N), p);

		Eigen::VectorXd z = solvePoisson(
			L, knownZ, w, p);

		return toImage(z, p);
	}

private:

	typedef Eigen::SparseMatrix<double> SpMat;
	typedef Eigen::Triplet<double> T;

	std::vector<char> visible;
	std::vector<char> knownZ;
	std::vector<char> knownW;
	Eigen::VectorXd   zk;

	// Mask Builder
	template<class VisibleFn>
	void buildMask(VisibleFn vis, const Params& p)
	{
		visible.assign(p.W * p.H, 0);
		knownZ.assign(p.W * p.H, 0);
		knownW.assign(p.W * p.H, 0);
		zk.resize(p.W * p.H);

		for (int y = 0; y < p.H; y++)
			for (int x = 0; x < p.W; x++)
				visible[id(x, y, p)] = vis(x, y);
	}

	// Rasterize Isolines (Bresenham — single pixel thick)
	void drawLine(int x0, int y0, int x1, int y1,
		double h, const Params& p)
	{
		int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;

		while (true)
		{
			int i = id(x0, y0, p);
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

	void rasterizeIsolines(
		const std::vector<IsoLine>& iso,
		const Params& p)
	{
		for (auto& l : iso)
			for (size_t i = 1; i < l.pts.size(); i++)
				drawLine(
					l.pts[i - 1].x(), l.pts[i - 1].y(),
					l.pts[i].x(), l.pts[i].y(),
					l.height, p);
	}

	// Masked Laplacian (auto-Neumann on aperture edge)
	SpMat buildMaskedL(const Params& p)
	{
		std::vector<T> t;

		for (int y = 0; y < p.H; y++)
			for (int x = 0; x < p.W; x++)
			{
				int i = id(x, y, p);
				if (!visible[i]) continue;

				int n = 0;

				auto add = [&](int nx, int ny)
					{
						if (nx >= 0 && nx < p.W &&
							ny >= 0 && ny < p.H)
						{
							int j = id(nx, ny, p);
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

		SpMat L(p.W * p.H, p.W * p.H);
		L.setFromTriplets(t.begin(), t.end());
		return L;
	}

	// Generic Poisson Solver
	Eigen::VectorXd solvePoisson(
		SpMat& L,
		const std::vector<char>& known,
		const Eigen::VectorXd& rhs,
		const Params& p)
	{
		SpMat A = L;
		Eigen::VectorXd b = rhs;

		for (int i = 0; i < A.rows(); i++)
			if (!visible[i] || known[i])
			{
				A.coeffRef(i, i) = 1;
				b[i] = known[i] ? zk[i] : 0;
			}

		Eigen::ConjugateGradient<
			SpMat, Eigen::Lower | Eigen::Upper> cg;

		cg.setTolerance(p.cgTol);
		cg.setMaxIterations(p.cgIter);

		cg.compute(A);
		return cg.solve(b);
	}

	// Output Z-map
	Eigen::MatrixXd toImage(
		const Eigen::VectorXd& z,
		const Params& p)
	{
		Eigen::MatrixXd M(p.H, p.W);

		for (int y = 0; y < p.H; y++)
			for (int x = 0; x < p.W; x++)
			{
				int i = id(x, y, p);
				M(y, x) = visible[i] ?
					z[i] :
					p.nanValue;
			}
		return M;
	}

	// Index Helper
	inline int id(int x, int y, const Params& p)
	{
		return y * p.W + x;
	}
};


// ✔ Usage
//
//WavefrontFromIsolines wf;
//
//WavefrontFromIsolines::Params p;
//p.W=1000;
//p.H=1000;
//
//Eigen::MatrixXd Z =
//    wf.solve(isolines,isVisible,p);
//
