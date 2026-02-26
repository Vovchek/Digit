#pragma once

#include <Eigen/Sparse>
#include "DigitMode/IsoLinesToTopogram.h"

class GradientPoissonStrategy :
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
        const int W = mask.width;
        const int H = mask.height;
        const int N = W * H;

        Eigen::VectorXd gx = Eigen::VectorXd::Zero(N);
        Eigen::VectorXd gy = Eigen::VectorXd::Zero(N);

        WavefrontFromIsolines::buildMask(mask);
        WavefrontFromIsolines::rasterizeFringes(fringes, mask);

        estimateGradient(fringsToPixels(fringes),
            gx, gy, visible, W, H);

        Eigen::VectorXd rhs = divergence(gx, gy, visible, W, H);

        std::vector<char> none(N, 0);

        return poisson(L, none, rhs, p, mask);
    }
    void estimateGradient(
        const std::vector<std::vector<Eigen::Vector2i>>& pix,
        Eigen::VectorXd& gx,
        Eigen::VectorXd& gy,
        const std::vector<char>& visible,
        int W, int H)
    {
        for (auto& line : pix)
            for (size_t i = 1; i < line.size() - 1; i++)
            {
                auto p0 = line[i - 1];
                auto p1 = line[i];
                auto p2 = line[i + 1];

                Eigen::Vector2d t = (p2 - p0).cast<double>();
                if (t.norm() < 1e-3) continue;
                t.normalize();

                Eigen::Vector2d n(-t.y(), t.x());

                int id = p1.y() * W + p1.x();
                if (!visible[id]) continue;

                double mag = 1.0; // fringe spacing scaling
                gx[id] = mag * n.x();
                gy[id] = mag * n.y();
            }
    }
    Eigen::VectorXd divergence(
        const Eigen::VectorXd& gx,
        const Eigen::VectorXd& gy,
        const std::vector<char>& visible,
        int W, int H)
    {
        Eigen::VectorXd d =
            Eigen::VectorXd::Zero(W * H);

        auto id = [&](int x, int y)
            {return y * W + x; };

        for (int y = 1; y < H - 1; y++)
            for (int x = 1; x < W - 1; x++)
            {
                int i = id(x, y);
                if (!visible[i]) continue;

                d[i] =
                    (gx[id(x + 1, y)] - gx[id(x - 1, y)]) * 0.5 +
                    (gy[id(x, y + 1)] - gy[id(x, y - 1)]) * 0.5;
            }
        return d;
    }
};
