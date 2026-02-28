#include "DigitMode/WavefrontSolver/WavefrontFromContours.h"

std::pair<std::vector<char>, std::vector<double>>
WavefrontFromContoursContext::rasterize(const std::vector<char>& mask) const
{
	const auto& visibilityMask = input_.visibilityMask_;
	
	// Use pre-resolved output dimensions from constructor
	int outWidth = input_.outWidth_;
	int outHeight = input_.outHeight_;

	// Allocate result with output dimensions
	std::vector<double> zk(outHeight * outWidth, std::numeric_limits<double>::quiet_NaN());
	std::vector<char> knownZ(outHeight * outWidth, 0);

	// Fill output pixels where visibility mask has corresponding visible pixels
	for (int outY = 0; outY < outHeight; ++outY) {
		for (int outX = 0; outX < outWidth; ++outX) {
			int outIndex = outY * outWidth + outX;

			// Map output pixel to visibility mask coordinate space
			double maskX = outX * visibilityMask.width / static_cast<double>(outWidth);
			double maskY = outY * visibilityMask.height / static_cast<double>(outHeight);

			// Handle coordinate system conversion
			double finalY = convertY(maskY);

			// Clamp to visibility mask bounds
			int x = static_cast<int>(maskX);
			int y = static_cast<int>(finalY);

			// Check mask and bounds
			if (x >= 0 && x < visibilityMask.width && y >= 0 && y < visibilityMask.height) {
				int maskIndex = y * visibilityMask.width + x;
				if (maskIndex < static_cast<int>(mask.size()) && mask[maskIndex]) {
					zk[outIndex] = 0.0;  // Placeholder for actual computed value
				}
			}
		}
	}

	return {knownZ, zk};
}
