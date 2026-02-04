#pragma once

/**
 * @file AutoNumberingAlgorithm_Phase2.h
 * @brief Phase 2 Implementation: Adjacency Graph Construction (Topology-First)
 * 
 * This header implements Phase 2 of the auto-numbering algorithm:
 * - Structure classification (parallel bands vs. nested rings vs. mixed)
 * - Topology-first adjacency graph construction
 * - NO hard distance thresholds for gating decisions
 * 
 * Reference: Docs/autonumbering_phase2_refine.md
 */

#include "DigitMode/CFringeSegment.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <set>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace DigitMode {
namespace AutoNumber {

// ============================================================================
// Phase 1: FringeNode (Preprocessing Output)
// ============================================================================

/**
 * @brief Fringe node with preprocessing data (input to Phase 2)
 * 
 * Computes geometry summary and quality metrics from fringe segments
 */
struct FringeNode {
    size_t index;                           ///< Index into Fringes array
    double knownValue;                      ///< Trusted number (if isTrusted)
    bool isTrusted;                         ///< From trustedFringeIndices
    
    double centroid_x, centroid_y;          ///< Centroid position
    bool isClosed;                          ///< Is closed curve
    double closureScore;                    ///< [0,1] how "closed"
    double representativeTangent_x;         ///< Tangent direction (unit)
    double representativeTangent_y;
    
    double confidence;                      ///< Confidence [0,1] (output)
};

// ============================================================================
// Phase 2: Adjacency Graph Structures
// ============================================================================

/**
 * @brief Adjacency edge (output of Phase 2)
 * 
 * MANDATORY: sign determined by topology/geometry, weight by confidence
 * NO: distance thresholds for gating
 */
struct AdjacencyEdge {
    size_t i, j;            ///< Node indices (i < j)
    double weight;          ///< Confidence [0,1], NOT from distance gating
    int sign;               ///< +1, -1, or 0 (unknown)
    double distance;        ///< Informational only
    double overlapLength;   ///< Informational only
};

/**
 * @brief Structure classification (soft, no hard branching)
 */
struct StructureClassification {
    enum Type { PARALLEL_BANDS, NESTED_RINGS, MIXED } type;
    double closedRatio;
    double parallelScore, nestedScore;
    double dominantTangent_x, dominantTangent_y;
    double dominantNormal_x, dominantNormal_y;
};

// ============================================================================
// Phase 2 Implementation
// ============================================================================

namespace impl {

/**
 * @brief Phase 2.1 — Classify structure
 * 
 * Uses closed ratio and mean tangent direction (soft classification)
 */
inline StructureClassification ClassifyStructure(
    const std::vector<CFringeSegment>& fringes,
    const std::vector<FringeNode>& nodes)
{
    StructureClassification result;
    
    // Closed ratio
    int closedCount = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].isClosed) closedCount++;
    }
    result.closedRatio = nodes.empty() ? 0.5 : static_cast<double>(closedCount) / nodes.size();
    
    // Mean tangent direction
    double meanTx = 0.0, meanTy = 0.0;
    int validCount = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        double mag = std::sqrt(nodes[i].representativeTangent_x * nodes[i].representativeTangent_x +
                               nodes[i].representativeTangent_y * nodes[i].representativeTangent_y);
        if (mag > 1e-6) {
            meanTx += nodes[i].representativeTangent_x / mag;
            meanTy += nodes[i].representativeTangent_y / mag;
            validCount++;
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
    if (mag > 1e-6) {
        result.dominantTangent_x = meanTx / mag;
        result.dominantTangent_y = meanTy / mag;
    } else {
        result.dominantTangent_x = 1.0;
        result.dominantTangent_y = 0.0;
    }
    
    result.dominantNormal_x = -result.dominantTangent_y;
    result.dominantNormal_y = result.dominantTangent_x;
    
    result.parallelScore = 1.0 - result.closedRatio;
    result.nestedScore = result.closedRatio;
    
    if (result.closedRatio > 0.6) {
        result.type = StructureClassification::NESTED_RINGS;
    } else if (result.parallelScore > 0.6) {
        result.type = StructureClassification::PARALLEL_BANDS;
    } else {
        result.type = StructureClassification::MIXED;
    }
    
    return result;
}

/**
 * @brief Phase 2.2 — Parallel-band adjacency
 * 
 * Project onto normal, sort, connect consecutive fringes
 */
inline std::vector<AdjacencyEdge> BuildParallelBandAdjacency(
    const std::vector<FringeNode>& nodes,
    double normalX, double normalY)
{
    std::vector<AdjacencyEdge> edges;
    
    if (nodes.empty()) return edges;
    
    // Project and sort
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
    
    // Connect consecutive
    for (size_t k = 0; k + 1 < projs.size(); ++k) {
        size_t i = projs[k].idx;
        size_t j = projs[k+1].idx;
        if (i > j) std::swap(i, j);
        
        AdjacencyEdge edge;
        edge.i = i;
        edge.j = j;
        edge.weight = 1.0;
        edge.sign = 0;
        edge.distance = std::sqrt(
            std::pow(nodes[i].centroid_x - nodes[j].centroid_x, 2) +
            std::pow(nodes[i].centroid_y - nodes[j].centroid_y, 2)
        );
        edge.overlapLength = -1.0;
        
        edges.push_back(edge);
    }
    
    return edges;
}

/**
 * @brief Phase 2.3 — Nested-ring adjacency
 * 
 * Centroid distance heuristic for nesting
 */
inline std::vector<AdjacencyEdge> BuildNestedRingAdjacency(
    const std::vector<CFringeSegment>& fringes,
    const std::vector<FringeNode>& nodes)
{
    std::vector<AdjacencyEdge> edges;
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (!nodes[i].isClosed) continue;
        for (size_t j = 0; j < nodes.size(); ++j) {
            if (i == j || !nodes[j].isClosed) continue;
            
            double di = std::sqrt(nodes[i].centroid_x * nodes[i].centroid_x +
                                  nodes[i].centroid_y * nodes[i].centroid_y);
            double dj = std::sqrt(nodes[j].centroid_x * nodes[j].centroid_x +
                                  nodes[j].centroid_y * nodes[j].centroid_y);
            
            if (di < dj) {
                size_t a = i, b = j;
                if (a > b) std::swap(a, b);
                
                AdjacencyEdge edge;
                edge.i = a;
                edge.j = b;
                edge.weight = 1.0;
                edge.sign = -1;
                edge.distance = std::abs(dj - di);
                edge.overlapLength = -1.0;
                
                edges.push_back(edge);
            }
        }
    }
    
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
 * @brief Phase 2.4 — Mixed/fallback adjacency
 * 
 * Nearest neighbor along normal (no distance gating)
 */
inline std::vector<AdjacencyEdge> BuildMixedAdjacency(
    const std::vector<FringeNode>& nodes,
    double normalX, double normalY)
{
    std::vector<AdjacencyEdge> edges;
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        double bestDistPlus = 1e99, bestDistMinus = 1e99;
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
            size_t a = i, b = nearestPlus;
            if (a > b) std::swap(a, b);
            
            AdjacencyEdge edge;
            edge.i = a;
            edge.j = b;
            edge.weight = 1.0 / (1.0 + 0.01 * bestDistPlus);
            edge.sign = 0;
            edge.distance = bestDistPlus;
            edge.overlapLength = -1.0;
            edges.push_back(edge);
        }
        
        if (nearestMinus != ~0u && nearestMinus != nearestPlus) {
            size_t a = i, b = nearestMinus;
            if (a > b) std::swap(a, b);
            
            AdjacencyEdge edge;
            edge.i = a;
            edge.j = b;
            edge.weight = 1.0 / (1.0 + 0.01 * bestDistMinus);
            edge.sign = 0;
            edge.distance = bestDistMinus;
            edge.overlapLength = -1.0;
            edges.push_back(edge);
        }
    }
    
    return edges;
}

/**
 * @brief Phase 2.5 — Post-processing
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
 * @brief Phase 2 Main — Route to appropriate builder
 */
inline std::vector<AdjacencyEdge> BuildAdjacencyGraph(
    const std::vector<CFringeSegment>& fringes,
    const std::vector<FringeNode>& nodes)
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
    }
    
    // Ensure connectivity with fallback
    if (edges.size() < nodes.size() - 1) {
        std::vector<AdjacencyEdge> fallback = BuildMixedAdjacency(nodes,
            classification.dominantNormal_x,
            classification.dominantNormal_y);
        edges.insert(edges.end(), fallback.begin(), fallback.end());
    }
    
    edges = PostProcessEdges(edges);
    
    return edges;
}

} // namespace impl

} // namespace AutoNumber
} // namespace DigitMode
