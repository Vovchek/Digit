#pragma once

#include <Eigen/Sparse>
#include <vector>
#include <iostream>
#include <iomanip>
#include "ApertureCore/include/aperturecore/visibility/VisibilityMask.h"
#include "ApertureCore/include/aperturecore/visibility/ShapeCollection.h"
#include "DigitMode/CFringeSegment.h"

/**
 * @brief Wavefront reconstruction from fringe contours
 * 
 * Reconstructs a height map (topogram) from fringe line contours
 * using Poisson equation solving over a visibility mask.
 * Output resolution is determined by aperture bounding box and requested resolution.
 */
class WavefrontFromIsolines
{
public:

	struct Params
	{
		double nanValue = std::numeric_limits<double>::quiet_NaN();
		double cgTol    = 1e-6;
		int    cgIter   = 2000;
		
		// Resolution parameters
		// outWidth: desired output matrix width (columns along x-axis)
		// outWidth = 0 means use aperture bounding box width
		int outWidth = 0;
		// outHeight: desired output matrix height (rows along y-axis)
		// outHeight = 0 means calculate from outWidth to maintain aspect ratio
		int outHeight = 0;
	};

	/**
	 * @brief Solve for height map from fringes within aperture bounds
	 * @param fringes Vector of CFringeSegment objects (fringe number used as height)
	 * @param shapes ShapeCollection containing aperture shapes
	 * @param mask Visibility mask defining aperture region
	 * @param p Solver parameters (includes resolution)
	 * @return Height map (topogram) at specified resolution - NaN outside aperture
	 */
	Eigen::MatrixXd solve(
		const std::vector<CFringeSegment>& fringes,
		const aperture::ShapeCollection& shapes,
		const aperture::visibility::VisibilityMask& mask,
		const Params& p);

	/**
	 * @brief Serialize topogram matrix to output stream
	 * 
	 * Format: rows x columns matrix with NaN-aware formatting
	 * @param os Output stream
	 * @param matrix Matrix to serialize
	 * @return Output stream for chaining
	 */
	friend std::ostream& operator<<(std::ostream& os, const Eigen::MatrixXd& matrix);

private:

	typedef Eigen::SparseMatrix<double> SpMat;
	typedef Eigen::Triplet<double> T;

	std::vector<char> visible;
	std::vector<char> knownZ;
	std::vector<char> knownW;
	Eigen::VectorXd   zk;

	// ========================================================================
	// Output Resampling (Bilinear interpolation)
	// ========================================================================

	/**
	 * @brief Resample topogram to desired output resolution
	 * Performs bilinear interpolation from original to target resolution
	 * @param original Original topogram at mask resolution
	 * @param apertureBounds Bounding rect of apertures (world coords)
	 * @param outWidth Target width in pixels
	 * @param outHeight Target height in pixels
	 * @param p Solver parameters
	 * @return Resampled topogram at (outHeight x outWidth)
	 */
	Eigen::MatrixXd resampleToResolution(
		const Eigen::MatrixXd& original,
		const aperture::Bounds& apertureBounds,
		int outWidth,
		int outHeight,
		const Params& p);

	/**
	 * @brief Bilinear interpolation helper
	 * @param image Source image (at original resolution)
	 * @param x World x-coordinate
	 * @param y World y-coordinate
	 * @param minX Left edge of world coords
	 * @param minY Top edge of world coords
	 * @param p Params (for nanValue)
	 * @return Interpolated value (or NaN if outside bounds)
	 */
	double bilinearInterpolate(
		const Eigen::MatrixXd& image,
		double x, double y,
		double minX, double minY,
		const Params& p);

	// ========================================================================
	// Mask Builder
	// ========================================================================

	void buildMask(const aperture::visibility::VisibilityMask& mask);

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
		double h, const aperture::visibility::VisibilityMask& mask);

	/**
	 * @brief Rasterize all fringe contours
	 * Extracts point sequences from each fringe and draws them
	 */
	void rasterizeFringes(
		const std::vector<CFringeSegment>& fringes,
		const aperture::visibility::VisibilityMask& mask);

	// ========================================================================
	// Laplacian Builder (auto-Neumann on aperture edge)
	// ========================================================================

	SpMat buildMaskedL(const aperture::visibility::VisibilityMask& mask);

	// ========================================================================
	// Poisson Solver (Conjugate Gradient)
	// ========================================================================

	Eigen::VectorXd solvePoisson(
		SpMat& L,
		const std::vector<char>& known,
		const Eigen::VectorXd& rhs,
		const Params& p,
		const aperture::visibility::VisibilityMask& mask);

	// ========================================================================
	// Output Conversion
	// ========================================================================

	Eigen::MatrixXd toImage(
		const Eigen::VectorXd& z,
		const aperture::visibility::VisibilityMask& mask,
		const Params& p);

	// ========================================================================
	// Index Helper
	// ========================================================================

	inline int id(int x, int y, const aperture::visibility::VisibilityMask& mask) const
	{
		return y * mask.width + x;
	}
};
