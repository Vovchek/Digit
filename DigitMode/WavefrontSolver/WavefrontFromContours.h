#pragma once

#include "aperturecore/include/aperturecore/geometry/Bounds.h"
#include "aperturecore/include/aperturecore/visibility/VisibilityMask.h"
#include "aperturecore/include/aperturecore/geometry/CoordinateSystem.h"
#include "DigitMode/CFringeSegment.h"
#include <iomanip>
#include <limits>
#include <fstream>

// Save current macro state and undefine conflicting MFC macros for Eigen
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

#include <Eigen/Dense>
#include <Eigen/Sparse>

// Restore original macro state
#pragma pop_macro("max")
#pragma pop_macro("min")

struct WavefrontFromContoursInput
{
    explicit WavefrontFromContoursInput(
        const aperture::Bounds& bounds,
        const aperture::visibility::VisibilityMask& visibilityMask,
        const std::vector<CFringeSegment>& fringeSegments,
        int outWidth = 0,
        int outHeight = 0,
        aperture::CoordinateSystemType inputCoordType = aperture::CoordinateSystemType::SCREEN,
        aperture::CoordinateSystemType outputCoordType = aperture::CoordinateSystemType::MATH)
        : bounds_(bounds)
        , visibilityMask_(visibilityMask)
        , fringeSegments_(fringeSegments)
        , outWidth_(outWidth > 0 ? outWidth : visibilityMask.width)
        , outHeight_(outHeight > 0 ? outHeight : 
            static_cast<int>(visibilityMask.height * outWidth_ / static_cast<double>(visibilityMask.width)))
        , inputCoordType_(inputCoordType)
		, outputCoordType_(outputCoordType) {
	}

    const aperture::Bounds bounds_;
    const aperture::visibility::VisibilityMask& visibilityMask_;
    const std::vector<CFringeSegment>& fringeSegments_;
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
public:
    explicit WavefrontFromContoursContext(const WavefrontFromContoursInput& input) :
        input_(input) {}
	// solver helpers
	// rasterize to output resolution, considering visibility mask, bounds, input & output coordinate systems
	std::vector<char> buildMask() const
	{
		const auto& visibilityMask = input_.visibilityMask_;
		
		// Use pre-resolved output dimensions from constructor
		int outWidth = input_.outWidth_;
		int outHeight = input_.outHeight_;

		// Build output mask with output dimensions
		const int N = outHeight * outWidth;
		std::vector<char> visible(N, 0);

		// Create coordinate systems for conversion
		aperture::CoordinateSystem inputSys = (input_.inputCoordType_ == aperture::CoordinateSystemType::SCREEN) 
			? aperture::CoordinateSystem::screen(visibilityMask.height)
			: aperture::CoordinateSystem::math(visibilityMask.height);
		
		aperture::CoordinateSystem outputSys = (input_.outputCoordType_ == aperture::CoordinateSystemType::SCREEN)
			? aperture::CoordinateSystem::screen(outHeight)
			: aperture::CoordinateSystem::math(outHeight);

		for (int outY = 0; outY < outHeight; ++outY) {
			for (int outX = 0; outX < outWidth; ++outX) {
				// Map output pixel to visibility mask coordinate space
				double maskX = outX * visibilityMask.width / static_cast<double>(outWidth);
				double maskY = outY * visibilityMask.height / static_cast<double>(outHeight);

				// Handle coordinate system conversion
				double finalY = convertY(maskY);

				// Clamp to visibility mask bounds
				int x = static_cast<int>(maskX);
				int y = static_cast<int>(finalY);
				
				if (x >= 0 && x < visibilityMask.width && y >= 0 && y < visibilityMask.height) {
					int outIndex = outY * outWidth + outX;
					visible[outIndex] = visibilityMask.IsVisible(x, y) ? 1 : 0;
				}
			}
		}

		return visible;
	}

	std::pair<std::vector<char>, std::vector<double>> 
		rasterize(const std::vector<char>& mask) const;

	// Helper method to convert Y coordinate between reference systems
	double convertY(double y) const
	{
		// If coordinate systems match, no conversion needed
		if (input_.inputCoordType_ == input_.outputCoordType_) {
			return y;
		}

		// Create coordinate systems for conversion
		aperture::CoordinateSystem inputSys = (input_.inputCoordType_ == aperture::CoordinateSystemType::SCREEN)
			? aperture::CoordinateSystem::screen(input_.visibilityMask_.height)
			: aperture::CoordinateSystem::math(input_.visibilityMask_.height);
		
		aperture::CoordinateSystem outputSys = (input_.outputCoordType_ == aperture::CoordinateSystemType::SCREEN)
			? aperture::CoordinateSystem::screen(input_.outHeight_)
			: aperture::CoordinateSystem::math(input_.outHeight_);

		// Convert Y coordinate between coordinate systems
		return inputSys.convertY(y, outputSys);
	}

	const WavefrontFromContoursInput input_;
};

class WavefrontFromContoursResult
{
public:
	bool saveAsImage(const std::string& filename) const {return false;};
	bool saveAsMTR(const std::string& filename) const {return false;};

	// ============================================================================
	// Stream Serialization
	// ============================================================================

	std::ostream& operator<<(std::ostream& os)
	{
		os << "MatrixXd(" << matrix_.rows() << " x " << matrix_.cols() << ")\n";

		// Set formatting for floating point
		std::streamsize prevPrecision = os.precision();
		os.precision(6);
		os << std::fixed;

		for (int y = 0; y < matrix_.rows(); ++y)
		{
			for (int x = 0; x < matrix_.cols(); ++x)
			{
				double val = matrix_(y, x);

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

				if (x < matrix_.cols() - 1)
					os << " ";
			}
			os << "\n";
		}

		// Restore formatting
		os.precision(prevPrecision);
		os.unsetf(std::ios_base::fixed);

		return os;
	}

private:
    Eigen::MatrixXd matrix_;
    aperture::Bounds bounds_;
    aperture::CoordinateSystemType coordType_;

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
	WavefrontFromContoursResult run(const IWavefrontFromContoursSolver& solver)
    {
        return solver.solve(context_);
    }

private:
    WavefrontFromContoursContext context_;
};

// Helper function to access context parameters for solvers
inline void getContextDimensions(const WavefrontFromContoursContext& ctx, int& outWidth, int& outHeight)
{
	// Output dimensions are already resolved in constructor
	outWidth = ctx.input_.outWidth_;
	outHeight = ctx.input_.outHeight_;
}

// concreate solvers (e.g., Poisson, Bilinear, etc.) 
class WavefrontFromContoursSolver_Bilinear : public IWavefrontFromContoursSolver
{
public:
	WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override
	{
		// Placeholder implementation - replace with actual bilinear solver logic
		WavefrontFromContoursResult result;

		auto mask = ctx.buildMask();
		auto rasterized = ctx.rasterize(mask);

		int outWidth = 0, outHeight = 0;
		getContextDimensions(ctx, outWidth, outHeight);
		Eigen::Map<const Eigen::MatrixXd> eigenRasterized(rasterized.data(), outHeight, outWidth);

		// ... perform bilinear interpolation based on ctx.input_ ...
		return result;
	}
};
