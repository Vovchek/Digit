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
	double centerX = (bounds_.minX() + bounds_.maxX()) / 2.0;
	double centerY = (bounds_.minY() + bounds_.maxY()) / 2.0;
	double halfWidth = bounds_.width() / 2.0;
	double halfHeight = bounds_.height() / 2.0;
	double maxScale = (std::max)(halfWidth, halfHeight);

	if (maxScale == 0.0) maxScale = 1.0;

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
		double y_phys = bounds_.minY() + (row + 0.5) * bounds_.height() / rows_;
		double y_norm = (y_phys - centerY) / maxScale;

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
			double x_phys = bounds_.minX() + (col + 0.5) * bounds_.width() / cols_;
			double x_norm = (x_phys - centerX) / maxScale;

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
