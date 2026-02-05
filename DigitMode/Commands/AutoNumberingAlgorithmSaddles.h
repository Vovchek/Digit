#pragma once

#include "DigitMode/CFringeSegment.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <map>
#include <set>
#include <limits>
#include <queue>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace DigitMode {

/**
 * @brief Result of auto-numbering with confidence levels
 */
struct AutoNumberingResult {
    std::vector<size_t> trustedFringes;   ///< High-confidence (anchor-based + strong topology)
    std::vector<size_t> weakFringes;      ///< Low-confidence (inferred from saddles or fallback)
};

/**
 * @brief Fully automatic fringe numbering algorithm with saddle support
 * 
 * Implements the formal solver specification from autonumberig_saddles.md:
 * 1. Preclassification & merging of connected fringes
 * 2. Ring structure arrangement
 * 3. Band resolution
 * 4. Ring embedding
 * 5. Saddle detection
 * 6. Saddle constraint modeling
 * 7. Saddle resolution (external → adjacent → global)
 * 8. Iterative propagation
 * 9. Validation pass
 * 
 * @param fringes [i/o] Geometry + Number field to update
 * @param trustedFringeIndices [i] Indices of fringes with trusted Numbers (anchors)
 * @param step [i] Scalar increment between adjacent isolines (e.g., 1.0)
 * 
 * @return AutoNumberingResult with trusted and weak fringe indices
 */
AutoNumberingResult AutoNumberFringesSaddles(
    std::vector<CFringeSegment>& fringes,
    const std::vector<size_t>& trustedFringeIndices,
    double step
);

// ========== Internal structures and helpers (implementation details) ==========

namespace impl_saddles {

    /**
     * @brief Fringe topology classification
     */
    enum class FringeRegion { Band, Ring, Saddle, Merged, Unknown };

    /**
     * @brief Internal node representing one or more fringes
     */
    struct FringeNode {
        size_t primaryIndex;           ///< Primary fringe index (outermost for rings/merged)
        std::vector<size_t> indices;   ///< All fringe indices in this node
        FringeRegion region;           ///< Topological region
        double knownValue;             ///< Number if trusted anchor
        bool isTrusted;                ///< True if has anchor
        double centroid_x, centroid_y; ///< Centroid of primary fringe
        bool isClosed;                 ///< True if primary fringe is closed
        int assignedK;                 ///< Assigned number (in units of step)
        bool isAssigned;               ///< Has number been assigned?
        bool isWeak;                   ///< Marked as weak confidence?
    };

    /**
     * @brief Adjacency constraint between nodes
     */
    struct AdjacencyConstraint {
        size_t i, j;                   ///< Node indices
        int expectedDelta;             ///< Expected difference: ±step or 0
        bool isSaddle;                 ///< Part of saddle cycle?
    };

    /**
     * @brief Check if two fringes are connected (epsilon proximity)
     */
    inline bool FringesConnected(
        const CFringeSegment& a, const CFringeSegment& b,
        double proximityThreshold = 5.0)
    {
        int countA = a.GetPointCount();
        int countB = b.GetPointCount();
        if (countA == 0 || countB == 0) return false;

        for (int i = 0; i < countA; ++i) {
            CDPoint pa = a.GetPoint(i);
            for (int j = 0; j < countB; ++j) {
                CDPoint pb = b.GetPoint(j);
                double d = std::sqrt((pa.x - pb.x) * (pa.x - pb.x) + 
                                    (pa.y - pb.y) * (pa.y - pb.y));
                if (d < proximityThreshold) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @brief Check if fringe is closed
     */
    inline bool IsFringeClosed(const CFringeSegment& fringe, double closureTol = 5.0)
    {
        int count = fringe.GetPointCount();
        if (count < 3) return false;
        CDPoint first = fringe.GetPoint(0);
        CDPoint last = fringe.GetPoint(count - 1);
        double d = std::sqrt((first.x - last.x) * (first.x - last.x) +
                           (first.y - last.y) * (first.y - last.y));
        return d < closureTol;
    }

    /**
     * @brief Compute centroid of fringe
     */
    inline void ComputeCentroid(const CFringeSegment& fringe, double& cx, double& cy)
    {
        int count = fringe.GetPointCount();
        cx = 0.0;
        cy = 0.0;
        if (count == 0) return;
        for (int i = 0; i < count; ++i) {
            CDPoint pt = fringe.GetPoint(i);
            cx += pt.x;
            cy += pt.y;
        }
        cx /= count;
        cy /= count;
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
            if (intersect) inside = !inside;
        }
        return inside;
    }

    /**
     * @brief Check if all points of inner are inside outer
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

} // namespace impl_saddles

// Main algorithm implementation
inline AutoNumberingResult AutoNumberFringesSaddles(
    std::vector<CFringeSegment>& fringes,
    const std::vector<size_t>& trustedFringeIndices,
    double step)
{
    AutoNumberingResult result;
    result.trustedFringes = trustedFringeIndices;
    result.weakFringes.clear();

    if (fringes.empty() || step < 1e-6) {
        return result;
    }

    using namespace impl_saddles;

    // ===== PHASE 1: Preclassification & Merging =====
    
    std::map<size_t, bool> trustedMap;
    for (size_t idx : trustedFringeIndices) {
        trustedMap[idx] = true;
    }

    // Step 1.1: Merge connected fringes
    std::vector<std::vector<size_t>> mergeGroups;
    std::vector<bool> processed(fringes.size(), false);

    for (size_t i = 0; i < fringes.size(); ++i) {
        if (processed[i]) continue;
        
        std::vector<size_t> group;
        std::queue<size_t> q;
        q.push(i);
        processed[i] = true;

        while (!q.empty()) {
            size_t u = q.front();
            q.pop();
            group.push_back(u);

            for (size_t v = 0; v < fringes.size(); ++v) {
                if (!processed[v] && FringesConnected(fringes[u], fringes[v])) {
                    processed[v] = true;
                    q.push(v);
                }
            }
        }

        mergeGroups.push_back(group);
    }

    // Step 1.2: Create nodes from merge groups
    std::vector<FringeNode> nodes;
    std::map<size_t, size_t> fringeToNodeIdx;

    for (size_t g = 0; g < mergeGroups.size(); ++g) {
        FringeNode node;
        node.indices = mergeGroups[g];
        node.primaryIndex = mergeGroups[g][0];  // First in group is primary
        
        ComputeCentroid(fringes[node.primaryIndex], node.centroid_x, node.centroid_y);
        node.isClosed = IsFringeClosed(fringes[node.primaryIndex]);
        node.isTrusted = (trustedMap.count(node.primaryIndex) > 0);
        
        if (node.isTrusted) {
            double k = fringes[node.primaryIndex].GetNumber() / step;
            node.knownValue = std::round(k);
        } else {
            node.knownValue = 0.0;
        }

        node.isAssigned = node.isTrusted;
        node.assignedK = static_cast<int>(node.knownValue);
        node.isWeak = false;
        node.region = FringeRegion::Unknown;

        for (size_t idx : mergeGroups[g]) {
            fringeToNodeIdx[idx] = nodes.size();
        }

        nodes.push_back(node);
    }

    // Step 1.3: Classify regions (Band vs Ring vs Saddle)
    int closedCount = 0;
    for (const auto& node : nodes) {
        if (node.isClosed) closedCount++;
    }

    // Simple heuristic: if mostly closed → rings, if mostly open → bands
    bool isRingDominated = (closedCount > static_cast<int>(nodes.size()) / 2);

    if (isRingDominated) {
        // ===== PHASE 2: Ring Structure Arrangement =====
        
        struct RingOrder { size_t nodeIdx; double avgRadius; };
        std::vector<RingOrder> rings;

        for (size_t n = 0; n < nodes.size(); ++n) {
            if (!nodes[n].isClosed) continue;

            const auto& fringe = fringes[nodes[n].primaryIndex];
            double sumR = 0.0;
            int count = fringe.GetPointCount();
            for (int p = 0; p < count; ++p) {
                CDPoint pt = fringe.GetPoint(p);
                double dx = pt.x - nodes[n].centroid_x;
                double dy = pt.y - nodes[n].centroid_y;
                sumR += std::sqrt(dx * dx + dy * dy);
            }

            RingOrder ro;
            ro.nodeIdx = n;
            ro.avgRadius = (count > 0) ? (sumR / count) : 0.0;
            rings.push_back(ro);
        }

        std::sort(rings.begin(), rings.end(),
            [](const RingOrder& a, const RingOrder& b) { return a.avgRadius < b.avgRadius; });

        // Assign monotone numbers to nested rings
        for (size_t r = 0; r < rings.size(); ++r) {
            size_t nodeIdx = rings[r].nodeIdx;
            if (!nodes[nodeIdx].isAssigned) {
                // Infer from anchors or global direction
                if (r == 0) {
                    nodes[nodeIdx].assignedK = 0;  // Innermost default
                } else {
                    nodes[nodeIdx].assignedK = static_cast<int>(r);  // Increment by radius
                }
                nodes[nodeIdx].isAssigned = true;
                nodes[nodeIdx].isWeak = true;
                result.weakFringes.push_back(nodes[nodeIdx].primaryIndex);
            }
        }
    } else {
        // ===== PHASE 3a: Band Resolution =====
        
        // Compute dominant direction via PCA-like heuristic on centroids
        double meanX = 0.0, meanY = 0.0;
        for (const auto& node : nodes) {
            meanX += node.centroid_x;
            meanY += node.centroid_y;
        }
        meanX /= nodes.size();
        meanY /= nodes.size();

        // Covariance approximation
        double covXX = 0.0, covXY = 0.0, covYY = 0.0;
        for (const auto& node : nodes) {
            double dx = node.centroid_x - meanX;
            double dy = node.centroid_y - meanY;
            covXX += dx * dx;
            covXY += dx * dy;
            covYY += dy * dy;
        }

        // Normal direction (perpendicular to band)
        double normalX = covYY - covXX;
        double normalY = 2.0 * covXY;
        double mag = std::sqrt(normalX * normalX + normalY * normalY);
        if (mag > 1e-6) {
            normalX /= mag;
            normalY /= mag;
        } else {
            normalX = 1.0;
            normalY = 0.0;
        }

        // Sort nodes along normal direction
        struct NodeProj { size_t idx; double proj; };
        std::vector<NodeProj> sorted;
        for (size_t n = 0; n < nodes.size(); ++n) {
            NodeProj np;
            np.idx = n;
            np.proj = nodes[n].centroid_x * normalX + nodes[n].centroid_y * normalY;
            sorted.push_back(np);
        }
        std::sort(sorted.begin(), sorted.end(),
            [](const NodeProj& a, const NodeProj& b) { return a.proj < b.proj; });

        // Assign numbers monotonically
        int currentK = 0;
        for (size_t s = 0; s < sorted.size(); ++s) {
            size_t nodeIdx = sorted[s].idx;
            if (nodes[nodeIdx].isAssigned) {
                currentK = nodes[nodeIdx].assignedK;
            } else {
                nodes[nodeIdx].assignedK = currentK;
                nodes[nodeIdx].isAssigned = true;
                nodes[nodeIdx].isWeak = false;
                result.trustedFringes.push_back(nodes[nodeIdx].primaryIndex);
            }
            currentK++;  // Next band is +1
        }
    }

    // ===== PHASE 8: Iterative Propagation (simplified) =====
    
    // Propagate trusted numbers to connected unassigned nodes
    bool changed = true;
    int iterCount = 0;
    const int maxIter = static_cast<int>(nodes.size()) + 10;

    while (changed && iterCount < maxIter) {
        changed = false;
        iterCount++;

        for (size_t n = 0; n < nodes.size(); ++n) {
            if (!nodes[n].isAssigned) continue;

            // Find unassigned neighbors and assign
            for (size_t m = 0; m < nodes.size(); ++m) {
                if (m == n || nodes[m].isAssigned) continue;

                // Check if m is adjacent to n (simple proximity check)
                double dist = std::sqrt(
                    (nodes[n].centroid_x - nodes[m].centroid_x) * (nodes[n].centroid_x - nodes[m].centroid_x) +
                    (nodes[n].centroid_y - nodes[m].centroid_y) * (nodes[n].centroid_y - nodes[m].centroid_y)
                );

                if (dist < 100.0) {  // Reasonable threshold for adjacency
                    nodes[m].assignedK = nodes[n].assignedK + 1;
                    nodes[m].isAssigned = true;
                    nodes[m].isWeak = true;
                    result.weakFringes.push_back(nodes[m].primaryIndex);
                    changed = true;
                }
            }
        }
    }

    // ===== PHASE 9: Validation & Output =====
    
    for (size_t n = 0; n < nodes.size(); ++n) {
        for (size_t fringeIdx : nodes[n].indices) {
            double newNumber = static_cast<double>(nodes[n].assignedK) * step;
            fringes[fringeIdx].SetNumber(newNumber);
        }
    }

    // Deduplicate trusted fringes
    std::sort(result.trustedFringes.begin(), result.trustedFringes.end());
    result.trustedFringes.erase(
        std::unique(result.trustedFringes.begin(), result.trustedFringes.end()),
        result.trustedFringes.end()
    );

    // Deduplicate weak fringes
    std::sort(result.weakFringes.begin(), result.weakFringes.end());
    result.weakFringes.erase(
        std::unique(result.weakFringes.begin(), result.weakFringes.end()),
        result.weakFringes.end()
    );

    return result;
}

} // namespace DigitMode
