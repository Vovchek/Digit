/**
 * @file WavefrontFromContours.h
 * @brief Interface of classes for converting isoline fringes into a surface heights map
 * @author Vladimir N. Chekal
 * @see https://github.com/Vovchek
 */
#pragma once

#include <aperturecore/geometry/Bounds.h>
#include <aperturecore/visibility/VisibilityChecker.h>
#include <aperturecore/geometry/CoordinateSystem.h>
#include <aperturecore/visibility/ShapeCollection.h>
#include "FringeSegment.h"
#include <string>

// Undefine conflicting MFC macros so standard library min/max remain usable here.
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace isomap {

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
			const std::vector<FringeSegment>& fringeSegments,
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
			, outputCoordType_(outputCoordType) {}

		const aperture::ShapeCollection& shapeCollection_;
		const std::vector<FringeSegment>& fringeSegments_;
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
				input_.bounds_.minY() * yScale_ : -input_.outHeight_ + input_.bounds_.minY() * yScale_;
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
		bool saveAsImage([[maybe_unused]] const std::string& filename) const { return false; };
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

	};

	// True horizontal linear interpolation solver
	// For each row, finds fringe intersections and linearly interpolates between them
	class WavefrontFromContoursSolver_HorizontalLinear : public IWavefrontFromContoursSolver
	{
	public:
		WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

	};

	// Delaunay triangulation + De Casteljau (few other were tried) interpolation solver
	// Treats the fringe points as an irregular 2D mesh and interpolates height locally
	class WavefrontFromContoursSolver_Delaunay : public IWavefrontFromContoursSolver
	{
	public:
		enum class InterpolationMethod
		{
			DeCasteljau = 0,    // best choice for now, yet not really C1
			LocalQuadric = 1,   // promising, mostly C1 but unstable, need exploring/fixing
			AkimaBivariate = 2, // broken or invalid
			Barycentric = 3,    // most simple, stable C0
			IDW = 4				// broken of invalid
			// Clough-Tocher = 5, // stated as C1, but my code gliched, failed to find correct formulas (Farin, "Curves and Surfaces for CAGD"?)
			// Powell-Sabin = 6, // stated as C1, did not try yet
		};
		explicit WavefrontFromContoursSolver_Delaunay(InterpolationMethod method = InterpolationMethod::DeCasteljau) :
			method_(method) {}
		WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

	private:
		InterpolationMethod method_ = InterpolationMethod::DeCasteljau;
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

} // namespace isomap