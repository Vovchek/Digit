#include "DigitMode/IsoLinesToTopogram.h"

// ============================================================================
// Public API
// ============================================================================

Eigen::MatrixXd WavefrontFromIsolines::solve(
	const std::vector<CFringeSegment>& fringes,
	const aperture::ShapeCollection& shapes,
	const aperture::visibility::VisibilityMask& mask,
	const Params& p)
{
	// Get bounding rect from shapes using ShapeCollection API
	aperture::Bounds apertureBounds = shapes.getCombinedBounds();
	
	// Resolve output resolution (default to aperture bounds, maintaining aspect ratio)
	int outWidth = p.outWidth > 0 ? p.outWidth : static_cast<int>(apertureBounds.width());
	int outHeight = p.outHeight > 0 ? p.outHeight : 
		static_cast<int>(apertureBounds.height() * outWidth / apertureBounds.width());

	const int N = mask.width * mask.height;

	buildMask(mask);
	rasterizeFringes(fringes, mask);

	SpMat L = buildMaskedL(mask);

	Eigen::VectorXd w = solvePoisson(
		L, knownW, Eigen::VectorXd::Zero(N), p, mask);

	Eigen::VectorXd z = solvePoisson(
		L, knownZ, w, p, mask);

	// Convert to image at original mask resolution
	Eigen::MatrixXd topogram = toImage(z, mask, p);

	// Resample to output resolution if different from mask
	if (outWidth != mask.width || outHeight != mask.height)
	{
		topogram = resampleToResolution(topogram, apertureBounds, outWidth, outHeight, p);
	}

	return topogram;
}

// ============================================================================
// Output Resampling (Bilinear interpolation)
// ============================================================================

Eigen::MatrixXd WavefrontFromIsolines::resampleToResolution(
	const Eigen::MatrixXd& original,
	const aperture::Bounds& apertureBounds,
	int outWidth,
	int outHeight,
	const Params& p)
{
	Eigen::MatrixXd resampled(outHeight, outWidth);

	double srcWidth = apertureBounds.width();
	double srcHeight = apertureBounds.height();
	double minX = apertureBounds.minX();
	double minY = apertureBounds.minY();

	for (int outY = 0; outY < outHeight; outY++)
	{
		for (int outX = 0; outX < outWidth; outX++)
		{
			// Map output pixel to original coordinate space
			double u = (outX + 0.5) / outWidth;  // Normalized [0,1]
			double v = (outY + 0.5) / outHeight;

			double origX = minX + u * srcWidth;
			double origY = minY + v * srcHeight;

			// Bilinear interpolation in original image
			double val = bilinearInterpolate(original, origX, origY, minX, minY, p);
			resampled(outY, outX) = val;
		}
	}

	return resampled;
}

double WavefrontFromIsolines::bilinearInterpolate(
	const Eigen::MatrixXd& image,
	double x, double y,
	double minX, double minY,
	const Params& p)
{
	// Convert world coords to image pixel coords
	double px = x - minX;
	double py = y - minY;

	int x0 = static_cast<int>(px);
	int y0 = static_cast<int>(py);
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// Clamp to image bounds
	if (x0 < 0 || x1 >= image.cols() || y0 < 0 || y1 >= image.rows())
	{
		return p.nanValue;
	}

	double fx = px - x0;  // Fractional part
	double fy = py - y0;

	double v00 = image(y0, x0);
	double v01 = image(y0, x1);
	double v10 = image(y1, x0);
	double v11 = image(y1, x1);

	// Handle NaN values in source
	if (std::isnan(v00) || std::isnan(v01) || 
	    std::isnan(v10) || std::isnan(v11))
	{
		return p.nanValue;
	}

	// Bilinear blend
	double v0 = v00 * (1.0 - fx) + v01 * fx;
	double v1 = v10 * (1.0 - fx) + v11 * fx;
	return v0 * (1.0 - fy) + v1 * fy;
}

// ============================================================================
// Mask Builder
// ============================================================================

void WavefrontFromIsolines::buildMask(const aperture::visibility::VisibilityMask& mask)
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

// ============================================================================
// Fringe Rasterization (Bresenham — single pixel thick)
// ============================================================================

void WavefrontFromIsolines::drawLine(int x0, int y0, int x1, int y1,
	double h, const aperture::visibility::VisibilityMask& mask)
{
	// Clip line endpoints to mask bounds using Cohen-Sutherland algorithm
	// to ensure rasterization stays within [0, mask.width) x [0, mask.height)
	
	const int xMin = 0, xMax = mask.width - 1;
	const int yMin = 0, yMax = mask.height - 1;
	
	// Cohen-Sutherland line clipping
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
	
	// Rasterize clipped line using Bresenham
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;

	while (true)
	{
		// Bounds check (should never fail after clipping, but defensive)
		if (x0 >= 0 && x0 < mask.width && y0 >= 0 && y0 < mask.height)
		{
			int i = id(x0, y0, mask);
			if (visible[i])
			{
				knownZ[i] = 1;
				zk[i] = h;
			}
		}

		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

void WavefrontFromIsolines::rasterizeFringes(
	const std::vector<CFringeSegment>& fringes,
	const aperture::visibility::VisibilityMask& mask)
{
	for (const auto& fringe : fringes)
	{
		double height = static_cast<double>(fringe.GetNumber());
		
		// Rasterize line segments between consecutive fringe points
		// Each segment is clipped to mask bounds before rasterization
		for (int i = 1; i < fringe.GetPointCount(); i++)
		{
			// Get endpoint coordinates (may be outside mask bounds)
			int x0 = static_cast<int>(std::round(fringe.GetPoint(i - 1).x));
			int y0 = static_cast<int>(std::round(fringe.GetPoint(i - 1).y));
			int x1 = static_cast<int>(std::round(fringe.GetPoint(i).x));
			int y1 = static_cast<int>(std::round(fringe.GetPoint(i).y));
			
			// drawLine clips to bounds before rasterizing
			drawLine(x0, y0, x1, y1, height, mask);
		}
	}
}

// ============================================================================
// Laplacian Builder (auto-Neumann on aperture edge)
// ============================================================================

WavefrontFromIsolines::SpMat WavefrontFromIsolines::buildMaskedL(const aperture::visibility::VisibilityMask& mask)
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

// ============================================================================
// Poisson Solver (Conjugate Gradient)
// ============================================================================

Eigen::VectorXd WavefrontFromIsolines::solvePoisson(
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

// ============================================================================
// Output Conversion
// ============================================================================

Eigen::MatrixXd WavefrontFromIsolines::toImage(
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

// ============================================================================
// Stream Serialization
// ============================================================================

std::ostream& operator<<(std::ostream& os, const Eigen::MatrixXd& matrix)
{
	os << "MatrixXd(" << matrix.rows() << " x " << matrix.cols() << ")\n";
	
	// Set formatting for floating point
	std::streamsize prevPrecision = os.precision();
	os.precision(6);
	os << std::fixed;
	
	for (int y = 0; y < matrix.rows(); ++y)
	{
		for (int x = 0; x < matrix.cols(); ++x)
		{
			double val = matrix(y, x);
			
			if (std::isnan(val))
			{
				os << std::setw(12) << "NaN";
			}
			else if (std::isinf(val))
			{
				os << std::setw(12) << (val > 0 ? "Inf" : "-Inf");
			}
			else
			{
				os << std::setw(12) << val;
			}
			
			if (x < matrix.cols() - 1)
				os << " ";
		}
		os << "\n";
	}
	
	// Restore formatting
	os.precision(prevPrecision);
	os.unsetf(std::ios_base::fixed);
	
	return os;
}

// ✔ Usage Example
//
// #include "DigitMode/IsoLinesToTopogram.h"
//
// aperture::visibility::VisibilityMask mask(1000, 1000);
// // ... populate mask with aperture visibility ...
//
// aperture::ShapeCollection shapes;
// // ... add shapes to collection ...
// std::vector<CFringeSegment> fringes = /* ... */;
//
// WavefrontFromIsolines wf;
// WavefrontFromIsolines::Params p;
// p.cgTol = 1e-6;
// p.cgIter = 2000;
// p.outWidth = 512;  // Output: 512 pixels wide
// p.outHeight = 0;   // Auto-calculate height to maintain aspect ratio
//
// Eigen::MatrixXd topogram = wf.solve(fringes, shapes, mask, p);
// std::cout << topogram << std::endl;
//
