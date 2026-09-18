#pragma once

#include "aperturecore/include/aperturecore/geometry/Bounds.h"
#include "aperturecore/include/aperturecore/visibility/VisibilityChecker.h"
#include "aperturecore/include/aperturecore/geometry/CoordinateSystem.h"
#include "aperturecore/include/aperturecore/visibility/ShapeCollection.h"
#include "DigitMode/CFringeSegment.h"
#include <string>

// Undefine conflicting MFC macros so standard library min/max remain usable here.
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

struct XyzSample
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};

struct WavefrontFromContoursInput
{
    explicit WavefrontFromContoursInput(
        const aperture::ShapeCollection& shapeCollection,
        const std::vector<CFringeSegment>& fringeSegments,
		double scaleFactor = 1.,
		double fiScan = 0.,
        int outWidth = 0,
        int outHeight = 0,
        aperture::CoordinateSystemType inputCoordType = aperture::CoordinateSystemType::SCREEN,
        aperture::CoordinateSystemType outputCoordType = aperture::CoordinateSystemType::MATH)
        : shapeCollection_(shapeCollection)
        , bounds_(shapeCollection.getVisibleRegion())
        , fringeSegments_(fringeSegments)
		, scaleFactor_(scaleFactor)
		, fiScan_(fiScan)
        , outWidth_(outWidth > 0 ? outWidth : static_cast<int>(bounds_.width()))
        , outHeight_(outHeight > 0 ? outHeight : 
            static_cast<int>(bounds_.height() * outWidth_ / static_cast<double>(bounds_.width())))
        , inputCoordType_(inputCoordType)
		, outputCoordType_(outputCoordType) {
	}

    const aperture::ShapeCollection& shapeCollection_;
    const std::vector<CFringeSegment>& fringeSegments_;
	double scaleFactor_;
	double fiScan_;
	// ROI in input coordinate system, cached from shapeCollection_.getVisibleRegion()
	const aperture::Bounds bounds_;
	// Resolution parameters
	// outWidth: desired output matrix width (columns along x-axis)
    // outWidth = 0 means use aperture bounding box width (resolved in constructor)
    int outWidth_;
    // outHeight: desired output matrix height (rows along y-axis)
    // outHeight = 0 means calculate from outWidth to maintain aspect ratio (resolved in constructor)
    int outHeight_;
    aperture::CoordinateSystemType inputCoordType_{ aperture::CoordinateSystemType::SCREEN };
    aperture::CoordinateSystemType outputCoordType_{ aperture::CoordinateSystemType::MATH };
};

class WavefrontFromContoursContext
{
	double xShift_ = 0.0;
	double yShift_ = 0.0;
	double xScale_ = 1.0;
	double yScale_ = 1.0;

public:
	// Helper structure to store fringe crossing point
	struct FringeCrossing
	{
		double x;           // X-coordinate where fringe crosses horizontal line
		double fringeValue; // Fringe number at this crossing
		
		bool operator<(const FringeCrossing& other) const
		{
			return x < other.x;
		}
	};

    explicit WavefrontFromContoursContext(const WavefrontFromContoursInput& input) :
        input_(input)
	{
		xScale_ = static_cast<double>(input_.outWidth_) / input_.bounds_.width();
		xShift_ = input_.bounds_.minX();
		yScale_ = (input_.inputCoordType_ == input_.outputCoordType_) ?
			static_cast<double>(input_.outHeight_) / input_.bounds_.height() :
			-static_cast<double>(input_.outHeight_) / input_.bounds_.height();
		yShift_ = (input_.inputCoordType_ == input_.outputCoordType_) ?
			input_.bounds_.minY() * yScale_ : - input_.outHeight_ + input_.bounds_.minY() * yScale_;
	}
	
	// solver helpers
	double xToOutput(double x) const
	{
		return (x * xScale_ - xShift_);
	}
	double yToOutput(double y) const
	{
		return (y * yScale_ - yShift_);
	}
	double xToInput(double x) const
	{
		return (x + xShift_) / xScale_;
	}
	double yToInput(double y) const
	{
		return (y + yShift_) / yScale_;
	}

	// rasterize to output resolution, considering visibility mask, bounds, input & output coordinate systems
	std::vector<char> buildMask() const
	{
		aperture::VisibilityChecker checker(input_.shapeCollection_);  // <-- New

		// Use pre-resolved output dimensions from constructor
		const int outWidth = input_.outWidth_;
		const int outHeight = input_.outHeight_;

		// Build output mask with output dimensions
		const int N = outHeight * outWidth;
		std::vector<char> visible(N, 0);

		size_t sumVisible = 0;
		size_t sumVisibleLine = 0;

		for (int outY = 0; outY < outHeight; ++outY) {
			const double maskY = yToInput(outY);
			for (int outX = 0; outX < outWidth; ++outX) {
				// Map output pixel to visibility mask coordinate space
				const double maskX = xToInput(outX);
				const int outIndex = outY * outWidth + outX;

				visible[outIndex] = checker.isVisible({ maskX, maskY }) ? 1 : 0;
				sumVisibleLine += visible[outIndex];
			}
			if (sumVisible && sumVisibleLine == 0) {
				TRACE("Warning: No visible pixels found in line %d\n", outY);
			}
			sumVisible += sumVisibleLine;
			sumVisibleLine = 0;
		}

		return visible;
	}

	std::pair<std::vector<char>, std::vector<double>> 
		rasterize(const std::vector<char>& mask) const;

	aperture::BoundingCircle computeBoundingCircle() const;

	// Find all points where fringes cross a horizontal line at given Y
	// Returns sorted vector of crossings
	std::vector<FringeCrossing> findFringeCrossings(double worldY) const;

	const WavefrontFromContoursInput input_;
};

class WavefrontFromContoursResult
{
public:

	// ============================================================================
	// Getters
	// ============================================================================

	const std::vector<double>& getData() const { return data_; }
	int getRows() const { return rows_; }
	int getCols() const { return cols_; }
	//const aperture::Bounds& getBounds() const { return bounds_; }
	aperture::CoordinateSystemType getCoordinateSystem() const { return coordType_; }
	double getScaleFactor() const { return scaleFactor_; }
	double getFiScan() const { return fiScan_; }
	const aperture::BoundingCircle& getBoundingCircle() const { return boundingCircle_; }
	const std::string& getTitle() const { return title_; }

	// ============================================================================
	// Setters
	// ============================================================================

	void setMatrixData(const double* data, int rows, int cols);
	//void setBounds(const aperture::Bounds& bounds) { bounds_ = bounds; }
	void setCoordinateSystem(aperture::CoordinateSystemType coordType) { coordType_ = coordType; }
	void setScaleFactor(double scale) { scaleFactor_ = scale; }
	void setFiScan(double fi) { fiScan_ = fi; }
	void setBoundingCircle(const aperture::BoundingCircle& circle) { boundingCircle_ = circle; }
	void setTitle(const std::string& title) { title_ = title; }

	// ============================================================================
	// Stream Serialization
	// ============================================================================

	friend std::ostream& operator<<(std::ostream& os, const WavefrontFromContoursResult& result);
	bool saveAsImage(const std::string& filename) const { return false; };
	bool saveMtrMatrix(std::ostream& os) const;

private:
	std::string title_;
    std::vector<double> data_;
    int rows_ = 0;
    int cols_ = 0;
    //aperture::Bounds bounds_;
    aperture::CoordinateSystemType coordType_;
	double scaleFactor_ = 1.0;
	double fiScan_ = 0.0;
	aperture::BoundingCircle boundingCircle_{};

};

class IWavefrontFromContoursSolver
{
public:
    virtual ~IWavefrontFromContoursSolver() = default;
    virtual WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const = 0;
};

// High-Level Facade (User API)
class WavefrontFromContours
{
public:
	explicit WavefrontFromContours(WavefrontFromContoursInput input) :
        context_(input) {}
	WavefrontFromContoursResult run(const IWavefrontFromContoursSolver& solver) const
    {
        return solver.solve(context_);
    }

private:
    WavefrontFromContoursContext context_;
};

// concrete solvers 
class WavefrontFromContoursSolver_Bilinear : public IWavefrontFromContoursSolver
{
public:
	WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

private:
	// Bilinear interpolation to fill unknown values using known neighbors
	// Only interpolates pixels where knownZ[idx]==0 && mask[idx]!=0
	// Updates knownZ as values are interpolated
	static void performBilinearInterpolation(
		std::vector<double>& zk,
		std::vector<char>& knownZ,
		const std::vector<char>& mask,
		int rows, int cols)
	{
		const int maxPasses = 20;

		auto idx = [&](int x, int y) { return y * cols + x; };

		for (int pass = 0; pass < maxPasses; ++pass)
		{
			bool changed = false;

			std::vector<double> newZ = zk;
			std::vector<char>   newKnown = knownZ;

			for (int y = 0; y < rows; ++y)
			{
				for (int x = 0; x < cols; ++x)
				{
					int i = idx(x, y);

					if (knownZ[i] || !mask[i])
						continue;

					// --- Find horizontal bracket
					int xl = x - 1;
					while (xl >= 0 && (!knownZ[idx(xl, y)] || !mask[idx(xl, y)]))
						--xl;

					int xr = x + 1;
					while (xr < cols && (!knownZ[idx(xr, y)] || !mask[idx(xr, y)]))
						++xr;

					bool hasX = (xl >= 0 && xr < cols);

					// --- Find vertical bracket
					int yt = y - 1;
					while (yt >= 0 && (!knownZ[idx(x, yt)] || !mask[idx(x, yt)]))
						--yt;

					int yb = y + 1;
					while (yb < rows && (!knownZ[idx(x, yb)] || !mask[idx(x, yb)]))
						++yb;

					bool hasY = (yt >= 0 && yb < rows);

					if (!hasX && !hasY)
						continue;

					double val = 0.0;
					int    used = 0;

					if (hasX)
					{
						double zl = zk[idx(xl, y)];
						double zr = zk[idx(xr, y)];
						double t = double(x - xl) / double(xr - xl);
						val += zl * (1.0 - t) + zr * t;
						used++;
					}

					if (hasY)
					{
						double zt = zk[idx(x, yt)];
						double zb = zk[idx(x, yb)];
						double t = double(y - yt) / double(yb - yt);
						val += zt * (1.0 - t) + zb * t;
						used++;
					}

					newZ[i] = val / used;
					newKnown[i] = 1;
					changed = true;
				}
			}

			zk.swap(newZ);
			knownZ.swap(newKnown);

			if (!changed)
				break;
		}
	}
};

// True horizontal linear interpolation solver
// For each row, finds fringe intersections and linearly interpolates between them
class WavefrontFromContoursSolver_HorizontalLinear : public IWavefrontFromContoursSolver
{
public:
	WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

private:
	// Linearly interpolate Z value at worldX given sorted crossings
	// Returns NaN if outside fringe coverage or no valid interpolation
	double interpolateAtX(
		const std::vector<WavefrontFromContoursContext::FringeCrossing>& crossings,
		double worldX) const;
};

// Delaunay triangulation + De Casteljau (few other were tried) interpolation solver
// Treats the fringe points as an irregular 2D mesh and interpolates height locally
class WavefrontFromContoursSolver_Delaunay : public IWavefrontFromContoursSolver
{
public:
	WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

private:
	std::vector<XyzSample> prepareSamples(const WavefrontFromContoursContext& ctx) const;
};

// Horizontal cubic spline interpolation solver
// For each row, finds fringe intersections and uses cubic spline for smooth interpolation
class WavefrontFromContoursSolver_HorizontalSpline : public IWavefrontFromContoursSolver
{
public:
	WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

private:
	// Cubic spline interpolation
	// Builds natural cubic spline coefficients for given crossings
	// Returns interpolated Z value at worldX
	double interpolateAtX(
		const std::vector<WavefrontFromContoursContext::FringeCrossing>& crossings,
		double worldX) const;
	
	// Helper to compute cubic spline coefficients (natural spline)
	// Input: n points (x[i], y[i]) sorted by x
	// Output: coefficients for cubic polynomials between points
	struct SplineCoefficients
	{
		std::vector<double> a, b, c, d; // Coefficients for each segment
		std::vector<double> x;          // X coordinates of knots
		
		// Evaluate spline at given x
		double evaluate(double xi) const;
	};
	
	SplineCoefficients buildSpline(const std::vector<WavefrontFromContoursContext::FringeCrossing>& crossings) const;
};
