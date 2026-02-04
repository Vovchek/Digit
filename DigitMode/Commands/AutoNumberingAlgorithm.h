#pragma once

#include "DigitMode/CFringeSegment.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <map>
#include <set>
#include <limits>

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
    enum class Topology { Band, Ring, Saddle, Unknown } topology;
    size_t i, j;            ///< Node indices
    double weight;          ///< Edge confidence weight [0,1]
    int sign;               ///< +1, -1, or 0 (unknown)
    double distance;        ///< Minimum distance between fringes
};

// Default adjacency parameters (configurable)
struct AdjacencyParams {
    // Note: These are INFORMATIONAL ONLY
    // The new Phase 2 implementation uses topology-first adjacency,
    // not distance/overlap/angle gating. These fields are kept for
    // compatibility but are no longer used for hard thresholds.
    double maxDistance = 50.0;      ///< (unused) kept for reference
    double minOverlapLength = 10.0; ///< (unused) kept for reference
    double maxTangentAngle = 30.0;  ///< (unused) kept for reference
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

    // Phase 2: Build adjacency graph (TOPOLOGY-FIRST, NO DISTANCE GATING)
    
    /**
     * @brief Structure classification for Phase 2.1
     * 
     * Soft (non-gating) classification used to select best adjacency builder
     */
    struct StructureClassification {
        enum Type { PARALLEL_BANDS, NESTED_RINGS, MIXED, SADDLE_DOMINATED, UNKNOWN } type;
        double closedRatio;
        double parallelScore, nestedScore;
        double dominantTangent_x, dominantTangent_y;
        double dominantNormal_x, dominantNormal_y;
    };
    
    /**
     * @brief Phase 2.1 — Classify structure (topology first)
     */
    inline StructureClassification ClassifyStructure(
        const std::vector<CFringeSegment>& fringes,
        const std::vector<FringeNode>& nodes)
    {
        StructureClassification result;

        int closedCount = 0;
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].isClosed) closedCount++;
        }
        result.closedRatio = nodes.empty() ? 0.0 : static_cast<double>(closedCount) / nodes.size();

        // Dominant tangent (average of segment tangents), invariant to point order
        double meanTx = 0.0, meanTy = 0.0;
        int validCount = 0;
        bool hasReference = false;
        double refTx = 0.0, refTy = 0.0;

        for (size_t i = 0; i < nodes.size(); ++i) {
            const CFringeSegment& fringe = fringes[i];
            if (fringe.GetPointCount() >= 2) {
                CDPoint first = fringe.GetPoint(0);
                CDPoint last = fringe.GetPoint(fringe.GetPointCount() - 1);
                double dx = last.x - first.x;
                double dy = last.y - first.y;
                double mag = std::sqrt(dx * dx + dy * dy);
                if (mag > 0.0) {
                    double tx = dx / mag;
                    double ty = dy / mag;

                    if (!hasReference) {
                        refTx = tx;
                        refTy = ty;
                        hasReference = true;
                    } else {
                        double dot = tx * refTx + ty * refTy;
                        if (dot < 0.0) {
                            tx = -tx;
                            ty = -ty;
                        }
                    }

                    meanTx += tx;
                    meanTy += ty;
                    validCount++;
                }
            }
        }

        if (validCount > 0) {
            meanTx /= validCount;
            meanTy /= validCount;
        } else {
            meanTx = 1.0;
            meanTy = 0.0;
        }

        double mag = std::sqrt(meanTx * meanTx + meanTy * meanTy);
        if (mag > 0.0) {
            result.dominantTangent_x = meanTx / mag;
            result.dominantTangent_y = meanTy / mag;
        } else {
            result.dominantTangent_x = 1.0;
            result.dominantTangent_y = 0.0;
        }

        result.dominantNormal_x = -result.dominantTangent_y;
        result.dominantNormal_y = result.dominantTangent_x;

        result.parallelScore = (nodes.size() - closedCount);
        result.nestedScore = closedCount;

        if (!nodes.empty() && closedCount == static_cast<int>(nodes.size())) {
            result.type = StructureClassification::NESTED_RINGS;
        } else if (closedCount == 0 && !nodes.empty()) {
            result.type = StructureClassification::PARALLEL_BANDS;
        } else if (closedCount > 0 && closedCount < static_cast<int>(nodes.size())) {
            result.type = StructureClassification::MIXED;
        } else {
            result.type = StructureClassification::UNKNOWN;
        }

        return result;
    }
    
    /**
     * @brief Phase 2.2 — Parallel-band adjacency (ordering-based)
     * 
     * Project onto normal, sort, connect consecutive
     */
    inline std::vector<AdjacencyEdge> BuildParallelBandAdjacency(
        const std::vector<FringeNode>& nodes,
        double normalX, double normalY)
    {
        std::vector<AdjacencyEdge> edges;

        if (nodes.empty()) return edges;

        struct Proj { size_t idx; double s; };
        std::vector<Proj> projs;

        for (size_t i = 0; i < nodes.size(); ++i) {
            Proj p;
            p.idx = i;
            p.s = nodes[i].centroid_x * normalX + nodes[i].centroid_y * normalY;
            projs.push_back(p);
        }

        std::sort(projs.begin(), projs.end(),
            [](const Proj& a, const Proj& b) { return a.s < b.s; });

        for (size_t k = 0; k + 1 < projs.size(); ++k) {
            size_t i = projs[k].idx;
            size_t j = projs[k + 1].idx;

            AdjacencyEdge edge;
            edge.topology = AdjacencyEdge::Topology::Band;
            edge.i = i;
            edge.j = j;
            edge.weight = 1.0;
            edge.sign = +1; // k[j] = k[i] + step in projection order
            edge.distance = std::sqrt(
                std::pow(nodes[i].centroid_x - nodes[j].centroid_x, 2) +
                std::pow(nodes[i].centroid_y - nodes[j].centroid_y, 2)
            );
            edges.push_back(edge);
        }

        return edges;
    }
    
    /**
     * @brief Point-in-polygon test (ray casting)
     */
    inline bool IsPointInsidePolygon(const CFringeSegment& poly, double x, double y)
    {
        int n = poly.GetPointCount();
        if (n < 3) return false;

        bool inside = false;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            CDPoint pi = poly.GetPoint(i);
            CDPoint pj = poly.GetPoint(j);

            bool intersect = ((pi.y > y) != (pj.y > y)) &&
                (x < (pj.x - pi.x) * (y - pi.y) / ((pj.y - pi.y) == 0 ? 1e-12 : (pj.y - pi.y)) + pi.x);
            if (intersect)
                inside = !inside;
        }
        return inside;
    }

    /**
     * @brief Check if all points of inner are inside outer polygon
     */
    inline bool IsFullyInside(const CFringeSegment& inner, const CFringeSegment& outer)
    {
        int count = inner.GetPointCount();
        if (count < 3) return false;
        for (int i = 0; i < count; ++i) {
            CDPoint pt = inner.GetPoint(i);
            if (!IsPointInsidePolygon(outer, pt.x, pt.y)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Check if any point of one polygon lies on the other polyline (touch/intersect)
     */
    inline bool PolylinesTouch(const CFringeSegment& a, const CFringeSegment& b, int tol = 1)
    {
        int countA = a.GetPointCount();
        for (int i = 0; i < countA; ++i) {
            CDPoint pt = a.GetPoint(i);
            CPoint cpt(static_cast<int>(pt.x), static_cast<int>(pt.y));
            int nearest = -1;
            if (const_cast<CFringeSegment&>(b).IsPointOnPolyline(cpt, tol, nearest)) {
                return true;
            }
        }
        int countB = b.GetPointCount();
        for (int i = 0; i < countB; ++i) {
            CDPoint pt = b.GetPoint(i);
            CPoint cpt(static_cast<int>(pt.x), static_cast<int>(pt.y));
            int nearest = -1;
            if (const_cast<CFringeSegment&>(a).IsPointOnPolyline(cpt, tol, nearest)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Phase 2.3 — Nested-ring adjacency (containment-based)
     */
    inline std::vector<AdjacencyEdge> BuildNestedRingAdjacency(
        const std::vector<CFringeSegment>& fringes,
        const std::vector<FringeNode>& nodes)
    {
        std::vector<AdjacencyEdge> edges;

        if (nodes.empty()) return edges;

        // Order closed nodes by average radius from centroid
        struct RingOrder { size_t idx; double r; };
        std::vector<RingOrder> rings;
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (!nodes[i].isClosed) continue;
            const auto& fringe = fringes[i];
            double sumR = 0.0;
            int count = fringe.GetPointCount();
            for (int p = 0; p < count; ++p) {
                CDPoint pt = fringe.GetPoint(p);
                double dx = pt.x - nodes[i].centroid_x;
                double dy = pt.y - nodes[i].centroid_y;
                sumR += std::sqrt(dx * dx + dy * dy);
            }
            RingOrder ro;
            ro.idx = i;
            ro.r = count > 0 ? (sumR / count) : 0.0;
            rings.push_back(ro);
        }

        std::sort(rings.begin(), rings.end(),
            [](const RingOrder& a, const RingOrder& b) { return a.r < b.r; });

        for (size_t k = 0; k + 1 < rings.size(); ++k) {
            size_t innerIdx = rings[k].idx;
            size_t outerIdx = rings[k + 1].idx;

            const auto& inner = fringes[innerIdx];
            const auto& outer = fringes[outerIdx];

            if (PolylinesTouch(inner, outer)) {
                AdjacencyEdge edge;
                edge.topology = AdjacencyEdge::Topology::Saddle;
                edge.i = innerIdx;
                edge.j = outerIdx;
                edge.weight = 1.0;
                edge.sign = 0; // same k if touching/intersecting
                edge.distance = std::sqrt(
                    std::pow(nodes[innerIdx].centroid_x - nodes[outerIdx].centroid_x, 2) +
                    std::pow(nodes[innerIdx].centroid_y - nodes[outerIdx].centroid_y, 2)
                );
                edges.push_back(edge);
                continue;
            }

            // Require all points of inner inside outer
            if (!IsFullyInside(inner, outer)) {
                continue;
            }

            AdjacencyEdge edge;
            edge.topology = AdjacencyEdge::Topology::Ring;
            edge.i = innerIdx;
            edge.j = outerIdx;
            edge.weight = 1.0;
            edge.sign = +1; // k[outer] = k[inner] + step (radially outward)
            edge.distance = std::sqrt(
                std::pow(nodes[innerIdx].centroid_x - nodes[outerIdx].centroid_x, 2) +
                std::pow(nodes[innerIdx].centroid_y - nodes[outerIdx].centroid_y, 2)
            );
            edges.push_back(edge);
        }

        return edges;
    }
    
    /**
     * @brief Phase 2.4 — Mixed/fallback adjacency (robust connectivity)
     * 
     * Nearest neighbor along normal direction (distance affects weight, not gating)
     */
    inline std::vector<AdjacencyEdge> BuildMixedAdjacency(
        const std::vector<FringeNode>& nodes,
        double normalX, double normalY)
    {
        std::vector<AdjacencyEdge> edges;

        for (size_t i = 0; i < nodes.size(); ++i) {
            double bestDistPlus = (std::numeric_limits<double>::max)();
            double bestDistMinus = (std::numeric_limits<double>::max)();
            size_t nearestPlus = ~0u, nearestMinus = ~0u;

            for (size_t j = 0; j < nodes.size(); ++j) {
                if (i == j) continue;

                double dx = nodes[j].centroid_x - nodes[i].centroid_x;
                double dy = nodes[j].centroid_y - nodes[i].centroid_y;
                double s = dx * normalX + dy * normalY;
                double d = std::sqrt(dx * dx + dy * dy);

                if (s > 0 && d < bestDistPlus) {
                    bestDistPlus = d;
                    nearestPlus = j;
                }
                if (s < 0 && d < bestDistMinus) {
                    bestDistMinus = d;
                    nearestMinus = j;
                }
            }

            if (nearestPlus != ~0u) {
                AdjacencyEdge edge;
                edge.topology = AdjacencyEdge::Topology::Band;
                edge.i = i;
                edge.j = nearestPlus;
                edge.weight = 1.0;
                edge.sign = +1;
                edge.distance = bestDistPlus;
                edges.push_back(edge);
            }

            if (nearestMinus != ~0u && nearestMinus != nearestPlus) {
                AdjacencyEdge edge;
                edge.topology = AdjacencyEdge::Topology::Band;
                edge.i = i;
                edge.j = nearestMinus;
                edge.weight = 1.0;
                edge.sign = -1;
                edge.distance = bestDistMinus;
                edges.push_back(edge);
            }
        }

        return edges;
    }
    
    /**
     * @brief Phase 2.5 — Post-processing (deduplication + connectivity check)
     */
    inline std::vector<AdjacencyEdge> PostProcessEdges(
        std::vector<AdjacencyEdge> edges)
    {
        if (edges.empty()) return edges;
        
        // Deduplicate
        std::sort(edges.begin(), edges.end(),
            [](const AdjacencyEdge& a, const AdjacencyEdge& b) {
                if (a.i != b.i) return a.i < b.i;
                if (a.j != b.j) return a.j < b.j;
                return a.weight > b.weight;
            });
        
        std::vector<AdjacencyEdge> deduped;
        typedef std::pair<size_t,size_t> Pair;
        std::set<Pair> seen;
        
        for (size_t e = 0; e < edges.size(); ++e) {
            Pair key(edges[e].i < edges[e].j ? edges[e].i : edges[e].j,
                    edges[e].i < edges[e].j ? edges[e].j : edges[e].i);
            if (seen.find(key) == seen.end()) {
                deduped.push_back(edges[e]);
                seen.insert(key);
            }
        }
        
        return deduped;
    }
    
    /**
     * @brief Phase 2 Main — Topology-first adjacency graph construction
     * 
     * **Key principle**: Topology (ordering, containment) determines adjacency.
     * Distance affects WEIGHT only, never gates decisions.
     * 
     * **Route selection** (soft classification):
     * 1. If parallel bands: Use ordering-based adjacency (Phase 2.2)
     * 2. If nested rings: Use containment-based adjacency (Phase 2.3)
     * 3. Always ensure connectivity via fallback (Phase 2.4)
     */
    inline std::vector<AdjacencyEdge> BuildAdjacencyGraph(
        const std::vector<CFringeSegment>& fringes,
        const std::vector<FringeNode>& nodes,
        const AdjacencyParams& params)
    {
        if (nodes.empty()) return std::vector<AdjacencyEdge>();

        StructureClassification classification = ClassifyStructure(fringes, nodes);

        std::vector<AdjacencyEdge> edges;

        if (classification.type == StructureClassification::PARALLEL_BANDS) {
            edges = BuildParallelBandAdjacency(nodes,
                classification.dominantNormal_x,
                classification.dominantNormal_y);
        } else if (classification.type == StructureClassification::NESTED_RINGS) {
            edges = BuildNestedRingAdjacency(fringes, nodes);
        } else {
            // Mixed/Unknown fallback
            edges = BuildMixedAdjacency(nodes,
                classification.dominantNormal_x,
                classification.dominantNormal_y);

            // Add ring adjacency for closed segments when present
            std::vector<AdjacencyEdge> ringEdges = BuildNestedRingAdjacency(fringes, nodes);
            edges.insert(edges.end(), ringEdges.begin(), ringEdges.end());
        }

        // Phase 2.4: Ensure connectivity with fallback
        if (edges.size() < nodes.size() - 1) {
            std::vector<AdjacencyEdge> fallback = BuildMixedAdjacency(nodes,
                classification.dominantNormal_x,
                classification.dominantNormal_y);
            edges.insert(edges.end(), fallback.begin(), fallback.end());
        }

        // Phase 2.5: Post-process
        edges = PostProcessEdges(edges);

        // Saddle detection: only when both nodes have degree >= 3
        std::vector<size_t> degree(nodes.size(), 0);
        for (const auto& e : edges) {
            degree[e.i]++;
            degree[e.j]++;
        }
        for (auto& e : edges) {
            if (degree[e.i] >= 3 && degree[e.j] >= 3) {
                e.sign = 0;
                e.topology = AdjacencyEdge::Topology::Saddle;
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
    if (fringes.empty() || step == 0.0) {
        return trustedFringeIndices;
    }

    // Phase 0: Preprocessing
    auto nodes = impl::PreprocessFringes(fringes, trustedFringeIndices, step);

    // Phase 1: Build adjacency graph
    AdjacencyParams params;
    auto edges = impl::BuildAdjacencyGraph(fringes, nodes, params);

    // Topology classification for orientation
    auto classification = impl::ClassifyStructure(fringes, nodes);
    double normalX = classification.dominantNormal_x;
    double normalY = classification.dominantNormal_y;
    bool bandNormalFromAnchors = false;

    if (classification.type == impl::StructureClassification::PARALLEL_BANDS && trustedFringeIndices.size() >= 2) {
        // Derive ordering axis from trusted anchors (number increases along this direction)
        for (size_t a = 0; a < trustedFringeIndices.size(); ++a) {
            for (size_t b = a + 1; b < trustedFringeIndices.size(); ++b) {
                size_t ia = trustedFringeIndices[a];
                size_t ib = trustedFringeIndices[b];
                if (ia >= fringes.size() || ib >= fringes.size()) continue;

                int ka = static_cast<int>(std::round(fringes[ia].GetNumber() / step));
                int kb = static_cast<int>(std::round(fringes[ib].GetNumber() / step));
                int dk = kb - ka;
                if (dk == 0) continue;

                size_t lowIdx = dk > 0 ? ia : ib;
                size_t highIdx = dk > 0 ? ib : ia;

                double dx = nodes[highIdx].centroid_x - nodes[lowIdx].centroid_x;
                double dy = nodes[highIdx].centroid_y - nodes[lowIdx].centroid_y;
                double mag = std::sqrt(dx * dx + dy * dy);
                if (mag > 0.0) {
                    normalX = dx / mag;
                    normalY = dy / mag;
                    bandNormalFromAnchors = true;
                }
                a = trustedFringeIndices.size();
                break;
            }
        }

        if (bandNormalFromAnchors) {
            edges = impl::BuildParallelBandAdjacency(nodes, normalX, normalY);
        }
    }

    // Build adjacency list with directed constraints
    struct Adj { size_t to; int delta; };
    std::vector<std::vector<Adj>> adj(nodes.size());

    // Component center for ring orientation
    double centerX = 0.0, centerY = 0.0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        centerX += nodes[i].centroid_x;
        centerY += nodes[i].centroid_y;
    }
    if (!nodes.empty()) {
        centerX /= nodes.size();
        centerY /= nodes.size();
    }

    // Global orientation hint when anchors are insufficient
    std::vector<double> distToOrigin(nodes.size(), 0.0);
    std::vector<double> ringRadius(nodes.size(), 0.0);
    for (size_t i = 0; i < nodes.size(); ++i) {
        distToOrigin[i] = std::sqrt(nodes[i].centroid_x * nodes[i].centroid_x +
                                    nodes[i].centroid_y * nodes[i].centroid_y);

        const auto& fringe = fringes[i];
        double sumR = 0.0;
        int count = fringe.GetPointCount();
        for (int p = 0; p < count; ++p) {
            CDPoint pt = fringe.GetPoint(p);
            double dx = pt.x - nodes[i].centroid_x;
            double dy = pt.y - nodes[i].centroid_y;
            sumR += std::sqrt(dx * dx + dy * dy);
        }
        ringRadius[i] = count > 0 ? (sumR / count) : 0.0;
    }

    bool useGlobalDirection = trustedFringeIndices.size() < 2;
    int bandFlip = 1;
    int ringFlip = 1;
    int ringAnchorFlip = 1;

    if (useGlobalDirection && trustedFringeIndices.size() == 1) {
        size_t anchorIdx = trustedFringeIndices[0];
        if (anchorIdx < nodes.size() && nodes[anchorIdx].isClosed) {
            double minR = (std::numeric_limits<double>::max)();
            double maxR = 0.0;
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (!nodes[i].isClosed) continue;
                minR = (std::min)(minR, ringRadius[i]);
                maxR = (std::max)(maxR, ringRadius[i]);
            }
            double r = ringRadius[anchorIdx];
            if (r >= maxR) {
                ringAnchorFlip = -1; // anchor at outer ring => increase inward
            } else if (r <= minR) {
                ringAnchorFlip = 1; // anchor at inner ring => increase outward
            }
        }
    }

    if (!useGlobalDirection && !bandNormalFromAnchors) {
        // Determine direction from trusted anchors (same topology only)
        for (size_t a = 0; a < trustedFringeIndices.size(); ++a) {
            for (size_t b = a + 1; b < trustedFringeIndices.size(); ++b) {
                size_t ia = trustedFringeIndices[a];
                size_t ib = trustedFringeIndices[b];
                if (ia >= fringes.size() || ib >= fringes.size()) continue;

                int ka = static_cast<int>(std::round(fringes[ia].GetNumber() / step));
                int kb = static_cast<int>(std::round(fringes[ib].GetNumber() / step));
                int dk = kb - ka;
                if (dk == 0) continue;

                bool aClosed = nodes[ia].isClosed;
                bool bClosed = nodes[ib].isClosed;

                if (!aClosed && !bClosed) {
                    double pa = nodes[ia].centroid_x * normalX + nodes[ia].centroid_y * normalY;
                    double pb = nodes[ib].centroid_x * normalX + nodes[ib].centroid_y * normalY;
                    double dp = pb - pa;
                    if (dp != 0.0 && dk * dp < 0.0) {
                        bandFlip = -1;
                    }
                    a = trustedFringeIndices.size();
                    break;
                }

                if (aClosed && bClosed) {
                    double ra = std::sqrt(std::pow(nodes[ia].centroid_x - centerX, 2) +
                                          std::pow(nodes[ia].centroid_y - centerY, 2));
                    double rb = std::sqrt(std::pow(nodes[ib].centroid_x - centerX, 2) +
                                          std::pow(nodes[ib].centroid_y - centerY, 2));
                    double dr = rb - ra;
                    if (dr != 0.0 && dk * dr < 0.0) {
                        ringFlip = -1;
                    }
                    a = trustedFringeIndices.size();
                    break;
                }
            }
        }
    }

    for (auto e : edges) {
        if (useGlobalDirection && e.sign != 0) {
            // Enforce monotonic ordering by distance to origin
            if (e.topology == AdjacencyEdge::Topology::Ring) {
                if (ringRadius[e.j] > ringRadius[e.i]) {
                    e.sign = ringAnchorFlip;
                } else if (ringRadius[e.j] < ringRadius[e.i]) {
                    e.sign = -ringAnchorFlip;
                }
            } else {
                if (distToOrigin[e.j] > distToOrigin[e.i]) {
                    e.sign = +1;
                } else if (distToOrigin[e.j] < distToOrigin[e.i]) {
                    e.sign = -1;
                }
            }
        } else if (e.sign != 0) {
            // Apply anchor-based orientation
            if (e.topology == AdjacencyEdge::Topology::Ring) {
                e.sign *= ringFlip;
            } else if (e.topology == AdjacencyEdge::Topology::Band) {
                e.sign *= bandFlip;
            }
        }

        if (e.sign == 0) {
            // Equality (saddle) constraint
            adj[e.i].push_back({ e.j, 0 });
            adj[e.j].push_back({ e.i, 0 });
        } else {
            adj[e.i].push_back({ e.j, e.sign });
            adj[e.j].push_back({ e.i, -e.sign });
        }
    }

    // Prepare state
    std::vector<bool> assigned(nodes.size(), false);
    std::vector<bool> conflicted(nodes.size(), false);
    std::vector<int> k(nodes.size(), 0);

    // Phase 3: Deterministic propagation (primary solver)
    std::vector<size_t> queue;

    // Initialize from trusted fringes
    for (size_t idx : trustedFringeIndices) {
        if (idx >= fringes.size()) continue;
        double value = fringes[idx].GetNumber();
        k[idx] = static_cast<int>(std::round(value / step));
        assigned[idx] = true;
        queue.push_back(idx);
    }

    auto propagate = [&](std::vector<size_t>& workQueue) {
        for (size_t qi = 0; qi < workQueue.size(); ++qi) {
            size_t i = workQueue[qi];
            for (const auto& edge : adj[i]) {
                size_t j = edge.to;
                int proposed = k[i] + edge.delta;
                if (!assigned[j]) {
                    k[j] = proposed;
                    assigned[j] = true;
                    workQueue.push_back(j);
                } else if (k[j] != proposed) {
                    conflicted[j] = true;
                    conflicted[i] = true;
                }
            }
        }
    };

    // Propagate from trusted anchors first
    if (!queue.empty()) {
        propagate(queue);
    }

    // Edge-based propagation pass to ensure reachability across the component
    bool progressed = true;
    while (progressed) {
        progressed = false;
        for (const auto& e : edges) {
            int delta = e.sign;
            if (delta == 0) {
                if (assigned[e.i] && !assigned[e.j]) {
                    k[e.j] = k[e.i];
                    assigned[e.j] = true;
                    progressed = true;
                } else if (assigned[e.j] && !assigned[e.i]) {
                    k[e.i] = k[e.j];
                    assigned[e.i] = true;
                    progressed = true;
                }
                continue;
            }

            if (assigned[e.i] && !assigned[e.j]) {
                k[e.j] = k[e.i] + delta;
                assigned[e.j] = true;
                progressed = true;
            } else if (assigned[e.j] && !assigned[e.i]) {
                k[e.i] = k[e.j] - delta;
                assigned[e.i] = true;
                progressed = true;
            } else if (assigned[e.i] && assigned[e.j]) {
                if (k[e.j] != k[e.i] + delta) {
                    conflicted[e.i] = true;
                    conflicted[e.j] = true;
                }
            }
        }
    }

    // Phase 4: Component completion for unanchored components
    std::vector<bool> visited(nodes.size(), false);
    for (size_t start = 0; start < nodes.size(); ++start) {
        if (assigned[start] || visited[start]) continue;

        // Collect component
        std::vector<size_t> component;
        std::vector<size_t> stack;
        stack.push_back(start);
        visited[start] = true;

        while (!stack.empty()) {
            size_t v = stack.back();
            stack.pop_back();
            component.push_back(v);

            for (const auto& edge : adj[v]) {
                size_t u = edge.to;
                if (!visited[u] && !assigned[u]) {
                    visited[u] = true;
                    stack.push_back(u);
                }
            }
        }

        // Choose anchor closest to origin within this component
        size_t anchor = component.front();
        double bestDist = (std::numeric_limits<double>::max)();
        for (size_t idx : component) {
            double d = std::sqrt(nodes[idx].centroid_x * nodes[idx].centroid_x +
                                 nodes[idx].centroid_y * nodes[idx].centroid_y);
            if (d < bestDist) {
                bestDist = d;
                anchor = idx;
            }
        }

        k[anchor] = 0;
        assigned[anchor] = true;
        std::vector<size_t> componentQueue;
        componentQueue.push_back(anchor);
        propagate(componentQueue);
    }

    // Phase 6: Quantization & validation
    std::vector<size_t> newTrusted;
    for (size_t i = 0; i < fringes.size(); ++i) {
        if (assigned[i]) {
            double newNumber = static_cast<double>(k[i]) * step;
            fringes[i].SetNumber(newNumber);
        }

        // Trusted if assigned and not conflicted
        if (assigned[i] && !conflicted[i]) {
            newTrusted.push_back(i);
        }
    }

    // Ensure original trusted are preserved
    for (size_t idx : trustedFringeIndices) {
        if (std::find(newTrusted.begin(), newTrusted.end(), idx) == newTrusted.end()) {
            newTrusted.push_back(idx);
        }
    }

    return newTrusted;
}

} // namespace DigitMode
