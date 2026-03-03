#include "DigitMode/WavefrontSolver/WavefrontFromContours.h"

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

			// Map world coordinates to output pixel coordinates
			double u0 = (p0.x - input_.bounds_.minX()) / input_.bounds_.width();
			double v0 = (p0.y - input_.bounds_.minY()) / input_.bounds_.height();
			double u1 = (p1.x - input_.bounds_.minX()) / input_.bounds_.width();
			double v1 = (p1.y - input_.bounds_.minY()) / input_.bounds_.height();

			// Convert to output pixel coordinates
			int px0 = static_cast<int>(u0 * outWidth);
			int py0 = static_cast<int>(v0 * outHeight);
			int px1 = static_cast<int>(u1 * outWidth);
			int py1 = static_cast<int>(v1 * outHeight);

			// Handle coordinate system conversion
			py0 = static_cast<int>(convertY(v0 * outHeight));
			py1 = static_cast<int>(convertY(v1 * outHeight));

			// Draw the line segment
			drawLine(px0, py0, px1, py1, fringeValue);
		}
	}

	return {knownZ, zk};
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

	// Write header
	os << "Size=" << rows_ << "\n";
	os << "[MATRIX]\n";

	// Calculate normalization parameters
	double ratio = 2.0 / ((std::max)(rows_, cols_) - 1);  // Scale factor to fit largest dimension into [-1, 1]

	// Format settings
	const int pairsPerLine = 6;
	const std::string indent(7, ' ');  // 7 spaces to align with first Z value

	std::ios_base::fmtflags oldFlags = os.flags();
	std::streamsize oldPrec = os.precision();
	os << std::fixed << std::setprecision(4);

	// Iterate through rows
	for (int row = 0; row < rows_; ++row)
	{
		// Calculate Y coordinate at the center of this row
		double y_norm = row  * ratio - 1.0;  // Normalize to [-1, 1]

		bool firstLineOfRow = true;
		int pairCount = 0;

		// Write all columns for this row
		for (int col = 0; col < cols_; ++col)
		{
			int idx = row * cols_ + col;
			double z = data_[idx];
			if (std::isnan(z) || std::isinf(z))
				continue;  // Skip NaN/Inf values entirely for MTR output

			// Calculate X coordinate at the center of this column
			double x_norm = col * ratio - 1.0;  // Normalize to [-1, 1]

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

			// Move to next text line after 6 pairs (if more columns remain)
			if (pairCount == pairsPerLine && col < cols_ - 1)
			{
				pairCount = 0;
			}
		}

		// End of matrix row with E tag
		os << " E\n";
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

std::vector<WavefrontFromContoursSolver_HorizontalLinear::FringeCrossing>
WavefrontFromContoursSolver_HorizontalLinear::findFringeCrossings(
	const WavefrontFromContoursContext& ctx,
	double worldY) const
{
	std::vector<FringeCrossing> crossings;
	const auto& fringeSegments = ctx.input_.fringeSegments_;
	
	// Iterate through all fringe segments
	for (const auto& fringe : fringeSegments)
	{
		double fringeValue = fringe.GetNumber();
		int pointCount = fringe.GetPointCount();
		
		// Check each line segment in the fringe polyline
		for (int i = 0; i < pointCount - 1; ++i)
		{
			CDPoint p0 = fringe.GetPoint(i);
			CDPoint p1 = fringe.GetPoint(i + 1);
			
			double y0 = p0.y;
			double y1 = p1.y;
			
			// Check if horizontal line at worldY crosses this segment
			// Segment crosses if worldY is between y0 and y1 (exclusive endpoints to avoid duplicates)
			if ((y0 < worldY && worldY < y1) || (y1 < worldY && worldY < y0))
			{
				// Linear interpolation to find X coordinate at crossing
				double t = (worldY - y0) / (y1 - y0);
				double crossX = p0.x + t * (p1.x - p0.x);
				
				crossings.push_back({crossX, fringeValue});
			}
			// Include endpoint if exactly on the line (but only once per point)
			else if (i == 0 && std::abs(y0 - worldY) < 1e-10)
			{
				crossings.push_back({p0.x, fringeValue});
			}
			else if (i == pointCount - 2 && std::abs(y1 - worldY) < 1e-10)
			{
				crossings.push_back({p1.x, fringeValue});
			}
		}
	}
	
	// Sort crossings by X coordinate
	std::sort(crossings.begin(), crossings.end());
	
	return crossings;
}

double WavefrontFromContoursSolver_HorizontalLinear::interpolateAtX(
	const std::vector<FringeCrossing>& crossings,
	double worldX) const
{
	if (crossings.empty())
		return std::numeric_limits<double>::quiet_NaN();
	
	// Find the two crossings that bracket worldX
	// If worldX is before first crossing or after last, extrapolate or return NaN
	
	// Find first crossing at or after worldX
	auto it = std::lower_bound(crossings.begin(), crossings.end(), 
		FringeCrossing{worldX, 0.0});
	
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
	const FringeCrossing& right = *it;
	const FringeCrossing& left = *(it - 1);
	
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
		double v = (row + 0.5) / outHeight;
		double worldY = bounds.minY() + v * bounds.height();
		
		// Find all fringe crossings at this Y
		auto crossings = findFringeCrossings(ctx, worldY);
		
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
			double u = (col + 0.5) / outWidth;
			double worldX = bounds.minX() + u * bounds.width();
			
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

	return result;
}

// ============================================================================
// WavefrontFromContoursSolver_HorizontalSpline Implementation
// ============================================================================

std::vector<WavefrontFromContoursSolver_HorizontalSpline::FringeCrossing>
WavefrontFromContoursSolver_HorizontalSpline::findFringeCrossings(
	const WavefrontFromContoursContext& ctx,
	double worldY) const
{
	std::vector<FringeCrossing> crossings;
	const auto& fringeSegments = ctx.input_.fringeSegments_;
	
	// Iterate through all fringe segments
	for (const auto& fringe : fringeSegments)
	{
		double fringeValue = fringe.GetNumber();
		int pointCount = fringe.GetPointCount();
		
		// Check each line segment in the fringe polyline
		for (int i = 0; i < pointCount - 1; ++i)
		{
			CDPoint p0 = fringe.GetPoint(i);
			CDPoint p1 = fringe.GetPoint(i + 1);
			
			double y0 = p0.y;
			double y1 = p1.y;
			
			// Check if horizontal line at worldY crosses this segment
			if ((y0 < worldY && worldY < y1) || (y1 < worldY && worldY < y0))
			{
				// Linear interpolation to find X coordinate at crossing
				double t = (worldY - y0) / (y1 - y0);
				double crossX = p0.x + t * (p1.x - p0.x);
				
				crossings.push_back({crossX, fringeValue});
			}
			// Include endpoint if exactly on the line (but only once per point)
			else if (i == 0 && std::abs(y0 - worldY) < 1e-10)
			{
				crossings.push_back({p0.x, fringeValue});
			}
			else if (i == pointCount - 2 && std::abs(y1 - worldY) < 1e-10)
			{
				crossings.push_back({p1.x, fringeValue});
			}
		}
	}
	
	// Sort crossings by X coordinate
	std::sort(crossings.begin(), crossings.end());
	
	return crossings;
}

WavefrontFromContoursSolver_HorizontalSpline::SplineCoefficients
WavefrontFromContoursSolver_HorizontalSpline::buildSpline(
	const std::vector<FringeCrossing>& crossings) const
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
	const std::vector<FringeCrossing>& crossings,
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
		double v = (row + 0.5) / outHeight;
		double worldY = bounds.minY() + v * bounds.height();
		
		// Find all fringe crossings at this Y
		auto crossings = findFringeCrossings(ctx, worldY);
		
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
			double u = (col + 0.5) / outWidth;
			double worldX = bounds.minX() + u * bounds.width();
			
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
	
	return result;
}
