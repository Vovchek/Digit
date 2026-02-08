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

inline std::vector<size_t> AutoNumberFringes(
    std::vector<CFringeSegment>& fringes,
    const std::vector<size_t>& trustedFringeIndices,
    double step,
    double confidenceThreshold
) {
    auto res = AutoNumberFringesSaddles(fringes, trustedFringeIndices, step);
    return res.trustedFringes;
}

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
        CDRect boundingBox;            ///< Bounding box of fringes
        bool isClosed;                 ///< True if primary fringe is closed
        int assignedK;                 ///< Assigned number (in units of step)
        bool isAssigned;               ///< Has number been assigned?
        bool isWeak;                   ///< Marked as weak confidence?
        size_t outerNodeIdx;           ///< Enclosing ring node (for hierarchy)
        bool isSeparateFrom(const FringeNode& other, double proximityThreshold = 0.0) const {
            return
                (boundingBox.left > other.boundingBox.right + proximityThreshold) ||
                (other.boundingBox.left > boundingBox.right + proximityThreshold) ||
                (boundingBox.top > other.boundingBox.bottom + proximityThreshold) ||
                (other.boundingBox.top > boundingBox.bottom + proximityThreshold);
        }

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
     * @brief Result of band resolution
     */
    struct BandResolutionResult {
        std::vector<size_t> trustedFringes;
        std::vector<size_t> weakFringes;
    };

    /**
     * @brief Check if fringe is closed
     */
    inline bool IsFringeClosed(const CFringeSegment& fringe, double closureTol = 5.0)
    {
        int count = fringe.GetPointCount();
        if (count < 3) return false;
        CDPoint first = fringe.GetPoint(0);
        CDPoint last = fringe.GetPoint(count - 1);
        double d2 = ((first.x - last.x) * (first.x - last.x) +
            (first.y - last.y) * (first.y - last.y));
        return d2 < closureTol*closureTol;
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
     * @brief Compute bounding box of fringe
     */
    inline void ComputeBoundingBox(const CFringeSegment& fringe, CDRect& box)
    {
        int count = fringe.GetPointCount();
        box = { 1e10, 1e10, -1e10, -1e10 };
        if (count == 0) return;
        for (int i = 0; i < count; ++i) {
            CDPoint pt = fringe.GetPoint(i);
            box.left = (std::min)(box.left, pt.x);
            box.right = (std::max)(box.right, pt.x);
            box.top = (std::min)(box.top, pt.y);
            box.bottom = (std::max)(box.bottom, pt.y);
        }
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
        if (!IsFringeClosed(outer)) return false;
        int count = inner.GetPointCount();
        for (int i = 0; i < count; ++i) {
            CDPoint pt = inner.GetPoint(i);
            if (!IsPointInsidePolygon(outer, pt.x, pt.y)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Check if inner node is fully inside outer node (optimized with bounding box check)
     * 
     * @param inner Inner node to test
     * @param outer Outer node (must be closed to contain anything)
     * @param fringes Original fringe geometries
     * @return true if inner is fully contained within outer
     */
    inline bool IsFullyInside(
        const FringeNode& inner, 
        const FringeNode& outer, 
        const std::vector<CFringeSegment>& fringes)
    {
        // Quick reject: if bounding boxes don't overlap, can't be inside
        if (inner.isSeparateFrom(outer)) {
            return false;
        }
		// If multiple fringes, check all closed rings in outer 
        // if ANY of them contain ALL inner fringes
        for (auto idxOut : outer.indices) {
            // Geometric containment test is superfluious here
            // because crossing fringes are already merged within node
            // --- not nessesary ----
            // if(std::all_of(inner.indices.begin(), inner.indices.end(),
            //        [&fringes, idxOut](size_t idx) { return IsFullyInside(fringes[idx], fringes[idxOut]); }))
            //    return true;
            // ----------------------
            // it's either fully outside or inside, any sample point will do:
            CDPoint pt = fringes[inner.primaryIndex].GetPoint(0);
            if (IsPointInsidePolygon(fringes[idxOut], pt.x, pt.y))
                return true;
        }
        return false;
    }

    /**
     * @brief Topology classification result
     */
    struct TopologyClassification {
        std::vector<size_t> bandNodeIndices;  ///< Topmost band-like fringes (not enclosed in any ring)
        std::vector<size_t> ringNodeIndices;  ///< Ring structures (rings + anything inside them)
    };

    /**
     * @brief Classify nodes into topmost bands and ring structures
     * 
     * Separates nodes into two categories:
     * - Band nodes: Open fringes NOT enclosed in any ring (topmost bands)
     * - Ring nodes: Closed rings OR any fringe (open/closed) enclosed in a ring
     * 
     * Computes outerNodeIdx for each node: the immediately enclosing ring node.
     * For hierarchy, the immediate parent is the smallest enclosing ring.
     * 
     * @param nodes Node array to classify (modified with outerNodeIdx/outerFringIdx)
     * @param fringes Original fringe geometries
     * @return TopologyClassification with band and ring node index lists
     */
    inline TopologyClassification ClassifyTopology(
        std::vector<FringeNode>& nodes,
        const std::vector<CFringeSegment>& fringes)
    {
        TopologyClassification result;
        
        const size_t n = nodes.size();

        for (size_t i = 0; i < n; ++i) {

            size_t closestParent = SIZE_MAX;

            // Search for all parent candidates
            for (size_t j = 0; j < n; ++j) {
                if (i == j) continue;

                if (IsFullyInside(nodes[i], nodes[j], fringes)) {
					// Check if j is the closest parent (no other node fully inside j contains i)
                    bool hasMiddle = false;

                    for (size_t k = 0; k < n; ++k) {
                        if (k == i || k == j) continue;

                        if (IsFullyInside(nodes[i], nodes[k], fringes) &&
                            IsFullyInside(nodes[k], nodes[j], fringes)) {
                            hasMiddle = true;
                            break;
                        }
                    }
                    if(!hasMiddle) {
                        closestParent = j;
                        break; // No need to check further, we want the closest parent
                    }
                }
            }

            nodes[i].outerNodeIdx = closestParent;
        }

        // Step 2: Classify into bands and rings
        for (size_t n = 0; n < nodes.size(); ++n) {
            bool isEnclosedInRing = (nodes[n].outerNodeIdx != SIZE_MAX);
            
            if (isEnclosedInRing || nodes[n].isClosed) {
                // Part of ring structure
                result.ringNodeIndices.push_back(n);
            } else {
                // Topmost band (open and not enclosed)
                result.bandNodeIndices.push_back(n);
            }
        }
        
        return result;
    }

    struct HelperNode {
        const CFringeSegment& m_fringe;
        size_t m_i;
        CDRect m_boundingBox;
        double m_centroidX;
        double m_centroidY;
        HelperNode(const CFringeSegment& fr, size_t i) :m_fringe(fr), m_i(i) {
            ComputeBoundingBox(m_fringe, m_boundingBox);
            ComputeCentroid(m_fringe, m_centroidX, m_centroidY);
        };
        bool isSeparateFrom(const HelperNode& other, double proximityThreshold = 0.0) const {
            return
                (m_boundingBox.left > other.m_boundingBox.right + proximityThreshold) ||
                (other.m_boundingBox.left > m_boundingBox.right + proximityThreshold) ||
                (m_boundingBox.top > other.m_boundingBox.bottom + proximityThreshold) ||
                (other.m_boundingBox.top > m_boundingBox.bottom + proximityThreshold);
        }
    };

    struct GroupMetadata {
        CDRect boundingBox;
        double centroidX;
        double centroidY;
        size_t primaryIndex;
        bool isTrusted;
        double knownValue;
        std::vector<size_t> indices;
    };

    /**
     * @brief Check if two fringes are connected (epsilon proximity)
     */
    enum Orientation {
        COLLINEAR = 0,
        CLOCKWISE = 1,
        COUNTER_CLOCKWISE = 2
    };

    inline Orientation orientation(const CDPoint& p, const CDPoint& q, const CDPoint& r) {
        double val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

        if (std::abs(val) < 1e-9) return COLLINEAR;
        return (val > 0) ? CLOCKWISE : COUNTER_CLOCKWISE;
    }

    inline bool onSegment(const CDPoint& p, const CDPoint& r, const CDPoint& q, double tolerance = 1e-9) {
        // Проверяем, лежит ли q на отрезке pr с заданной точностью
        double cross = (q.y - p.y) * (r.x - p.x) - (q.x - p.x) * (r.y - p.y);

        // Если не коллинеарны (с учетом точности)
        if (std::abs(cross) > tolerance) {
            return false;
        }

        // Проверяем, находится ли q между p и r (с учетом точности)
        double dot = (q.x - p.x) * (r.x - p.x) + (q.y - p.y) * (r.y - p.y);
        if (dot < -tolerance) {
            return false;
        }

        double segLengthSquared = (r.x - p.x) * (r.x - p.x) + (r.y - p.y) * (r.y - p.y);
        if (dot > segLengthSquared + tolerance) {
            return false;
        }

        return true;
    }

    inline bool segmentsIntersect(const CDPoint& p1, const CDPoint& p2,
        const CDPoint& p3, const CDPoint& p4,
        bool includeTouching = true) {
        // Вычисляем ориентации для всех комбинаций
        Orientation o1 = orientation(p1, p2, p3);
        Orientation o2 = orientation(p1, p2, p4);
        Orientation o3 = orientation(p3, p4, p1);
        Orientation o4 = orientation(p3, p4, p2);

        // Общий случай: отрезки пересекаются
        if (o1 != o2 && o3 != o4) {
            return true;
        }
        if (includeTouching) {
            // Проверка специальных случаев коллинеарности
            if (o1 == COLLINEAR && onSegment(p1, p2, p3)) return true;
            if (o2 == COLLINEAR && onSegment(p1, p2, p4)) return true;
            if (o3 == COLLINEAR && onSegment(p3, p4, p1)) return true;
            if (o4 == COLLINEAR && onSegment(p3, p4, p2)) return true;

            // Проверка совпадения конечных точек
            if (p1 == p3 || p1 == p4 || p2 == p3 || p2 == p4) return true;
        }

        return false;
    }

    inline bool checkPointsProximity(const CDPoint& p1, const CDPoint& p2, double distance) {
        if (distance <= 0.0) {
            return p1 == p2; // Точное совпадение
        }
        double dx = p1.x - p2.x;
        double dy = p1.y - p2.y;
        return (dx * dx + dy * dy) <= (distance * distance);
    }

    inline bool pointsAreClose(const CDPoint& p1, const CDPoint& p2, double distance) {
        return checkPointsProximity(p1, p2, distance);
    }

    // Функция для проверки близости точки к отрезку
    inline bool pointNearSegment(const CDPoint& point, const CDPoint& segStart,
        const CDPoint& segEnd, double distance) {
        if (distance <= 0.0) {
            // Если distance = 0, проверяем точное попадание на отрезок
            return onSegment(segStart, segEnd, point);
        }

        // Вектор отрезка
        double segVecX = segEnd.x - segStart.x;
        double segVecY = segEnd.y - segStart.y;

        // Вектор от начала отрезка к точке
        double pointVecX = point.x - segStart.x;
        double pointVecY = point.y - segStart.y;

        // Длина отрезка в квадрате
        double segLengthSquared = segVecX * segVecX + segVecY * segVecY;

        // Если отрезок - точка
        if (segLengthSquared < 1e-9) {
            return checkPointsProximity(point, segStart, distance);
        }

        // Проекция точки на отрезок (параметр t)
        double t = (pointVecX * segVecX + pointVecY * segVecY) / segLengthSquared;

        // Ограничиваем t в пределах [0, 1]
        t = (std::max)(0.0, (std::min)(1.0, t));

        // Находим ближайшую точку на отрезке
        CDPoint closestPoint;
        closestPoint.x = segStart.x + t * segVecX;
        closestPoint.y = segStart.y + t * segVecY;

        // Проверяем расстояние до ближайшей точки
        return checkPointsProximity(point, closestPoint, distance);
    }

    inline bool FringesConnected(
        const HelperNode& a, const HelperNode& b,
        double proximityThreshold = 0.5)
    {
        size_t countA = a.m_fringe.GetPointCount();
        size_t countB = b.m_fringe.GetPointCount();

        if (countA == 0 || countB == 0) return false;

        if (a.isSeparateFrom(b, proximityThreshold))
            return false;

        bool isPoint1 = (countA == 1);
        bool isPoint2 = (countB == 1);

        if (isPoint1 && isPoint2) {
            return checkPointsProximity(a.m_fringe.GetPoint(0), b.m_fringe.GetPoint(0), proximityThreshold);
        }

        if (proximityThreshold > 0.0) {
            // Check if any points are within proximity threshold 
            for (size_t i = 0; i < countA; ++i) {
                const CDPoint& pa = a.m_fringe.GetPoint(i);
                for (size_t j = 0; j < countB; ++j) {
                    const CDPoint& pb = b.m_fringe.GetPoint(j);
                    if (pointsAreClose(pa, pb, proximityThreshold)) {
                        return true;
                    }
                }
            }

            // Проверка близости точек к отрезкам
            // Точки первой полилинии к отрезкам второй
            for (size_t i = 0; i < countA; ++i) {
                const CDPoint& point = a.m_fringe.GetPoint(i);
                for (size_t j = 0; j < countB - 1; ++j) {
                    const CDPoint& pb1 = b.m_fringe.GetPoint(j);
                    const CDPoint& pb2 = b.m_fringe.GetPoint(j + 1);
                    if (pointNearSegment(point, pb1, pb2, proximityThreshold)) {
                        return true;
                    }
                }
            }

            // Точки второй полилинии к отрезкам первой
            for (size_t i = 0; i < countB; ++i) {
                const CDPoint& point = b.m_fringe.GetPoint(i);
                for (size_t j = 0; j < countA - 1; ++j) {
                    const CDPoint& pa1 = a.m_fringe.GetPoint(j);
                    const CDPoint& pa2 = a.m_fringe.GetPoint(j + 1);
                    if (pointNearSegment(point, pa1, pa2, proximityThreshold)) {
                        return true;
                    }
                }
            }
        }
        // 3. Проверка пересечения отрезков (обычная проверка)
        if (!isPoint1 && !isPoint2) { // Только если обе полилинии имеют отрезки
            for (size_t i = 0; i < countA - 1; ++i) {
                const CDPoint& pa1 = a.m_fringe.GetPoint(i);
                const CDPoint& pa2 = a.m_fringe.GetPoint(i + 1);
                for (size_t j = 0; j < countB - 1; ++j) {
                    const CDPoint& pb1 = b.m_fringe.GetPoint(j);
                    const CDPoint& pb2 = b.m_fringe.GetPoint(j + 1);
                    if (segmentsIntersect(pa1, pa2, pb1, pb2)) {
                        return true;
                    }
                }
            }
        }

        // 4. Специальные случаи для полилиний-точек
        if (isPoint1) {
            // polyline1 - точка, проверяем её близость к отрезкам polyline2
            const CDPoint& point = a.m_fringe.GetPoint(0);
            for (size_t j = 0; j < countB - 1; ++j) {
                const CDPoint& pb1 = b.m_fringe.GetPoint(j);
                const CDPoint& pb2 = b.m_fringe.GetPoint(j + 1);
                if (pointNearSegment(point, pb1, pb2, proximityThreshold)) {
                    return true;
                }
            }
        }

        if (isPoint2) {
            // polyline2 - точка, проверяем её близость к отрезкам polyline1
            const CDPoint& point = b.m_fringe.GetPoint(0);
            for (size_t i = 0; i < countA - 1; ++i) {
                const CDPoint& pa1 = a.m_fringe.GetPoint(i);
                const CDPoint& pa2 = a.m_fringe.GetPoint(i + 1);
                if (pointNearSegment(point, pa1, pa2, proximityThreshold)) {
                    return true;
                }
            }
        }

        return false;
    }

    inline std::vector<GroupMetadata> MergeConnected(const std::vector<CFringeSegment>& fringes,
        const std::map<size_t, bool>& trustedMap, double step)
    {
        std::vector<std::vector<HelperNode>> mergeGroups;
        std::vector<bool> processed(fringes.size(), false);

        for (size_t i = 0; i < fringes.size(); ++i) {
            if (processed[i]) continue;
            std::vector<HelperNode> group;
            std::queue<HelperNode> q;
            HelperNode n_i(fringes[i], i);

            q.push(n_i);
            processed[i] = true;

            while (!q.empty()) {
                HelperNode n_u = q.front();
                q.pop();
                group.push_back(n_u);

                for (size_t v = 0; v < fringes.size(); ++v) {
                    HelperNode n_v(fringes[v], v);
                    if (!processed[v] && FringesConnected(n_u, n_v)) {
                        processed[v] = true;
                        q.push(n_v);
                    }
                }
            }

            mergeGroups.push_back(group);
        }

        std::vector<GroupMetadata> mergeMetadata;

        // postprocess groupes to derive metadata
        for (auto& group : mergeGroups) {
            CDRect mergedBox = { 1e10, 1e10, -1e10, -1e10 };
            double sumX = 0.0, sumY = 0.0;
            bool hasTrusted = false;
            double k = 0.0;
            size_t primaryIndex = group[0].m_i;
            std::vector<size_t> indices;
            for (const auto& node : group) {
                mergedBox.left = (std::min)(mergedBox.left, node.m_boundingBox.left);
                mergedBox.right = (std::max)(mergedBox.right, node.m_boundingBox.right);
                mergedBox.top = (std::min)(mergedBox.top, node.m_boundingBox.top);
                mergedBox.bottom = (std::max)(mergedBox.bottom, node.m_boundingBox.bottom);
                sumX += node.m_centroidX;
                sumY += node.m_centroidY;
                bool isNodeTrusted = (trustedMap.count(node.m_i) > 0);
                if (isNodeTrusted) {
                    k = std::round(node.m_fringe.GetNumber() / step);
                    primaryIndex = node.m_i; // Override primary index to trusted fringe
                }
                hasTrusted = hasTrusted || isNodeTrusted;
                indices.push_back(node.m_i);
            }
            double count = static_cast<double>(group.size());
            double centroidX = (count > 0) ? (sumX / count) : 0.0;
            double centroidY = (count > 0) ? (sumY / count) : 0.0;
            mergeMetadata.push_back({ mergedBox, centroidX, centroidY, primaryIndex, hasTrusted, k, indices });
        }

        return mergeMetadata;
    }

    /**
     * @brief Resolve band topology: assign monotonic numbers along dominant direction
     * 
     * @param nodes [i/o] Node array to assign numbers to
     * @param fringes [i] Original fringe geometries
     * @param trustedFringeIndices [i] Trusted anchor indices
     * @param step [i] Step value for numbering
     * @return BandResolutionResult with trusted and weak fringe lists
     */
    inline BandResolutionResult ResolveBandTopology(
        std::vector<FringeNode>& nodes,
        const std::vector<CFringeSegment>& fringes,
        const std::vector<size_t>& trustedFringeIndices,
        double step)
    {
        BandResolutionResult result;
        
        // Estimate alignment direction from trusted anchors
        CDPoint alignmentDir = { 1.0, 1.0 };
        if (trustedFringeIndices.size() >= 2) {
            CDPoint p1 = fringes[trustedFringeIndices[0]].GetPoint(0);
            CDPoint p2 = fringes[trustedFringeIndices[1]].GetPoint(0);
            if (fringes[trustedFringeIndices[1]].GetNumber() >
                fringes[trustedFringeIndices[0]].GetNumber()) {
                alignmentDir.x = p2.x - p1.x;
                alignmentDir.y = p2.y - p1.y;
            }
            else {
                alignmentDir.x = p1.x - p2.x;
                alignmentDir.y = p1.y - p2.y;
            }
        }

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

        // Compute principal direction using angle formula
        // For covariance matrix [[a, b], [b, c]]
        // The angle θ of principal axis (eigenvector for largest eigenvalue) is:
        // θ = 0.5 * atan2(2*b, a - c)

        double a = covXX, b = covXY, c = covYY;
        double dirX, dirY;

        if (std::abs(b) < 1e-9 && std::abs(a - c) < 1e-9) {
            // Isotropic (circular) distribution
            dirX = 1.0;
            dirY = 0.0;
        }
        else {
            double angle = 0.5 * std::atan2(2.0 * b, a - c);
            dirX = std::cos(angle);
            dirY = std::sin(angle);
        }

        // Choose direction that matches alignmentDir
        double dot = dirX * alignmentDir.x + dirY * alignmentDir.y;
        if (dot < 0) {
            dirX = -dirX;
            dirY = -dirY;
        }

        // Sort nodes along normal direction
        struct NodeProj { size_t idx; double proj; };
        std::vector<NodeProj> sorted;
        for (size_t n = 0; n < nodes.size(); ++n) {
            NodeProj np;
            np.idx = n;
            np.proj = nodes[n].centroid_x * dirX + nodes[n].centroid_y * dirY;
            sorted.push_back(np);
        }
        std::sort(sorted.begin(), sorted.end(),
            [](const NodeProj& a, const NodeProj& b) { return a.proj < b.proj; });

        // Find first assigned node in sorted order
        int currentK = 0;
        size_t firstAssigned = 0;
        for (size_t s = 0; s < sorted.size(); ++s) {
            size_t anchorIdx = sorted[s].idx;
            if (nodes[anchorIdx].isAssigned) {
                currentK = nodes[anchorIdx].assignedK;
                firstAssigned = s;
                result.trustedFringes.push_back(nodes[anchorIdx].primaryIndex);
                break;
            }
        }

        // Assign numbers monotonically forward
        for (size_t t = firstAssigned + 1; t < sorted.size(); ++t) {
            size_t idx = sorted[t].idx;
            if (nodes[idx].isAssigned) {
                // Check consistency
                int expectedDelta = (nodes[idx].assignedK - currentK);
                if (std::abs(expectedDelta) > 1) {
                    // Inconsistent, mark as weak
                    nodes[idx].isWeak = true;
                    result.weakFringes.push_back(nodes[idx].primaryIndex);
                } else
                    result.trustedFringes.push_back(nodes[idx].primaryIndex);
                currentK = nodes[idx].assignedK;
            }
            else {
                ++currentK;  // Next fringe is +1
                nodes[idx].assignedK = currentK;
                nodes[idx].isAssigned = true;
                nodes[idx].isWeak = false;
                result.trustedFringes.push_back(nodes[idx].primaryIndex);
            }
        }
        
        // Backward propagation
        if (firstAssigned > 0) {
            currentK = nodes[sorted[firstAssigned].idx].assignedK;
            for (int s = static_cast<int>(firstAssigned) - 1; s >= 0; --s) {
                size_t idx = sorted[s].idx;
                --currentK;  // Previous band is -1
                nodes[idx].assignedK = currentK;
                nodes[idx].isAssigned = true;
                nodes[idx].isWeak = false;
                result.trustedFringes.push_back(nodes[idx].primaryIndex);
            }
        }

        return result;
    }

} // namespace impl_saddles

// Main algorithm implementation
inline AutoNumberingResult AutoNumberFringesSaddles(
    std::vector<CFringeSegment>& fringes,
    const std::vector<size_t>& trustedFringeIndices,
    double step)
{
    AutoNumberingResult result;
	result.trustedFringes.clear(); // trustedFringeIndices may be reviewed
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

    std::vector<GroupMetadata> mergeMetadata = MergeConnected(fringes, trustedMap, step);

    // Step 1.2: Create nodes from merge groups
    std::vector<FringeNode> nodes;

    for (size_t g = 0; g < mergeMetadata.size(); ++g) {
        FringeNode node;
        node.indices = mergeMetadata[g].indices;
        node.primaryIndex = mergeMetadata[g].primaryIndex;
        node.centroid_x = mergeMetadata[g].centroidX;
        node.centroid_y = mergeMetadata[g].centroidY;
		node.boundingBox = mergeMetadata[g].boundingBox;

        node.isClosed = std::any_of(node.indices.begin(), node.indices.end(),
            [&fringes](size_t idx) { return IsFringeClosed(fringes[idx]); });
        node.isTrusted = mergeMetadata[g].isTrusted;
        node.knownValue = mergeMetadata[g].knownValue;

        node.isAssigned = node.isTrusted;
        node.assignedK = static_cast<int>(node.knownValue);
        node.isWeak = false;
        node.region = FringeRegion::Unknown;
		node.outerNodeIdx = SIZE_MAX;

        nodes.push_back(node);
    }

    // Step 1.3: Classify regions (Band vs Ring vs Saddle)
    
    TopologyClassification topology = ClassifyTopology(nodes, fringes);
    
    // Determine dominant topology based on classification
    bool isRingDominated = (!topology.ringNodeIndices.empty());

    if (isRingDominated) {
        // ===== PHASE 2: Ring Structure Arrangement =====
        // Rings ordered by inclusion hierarchy using outerNodeIdx
        // Numbering radiates from trusted anchors
        
        const size_t NO_PARENT = SIZE_MAX;
        
        // Step 2.1: Mark all trusted anchors first
        std::vector<size_t> trustedAnchors;
        for (size_t nodeIdx : topology.ringNodeIndices) {
            if (!nodes[nodeIdx].isClosed) continue;
            if (!nodes[nodeIdx].isTrusted) continue;
            trustedAnchors.push_back(nodeIdx);
            result.trustedFringes.push_back(nodes[nodeIdx].primaryIndex);
        }
        
        // Step 2.2: Helper lambda for recursive hierarchy propagation
        auto propagateHierarchy = [&](auto& self, size_t curIdx, int curK) -> void {
            // Find and assign immediate children (rings enclosed by this one)
            for (size_t childIdx : topology.ringNodeIndices) {
                if (!nodes[childIdx].isClosed) continue;
                if (nodes[childIdx].outerNodeIdx == curIdx && !nodes[childIdx].isAssigned) {
                    nodes[childIdx].assignedK = curK - 1;  // Children go inward: K - 1
                    nodes[childIdx].isAssigned = true;
                    nodes[childIdx].isWeak = true;
                    result.weakFringes.push_back(nodes[childIdx].primaryIndex);
                    self(self, childIdx, curK - 1);
                }
            }
            
            // Find and assign parent (ring that encloses this one)
            if (nodes[curIdx].outerNodeIdx != NO_PARENT) {
                size_t parentIdx = nodes[curIdx].outerNodeIdx;
                if (!nodes[parentIdx].isAssigned) {
                    nodes[parentIdx].assignedK = curK + 1;  // Parent goes outward: K + 1
                    nodes[parentIdx].isAssigned = true;
                    nodes[parentIdx].isWeak = true;
                    result.weakFringes.push_back(nodes[parentIdx].primaryIndex);
                    self(self, parentIdx, curK + 1);
                }
            }
        };
        
        // Step 2.3: Propagate from each trusted anchor
        for (size_t anchorIdx : trustedAnchors) {
            propagateHierarchy(propagateHierarchy, anchorIdx, nodes[anchorIdx].assignedK);
        }
        
        // Step 2.4: Process any remaining unassigned rings (not connected to anchors)
        for (size_t nodeIdx : topology.ringNodeIndices) {
            if (nodes[nodeIdx].isClosed && !nodes[nodeIdx].isAssigned) {
                // Find root of this ring's hierarchy (topmost parent)
                size_t rootIdx = nodeIdx;
                while (nodes[rootIdx].outerNodeIdx != NO_PARENT) {
                    rootIdx = nodes[rootIdx].outerNodeIdx;
                }
                
                // Assign starting number from root using hierarchy
                int startK = 0;  // Default starting point
                nodes[rootIdx].assignedK = startK;
                nodes[rootIdx].isAssigned = true;
                nodes[rootIdx].isWeak = true;
                result.weakFringes.push_back(nodes[rootIdx].primaryIndex);
                
                propagateHierarchy(propagateHierarchy, rootIdx, startK);
            }
        }
        
        // Step 2.5: Number bands inside rings (K_band = K_ring + 1)
        if (!topology.bandNodeIndices.empty()) {
            std::vector<FringeNode> bandNodes;
            std::vector<size_t> bandNodeOriginalIdx;
            
            // Collect band nodes and their parent rings
            for (size_t bandIdx : topology.bandNodeIndices) {
                bandNodeOriginalIdx.push_back(bandIdx);
                bandNodes.push_back(nodes[bandIdx]);
            }
            
            // Use ResolveBandTopology to assign band numbers
            BandResolutionResult bandResult = ResolveBandTopology(bandNodes, fringes, trustedFringeIndices, step);
            
            // Map results back to original nodes
            for (size_t i = 0; i < bandNodes.size(); ++i) {
                size_t origIdx = bandNodeOriginalIdx[i];
                nodes[origIdx] = bandNodes[i];
            }
            
            result.trustedFringes.insert(result.trustedFringes.end(), 
                bandResult.trustedFringes.begin(), bandResult.trustedFringes.end());
            result.weakFringes.insert(result.weakFringes.end(), 
                bandResult.weakFringes.begin(), bandResult.weakFringes.end());
        }
    }
    else {
        // ===== PHASE 3a: Band Resolution =====
        
        BandResolutionResult bandResult = ResolveBandTopology(nodes, fringes, trustedFringeIndices, step);
        
        // Merge results into main result
        result.trustedFringes.insert(result.trustedFringes.end(), 
            bandResult.trustedFringes.begin(), bandResult.trustedFringes.end());
        result.weakFringes.insert(result.weakFringes.end(), 
            bandResult.weakFringes.begin(), bandResult.weakFringes.end());
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

                if (dist < 100.0)  // Reasonable threshold for adjacency
                    nodes[m].assignedK = nodes[n].assignedK + 1;
                    nodes[m].isAssigned = true;
                    nodes[m].isWeak = true;
                    result.weakFringes.push_back(nodes[m].primaryIndex);
                    changed = true;
            }
        }
    }

    // ===== PHASE 9: Validation & Output =====
    
    // Apply assigned numbers to all fringes in each node
    // IMPORTANT: Nodes with multiple indices (merged fringes) will have ALL their fringes
    // set to the same number (assignedK * step) from the primary fringe
    for (size_t n = 0; n < nodes.size(); ++n) {
        double newNumber = static_cast<double>(nodes[n].assignedK) * step;
        
        // Set number for ALL fringes in this node (handles merged groups)
        for (size_t fringeIdx : nodes[n].indices) {
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
