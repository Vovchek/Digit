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

    /**
     * @brief Detect if two fringes intersect or overlap
     * 
     * Simple heuristic: check if centroid-to-centroid distance is small relative to fringe size
     * Or check if any point from fringe i is close to fringe j
     */
    inline bool FringesIntersect(
        const CFringeSegment& fringe_i,
        const CFringeSegment& fringe_j,
        double intersectionThreshold = 20.0)
    {
        // Quick check: centroid distance
        double sumX_i = 0.0, sumY_i = 0.0, sumX_j = 0.0, sumY_j = 0.0;
        int count_i = fringe_i.GetPointCount();
        int count_j = fringe_j.GetPointCount();
        
        if (count_i == 0 || count_j == 0) return false;
        
        for (int p = 0; p < count_i; ++p) {
            CDPoint pt = fringe_i.GetPoint(p);
            sumX_i += pt.x;
            sumY_i += pt.y;
        }
        double cx_i = sumX_i / count_i;
        double cy_i = sumY_i / count_i;
        
        for (int p = 0; p < count_j; ++p) {
            CDPoint pt = fringe_j.GetPoint(p);
            sumX_j += pt.x;
            sumY_j += pt.y;
        }
        double cx_j = sumX_j / count_j;
        double cy_j = sumY_j / count_j;
        
        double centroidDist = std::sqrt((cx_i - cx_j) * (cx_i - cx_j) + 
                                       (cy_i - cy_j) * (cy_i - cy_j));
        
        // Rough check: if centroids are very close, likely intersecting
        if (centroidDist < intersectionThreshold) {
            // More detailed check: do any points actually cross?
            for (int pi = 0; pi < count_i; ++pi) {
                CDPoint pti = fringe_i.GetPoint(pi);
                for (int pj = 0; pj < count_j; ++pj) {
                    CDPoint ptj = fringe_j.GetPoint(pj);
                    double d = std::sqrt((pti.x - ptj.x) * (pti.x - ptj.x) + 
                                        (pti.y - ptj.y) * (pti.y - ptj.y));
                    if (d < 5.0) {  // Points are very close
                        return true;
                    }
                }
            }
        }
        
        return false;
    }

    /**
     * @brief Detect saddle topology pattern (4 fringes forming rectangle)
     * 
     * Saddle: Top-Right-Bottom-Left forming a cycle with gaps
     * Returns pairs of indices that are opposite (should have same number)
     */
    inline std::vector<std::pair<size_t, size_t>> DetectSaddlePattern(
        const std::vector<CFringeSegment>& fringes,
        const std::vector<FringeNode>& nodes)
    {
        std::vector<std::pair<size_t, size_t>> opposites;
        
        // Look for 4 fringes that form a rectangle pattern
        if (fringes.size() < 4) return opposites;
        
        // Try to identify which fringes are which (top, right, bottom, left)
        // Based on centroid positions
        
        for (size_t i = 0; i < fringes.size(); ++i) {
            for (size_t j = i + 1; j < fringes.size(); ++j) {
                // Check if i and j are roughly opposite (far apart, similar orientation)
                double dy = std::abs(nodes[i].centroid_y - nodes[j].centroid_y);
                double dx = std::abs(nodes[i].centroid_x - nodes[j].centroid_x);
                
                // Get fringe orientations (roughly horizontal or vertical)
                int count_i = fringes[i].GetPointCount();
                int count_j = fringes[j].GetPointCount();
                
                if (count_i < 2 || count_j < 2) continue;
                
                CDPoint first_i = fringes[i].GetPoint(0);
                CDPoint last_i = fringes[i].GetPoint(count_i - 1);
                double span_x_i = std::abs(last_i.x - first_i.x);
                double span_y_i = std::abs(last_i.y - first_i.y);
                bool horizontal_i = (span_x_i > span_y_i);
                
                CDPoint first_j = fringes[j].GetPoint(0);
                CDPoint last_j = fringes[j].GetPoint(count_j - 1);
                double span_x_j = std::abs(last_j.x - first_j.x);
                double span_y_j = std::abs(last_j.y - first_j.y);
                bool horizontal_j = (span_x_j > span_y_j);
                
                // Opposite fringes: same orientation, far apart in perpendicular direction
                if (horizontal_i && horizontal_j && dy > 30.0) {
                    // Both horizontal, far apart vertically → top and bottom
                    opposites.push_back({i, j});
                } else if (!horizontal_i && !horizontal_j && dx > 30.0) {
                    // Both vertical, far apart horizontally → left and right
                    opposites.push_back({i, j});
                }
            }
        }
        
        return opposites;
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
    if (fringes.empty() || step < 1e-6) {
        return trustedFringeIndices;
    }

    // Phase 1: Preprocessing
    std::vector<FringeNode> nodes = impl::PreprocessFringes(fringes, trustedFringeIndices, step);

    // PHASE 1 ENHANCEMENT: Detect intersecting fringes and saddle patterns
    std::vector<std::pair<size_t, size_t>> intersectingPairs;
    for (size_t i = 0; i < fringes.size(); ++i) {
        for (size_t j = i + 1; j < fringes.size(); ++j) {
            if (impl::FringesIntersect(fringes[i], fringes[j])) {
                intersectingPairs.push_back(std::make_pair(i, j));
            }
        }
    }
    
    // Detect saddle topology patterns (opposite sides)
    std::vector<std::pair<size_t, size_t>> oppositePairs = 
        impl::DetectSaddlePattern(fringes, nodes);

    // Phase 2: Build adjacency graph
    AdjacencyParams params;  // Default parameters
    std::vector<AdjacencyEdge> edges = impl::BuildAdjacencyGraph(fringes, nodes, params);

    // Phase 3: Generate constraints
    impl::ConstraintSystem sys = impl::GenerateConstraints(nodes.size(), nodes, edges);

    // PHASE 3 ENHANCEMENT: Add equality constraints for intersecting fringes
    // Intersecting fringes MUST have same number
    for (size_t p = 0; p < intersectingPairs.size(); ++p) {
        std::vector<double> row(nodes.size(), 0.0);
        row[intersectingPairs[p].first] = 1.0;
        row[intersectingPairs[p].second] = -1.0;
        sys.A.push_back(row);
        sys.b.push_back(0.0);  // Difference must be 0
        sys.weights.push_back(1.0);  // High confidence
    }
    
    // NOTE: Opposite fringes in saddle are handled by Phase 2 adjacency graph
    // (BuildAdjacencyGraph detects topology-first adjacency)
    // Do NOT add explicit equality constraints - they override the proper stepping
    // The adjacency edges already encode the saddle topology correctly

    // Phase 4: Solve
    std::vector<double> continuousK = impl::SolveLeastSquares(sys, nodes.size());

    // Phase 5: Quantize & validate
    impl::QuantizationResult quantResult = impl::QuantizeAndValidate(continuousK, sys, edges);

    // Phase 6: Confidence evaluation
    std::vector<double> confidence = impl::EvaluateConfidence(nodes, edges, quantResult.residuals);

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
    std::vector<size_t> newTrusted = impl::UpdateTrustedSet(nodes, confidence, trustedFringeIndices, confidenceThreshold);

    return newTrusted;
}

} // namespace DigitMode
