/**
 * @file ShapeCollection.h
 * @brief Container for shapes organized by TypeLimits
 */
#pragma once

#include "../geometry/Shape.h"
#include "../geometry/Bounds.h"
#include "TypeLimits.h"
#include <vector>
#include <memory>

namespace aperture {

/**
 * @brief Collection of shapes organized by visibility type
 * 
 * Stores shapes in separate containers based on TypeLimits for efficient
 * visibility checking. Provides queries for shape management and bounds.
 */
class ShapeCollection {
public:
    ShapeCollection() = default;
    ~ShapeCollection() = default;
    
    // Non-copyable (shapes are unique_ptr)
    ShapeCollection(const ShapeCollection&) = delete;
    ShapeCollection& operator=(const ShapeCollection&) = delete;
    
    // Movable
    ShapeCollection(ShapeCollection&&) = default;
    ShapeCollection& operator=(ShapeCollection&&) = default;
    
    // Add shapes
    
    /**
     * @brief Add shape to collection (auto-categorized by TypeLimits)
     * @param shape Shape to add (ownership transferred)
     */
    void addShape(std::unique_ptr<Shape> shape);
    
    /**
     * @brief Add EXTERNAL shape (aperture)
     * @param shape Shape to add (TypeLimits set to EXTERNAL)
     */
    void addExternal(std::unique_ptr<Shape> shape);
    
    /**
     * @brief Add INTERNAL shape (obstruction)
     * @param shape Shape to add (TypeLimits set to INTERNAL)
     */
    void addInternal(std::unique_ptr<Shape> shape);
    
    /**
     * @brief Add APERTURE shape (opening)
     * @param shape Shape to add (TypeLimits set to APERTURE)
     */
    void addAperture(std::unique_ptr<Shape> shape);
    
    // Query shapes by type
    
    /**
     * @brief Get all EXTERNAL shapes
     * @return Const reference to EXTERNAL shapes container
     */
    const std::vector<std::unique_ptr<Shape>>& getExternal() const {
        return external_;
    }
    
    /**
     * @brief Get all INTERNAL shapes
     * @return Const reference to INTERNAL shapes container
     */
    const std::vector<std::unique_ptr<Shape>>& getInternal() const {
        return internal_;
    }
    
    /**
     * @brief Get all APERTURE shapes
     * @return Const reference to APERTURE shapes container
     */
    const std::vector<std::unique_ptr<Shape>>& getApertures() const {
        return apertures_;
    }
    
    // Collection queries
    
    /**
     * @brief Check if collection has any EXTERNAL shapes
     * @return true if at least one EXTERNAL shape exists
     */
    bool hasAnyExternal() const {
        return !external_.empty();
    }
    
    /**
     * @brief Get total number of shapes
     * @return Sum of shapes in all categories
     */
    size_t totalCount() const {
        return external_.size() + internal_.size() + apertures_.size();
    }
    
    /**
     * @brief Check if collection is empty
     * @return true if no shapes in any category
     */
    bool isEmpty() const {
        return totalCount() == 0;
    }
    
    /**
     * @brief Get combined bounds of all shapes
     * @return Bounds containing all shapes, or empty bounds if no shapes
     */
    Bounds getCombinedBounds() const;
    
    /**
     * @brief Clear all shapes
     */
    void clear();
    
    /**
     * @brief Get count of shapes by type
     * @param type TypeLimits to count
     * @return Number of shapes with specified type
     */
    size_t countByType(TypeLimits type) const;

private:
    std::vector<std::unique_ptr<Shape>> external_;   ///< EXTERNAL shapes
    std::vector<std::unique_ptr<Shape>> internal_;   ///< INTERNAL shapes
    std::vector<std::unique_ptr<Shape>> apertures_;  ///< APERTURE shapes
};

} // namespace aperture
