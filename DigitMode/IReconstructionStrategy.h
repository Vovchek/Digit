#pragma once

class IReconstructionStrategy
{
public:
    virtual ~IReconstructionStrategy() {}

    virtual Eigen::VectorXd reconstruct(
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
    ) = 0;
};
