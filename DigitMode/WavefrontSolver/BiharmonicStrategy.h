#pragma once

#include "DigitMode/IsolinesToTopogram.h"

class BiharmonicStrategy :
	public IReconstructionStrategy
{
public:

	Eigen::VectorXd reconstruct(
		WavefrontFromIsolines::SpMat& L,
		const std::vector<char>& visible,
		const std::vector<char>& knownZ,
		const Eigen::VectorXd& zk,
		const std::vector<CFringeSegment>& fringes,
		const aperture::visibility::VisibilityMask& mask,
		const WavefrontFromIsolines::Params& p,
		std::function<Eigen::VectorXd(
			WavefrontFromIsolines::SpMat&,
			const std::vector<char>&,
			const Eigen::VectorXd&,
			const WavefrontFromIsolines::Params&,
			const aperture::visibility::VisibilityMask&)> poisson
	) override
	{
		// Get bounding rect from shapes using ShapeCollection API
		aperture::Bounds apertureBounds = shapes.getCombinedBounds();

	// Resolve output resolution (default to aperture bounds, maintaining aspect ratio)
	int outWidth = p.outWidth > 0 ? p.outWidth : static_cast<int>(apertureBounds.width());
	int outHeight = p.outHeight > 0 ? p.outHeight :
		static_cast<int>(apertureBounds.height() * outWidth / apertureBounds.width());

	const int N = mask.width * mask.height;

	WavefrontFromIsolines::buildMask(mask);
	WavefrontFromIsolines::rasterizeFringes(fringes, mask);

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

};
