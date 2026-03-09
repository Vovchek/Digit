#include "DigitMode/WavefrontSolver/WavefrontFromContours.h"
#include <algorithm>
#include <ctime>
#include <random>

namespace
{
	constexpr double kCircleEps = 1e-9;

	double sqr(double v)
	{
		return v * v;
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

// Save current macro state and undefine conflicting MFC macros for Eigen
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

// Restore original macro state
#pragma pop_macro("max")
#pragma pop_macro("min")

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
		double fringeValue = fringe.GetNumber();
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

	os << "ScaleFactor=" << getScaleFactor() << "\n";
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
			zk[idx] = z;
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
			zk[idx] = z;
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
