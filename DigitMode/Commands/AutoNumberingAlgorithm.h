#pragma once

#include "DigitMode/CFringeSegment.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <map>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace DigitMode {

/**
 * @brief Fully automatic fringe numbering algorithm
 * 
 * Assigns consistent scalar "Number" values to fringe segments using topology
 * and adjacency constraints, given a set of trusted reference fringes.
 * 
 * Uses a constraint-based graph labeling solver that:
 * - Respects trusted fringes as hard constraints
 * - Infers numbers for all others via least-squares optimization
 * - Evaluates confidence and updates trusted set
 * 
 * @param fringes [i/o] Geometry + Number field to update
 * @param trustedFringeIndices [i] Indices of fringes with trusted Numbers
 * @param step [i] Scalar increment between adjacent isolines (e.g., 1.0)
 * @param confidenceThreshold [i] Min confidence [0,1] to accept inferred numbers
 * 
 * @return Vector of fringe indices considered trusted after processing
 *         (includes original trusted + newly validated fringes)
 * 
 * Deterministic, no UI interaction, undoable via command wrapper.
 */
std::vector<size_t> AutoNumberFringes(
    std::vector<CFringeSegment>& fringes,
    const std::vector<size_t>& trustedFringeIndices,
    double step,
    double confidenceThreshold
);

// ========== Internal structures and helpers (implementation details) ==========

/**
 * @brief Internal lightweight node representation for a fringe
 */
struct FringeNode {
    size_t index;           ///< Index into fringes[]
    double knownValue;      ///< Number if trusted
    bool isTrusted;         ///< True if from trustedFringeIndices
    double centroid_x;      ///< Centroid X coordinate
    double centroid_y;      ///< Centroid Y coordinate
    bool isClosed;          ///< True if fringe is closed curve
    double confidence;      ///< Confidence in inferred number [0,1]
};

/**
 * @brief Adjacency edge between two fringes
 */
struct AdjacencyEdge {
    size_t i, j;            ///< Node indices
    double weight;          ///< Edge confidence weight [0,1]
    int sign;               ///< +1, -1, or 0 (unknown)
    double distance;        ///< Minimum distance between fringes
    double overlapLength;   ///< Projected overlap length
};

// Default adjacency parameters (configurable)
struct AdjacencyParams {
    double maxDistance = 50.0;      ///< Maximum distance for adjacency
    double minOverlapLength = 10.0; ///< Minimum overlap to consider adjacent
    double maxTangentAngle = 30.0;  ///< Max angle deviation (degrees)
};

// ========== IMPLEMENTATION (header-only for compilation) ==========

namespace impl {

    // Phase 1: Preprocessing
    inline std::vector<FringeNode> PreprocessFringes(
        const std::vector<CFringeSegment>& fringes,
        const std::vector<size_t>& trustedFringeIndices,
        double step)
    {
        std::vector<FringeNode> nodes;
        nodes.reserve(fringes.size());

        std::map<size_t, bool> trustedMap;
        for (size_t idx : trustedFringeIndices) {
            trustedMap[idx] = true;
        }

        for (size_t i = 0; i < fringes.size(); ++i) {
            const auto& fringe = fringes[i];
            FringeNode node;
            node.index = i;
            node.isTrusted = (trustedMap.count(i) > 0);

            double sumX = 0.0, sumY = 0.0;
            int pointCount = fringe.GetPointCount();
            if (pointCount > 0) {
                for (int p = 0; p < pointCount; ++p) {
                    CDPoint pt = fringe.GetPoint(p);
                    sumX += pt.x;
                    sumY += pt.y;
                }
                node.centroid_x = sumX / pointCount;
                node.centroid_y = sumY / pointCount;
            } else {
                node.centroid_x = 0.0;
                node.centroid_y = 0.0;
            }

            node.isClosed = false;
            if (pointCount >= 3) {
                CDPoint first = fringe.GetPoint(0);
                CDPoint last = fringe.GetPoint(pointCount - 1);
                double dist = std::sqrt((first.x - last.x) * (first.x - last.x) +
                                       (first.y - last.y) * (first.y - last.y));
                node.isClosed = (dist < 5.0);
            }

            if (node.isTrusted) {
                double k = fringe.GetNumber() / step;
                node.knownValue = std::round(k);
            } else {
                node.knownValue = 0.0;
            }

            node.confidence = 0.0;
            nodes.push_back(node);
        }

        return nodes;
    }

    // Phase 2: Build adjacency graph
    inline std::vector<AdjacencyEdge> BuildAdjacencyGraph(
        const std::vector<CFringeSegment>& fringes,
        const std::vector<FringeNode>& nodes,
        const AdjacencyParams& params)
    {
        std::vector<AdjacencyEdge> edges;

        for (size_t i = 0; i < nodes.size(); ++i) {
            for (size_t j = i + 1; j < nodes.size(); ++j) {
                const auto& ni = nodes[i];
                const auto& nj = nodes[j];

                double dx = ni.centroid_x - nj.centroid_x;
                double dy = ni.centroid_y - nj.centroid_y;
                double dist = std::sqrt(dx * dx + dy * dy);

                if (dist > params.maxDistance) {
                    continue;
                }

                const auto& fi = fringes[i];
                const auto& fj = fringes[j];

                double min_x_i = 1e9, max_x_i = -1e9;
                double min_x_j = 1e9, max_x_j = -1e9;

                for (int p = 0; p < fi.GetPointCount(); ++p) {
                    CDPoint pt = fi.GetPoint(p);
                    min_x_i = (std::min)(min_x_i, pt.x);
                    max_x_i = (std::max)(max_x_i, pt.x);
                }
                for (int p = 0; p < fj.GetPointCount(); ++p) {
                    CDPoint pt = fj.GetPoint(p);
                    min_x_j = (std::min)(min_x_j, pt.x);
                    max_x_j = (std::max)(max_x_j, pt.x);
                }

                double overlap_x = (std::max)(0.0, (std::min)(max_x_i, max_x_j) - (std::max)(min_x_i, min_x_j));
                if (overlap_x < params.minOverlapLength) {
                    continue;
                }

                double angle_i = 0.0, angle_j = 0.0;
                if (fi.GetPointCount() >= 2) {
                    CDPoint p0 = fi.GetPoint(0);
                    CDPoint p1 = fi.GetPoint(fi.GetPointCount() - 1);
                    angle_i = std::atan2(p1.y - p0.y, p1.x - p0.x);
                }
                if (fj.GetPointCount() >= 2) {
                    CDPoint p0 = fj.GetPoint(0);
                    CDPoint p1 = fj.GetPoint(fj.GetPointCount() - 1);
                    angle_j = std::atan2(p1.y - p0.y, p1.x - p0.x);
                }

                double angleDiff = std::abs(angle_i - angle_j) * 180.0 / M_PI;
                if (angleDiff > 90.0) {
                    angleDiff = 180.0 - angleDiff;
                }
                if (angleDiff > params.maxTangentAngle) {
                    continue;
                }

                AdjacencyEdge edge;
                edge.i = i;
                edge.j = j;
                edge.distance = dist;
                edge.overlapLength = overlap_x;
                edge.sign = 0;
                edge.weight = 1.0 / (1.0 + dist / params.maxDistance);

                edges.push_back(edge);
            }
        }

        return edges;
    }

    // Phase 3: Constraint system
    struct ConstraintSystem {
        std::vector<std::vector<double>> A;
        std::vector<double> b;
        std::vector<double> weights;
        std::vector<bool> isFixed;
        std::vector<double> fixedValues;
    };

    inline ConstraintSystem GenerateConstraints(
        size_t numFringes,
        const std::vector<FringeNode>& nodes,
        const std::vector<AdjacencyEdge>& edges)
    {
        ConstraintSystem sys;
        sys.isFixed.resize(numFringes, false);
        sys.fixedValues.resize(numFringes, 0.0);

        for (const auto& node : nodes) {
            if (node.isTrusted) {
                sys.isFixed[node.index] = true;
                sys.fixedValues[node.index] = node.knownValue;
            }
        }

        for (const auto& edge : edges) {
            std::vector<double> row(numFringes, 0.0);
            row[edge.i] = -1.0;
            row[edge.j] = 1.0;

            sys.A.push_back(row);
            sys.b.push_back(static_cast<double>(edge.sign));
            sys.weights.push_back(edge.weight);
        }

        return sys;
    }

    // Phase 4: Solve system
    inline std::vector<double> SolveLeastSquares(
        const ConstraintSystem& sys,
        size_t numVars)
    {
        std::vector<std::vector<double>> AtWA(numVars, std::vector<double>(numVars, 0.0));
        std::vector<double> AtWb(numVars, 0.0);

        for (size_t i = 0; i < sys.A.size(); ++i) {
            double w = sys.weights[i];
            const auto& row = sys.A[i];
            double rhs = sys.b[i];

            for (size_t a = 0; a < numVars; ++a) {
                for (size_t b = 0; b < numVars; ++b) {
                    AtWA[a][b] += w * row[a] * row[b];
                }
                AtWb[a] += w * row[a] * rhs;
            }
        }

        double penalty = 1e6;
        for (size_t i = 0; i < numVars; ++i) {
            if (sys.isFixed[i]) {
                AtWA[i][i] += penalty;
                AtWb[i] += penalty * sys.fixedValues[i];
            }
        }

        std::vector<double> x(numVars, 0.0);
        int iterations = 100;
        for (int iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < numVars; ++i) {
                if (sys.isFixed[i]) {
                    x[i] = sys.fixedValues[i];
                    continue;
                }

                double sum = AtWb[i];
                for (size_t j = 0; j < numVars; ++j) {
                    if (i != j) {
                        sum -= AtWA[i][j] * x[j];
                    }
                }

                if (std::abs(AtWA[i][i]) > 1e-10) {
                    x[i] = sum / AtWA[i][i];
                }
            }
        }

        return x;
    }

    // Phase 5: Quantization
    struct QuantizationResult {
        std::vector<double> quantizedK;
        std::vector<double> residuals;
    };

    inline QuantizationResult QuantizeAndValidate(
        const std::vector<double>& continuousK,
        const ConstraintSystem& sys,
        const std::vector<AdjacencyEdge>& edges)
    {
        QuantizationResult result;
        result.quantizedK.resize(continuousK.size());
        result.residuals.resize(edges.size(), 0.0);

        for (size_t i = 0; i < continuousK.size(); ++i) {
            result.quantizedK[i] = std::round(continuousK[i]);
        }

        for (size_t e = 0; e < edges.size(); ++e) {
            const auto& edge = edges[e];
            double kj = result.quantizedK[edge.j];
            double ki = result.quantizedK[edge.i];
            double residual = std::abs(kj - ki - edge.sign);
            result.residuals[e] = residual;
        }

        return result;
    }

    // Phase 6: Confidence
    inline std::vector<double> EvaluateConfidence(
        const std::vector<FringeNode>& nodes,
        const std::vector<AdjacencyEdge>& edges,
        const std::vector<double>& residuals)
    {
        std::vector<double> confidence(nodes.size(), 1.0);

        std::vector<double> avgResidual(nodes.size(), 0.0);
        std::vector<int> degreeCount(nodes.size(), 0);

        for (size_t e = 0; e < edges.size(); ++e) {
            const auto& edge = edges[e];
            avgResidual[edge.i] += residuals[e];
            avgResidual[edge.j] += residuals[e];
            degreeCount[edge.i]++;
            degreeCount[edge.j]++;
        }

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (degreeCount[i] > 0) {
                avgResidual[i] /= degreeCount[i];
                confidence[i] = (std::max)(0.0, 1.0 - avgResidual[i]);
            } else if (nodes[i].isTrusted) {
                confidence[i] = 1.0;
            } else {
                confidence[i] = 0.1;
            }
        }

        return confidence;
    }

    inline std::vector<size_t> UpdateTrustedSet(
        const std::vector<FringeNode>& nodes,
        const std::vector<double>& confidence,
        const std::vector<size_t>& originalTrusted,
        double confidenceThreshold)
    {
        std::vector<size_t> newTrusted;

        for (size_t idx : originalTrusted) {
            newTrusted.push_back(idx);
        }

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (!nodes[i].isTrusted && confidence[i] >= confidenceThreshold) {
                newTrusted.push_back(i);
            }
        }

        return newTrusted;
    }

} // namespace impl

// Main algorithm implementation
inline std::vector<size_t> AutoNumberFringes(
    std::vector<CFringeSegment>& fringes,
    const std::vector<size_t>& trustedFringeIndices,
    double step,
    double confidenceThreshold)
{
    if (fringes.empty() || step < 1e-6) {
        return trustedFringeIndices;
    }

    // Phase 1: Preprocessing
    auto nodes = impl::PreprocessFringes(fringes, trustedFringeIndices, step);

    // Phase 2: Build adjacency graph
    AdjacencyParams params;
    auto edges = impl::BuildAdjacencyGraph(fringes, nodes, params);

    // Phase 3: Generate constraints
    auto sys = impl::GenerateConstraints(nodes.size(), nodes, edges);

    // Phase 4: Solve
    auto continuousK = impl::SolveLeastSquares(sys, nodes.size());

    // Phase 5: Quantize & validate
    auto quantResult = impl::QuantizeAndValidate(continuousK, sys, edges);

    // Phase 6: Confidence evaluation
    auto confidence = impl::EvaluateConfidence(nodes, edges, quantResult.residuals);

    // Update fringes with new numbers
    for (size_t i = 0; i < fringes.size(); ++i) {
        double newNumber = quantResult.quantizedK[i] * step;
        fringes[i].SetNumber(newNumber);
    }

    // Update nodes with confidence
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].confidence = confidence[i];
    }

    // Return new trusted set
    auto newTrusted = impl::UpdateTrustedSet(nodes, confidence, trustedFringeIndices, confidenceThreshold);

    return newTrusted;
}

} // namespace DigitMode
