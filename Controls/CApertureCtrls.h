/**
 * @file CApertureCtrls.h
 * @brief Aperture Subsystem Coordinator
 * 
 * ## Architectural Role (LOCKED):
 * 
 * CApertureCtrls is the **Aperture Subsystem Coordinator**.
 * 
 * Owns:
 * - ShapeCollection (pure geometry container)
 * - VisibilityMaskProvider (cached mask with lazy rebuild)
 * 
 * Coordinates:
 * - Invalidation when shapes or image change
 * - Hit-testing and read access
 * - Semantic APIs for file load and initialization
 * 
 * Does NOT:
 * - Implement undo/redo (that's Commands)
 * - Handle UI interaction (that's BoundsHandler)
 * 
 * Invariant:
 * All interactive shape edits go through Commands.
 * CApertureCtrls coordinates consistency.
 * ShapeCollection contains geometry only.
 * Visibility mask is cached and invalidated centrally.
 */
#pragma once

#include "ApertureCore\include\aperturecore\visibility\ShapeCollection.h"
#include "ApertureCore\include\aperturecore\visibility\VisibilityChecker.h"
#include "ApertureCore\include\aperturecore\visibility\VisibilityMaskProvider.h"
#include "ApertureCore\include\aperturecore\geometry\Shape.h"
#include "ApertureCore\include\aperturecore\geometry\Ellipse.h"
#include "ApertureCore\include\aperturecore\geometry\Rectangle.h"
#include "ApertureCore\include\aperturecore\geometry\Polygon.h"
#include <memory>
#include <vector>

namespace DigitMode {

struct VisibilityDomain {
    int width = 0;
    int height = 0;
};

/**
 * @brief Aperture Subsystem Coordinator
 * 
 * Owns ShapeCollection and VisibilityMaskProvider.
 * Coordinates invalidation and provides semantic APIs.
 * 
 * IMPORTANT:
 * Shape geometry MUST NOT be modified outside Command::Execute/Undo.
 * Commands modify ShapeCollection directly via index-based access.
 * 
 * Add/Remove methods are semantic, NOT undo-safe.
 * Use them ONLY for initialization and file load.
 */
class CApertureCtrls {
public:
    explicit CApertureCtrls(const IImageData& imageProvider)
        : m_shapes(), m_maskProvider(m_shapes, imageProvider)
    {
        Init();
    }
    ~CApertureCtrls() = default;
    
    void Init();
    void Clear();
    
    // ========================================================================
    // Core Subsystem Access
    // ========================================================================
    
    /**
     * @brief Get shape collection (for Commands to modify directly)
     */
    aperture::ShapeCollection& GetShapes() { return m_shapes; }
    const aperture::ShapeCollection& GetShapes() const { return m_shapes; }
    
    /**
     * @brief Get visibility mask provider
     */
    aperture::visibility::VisibilityMaskProvider& GetMaskProvider() { return m_maskProvider; }
    const aperture::visibility::VisibilityMaskProvider& GetMaskProvider() const { return m_maskProvider; }
    
    // ========================================================================
    // Invalidation Coordination (MANDATORY)
    // ========================================================================
    
    /**
     * @brief Notify that shapes have been modified
     * 
     * MUST be called by Commands after Execute/Undo.
     * Invalidates cached visibility mask.
     */
    void NotifyShapeModified()
    {
        m_shapes.notifyShapeModified();
        m_maskProvider.Invalidate();
    }
    
    /**
     * @brief Notify that image has been modified
     * 
     * Call after:
     * - Image load
     * - Image resize
     * - Image reallocation
     */
    void NotifyImageModified()
    {
        m_maskProvider.Invalidate();
    }
    
    // ========================================================================
    // Semantic APIs (Initialization & File Load ONLY - NOT Undo-Safe)
    // ========================================================================
    
    /**
     * NOTE: These methods are NOT undo-safe.
     * Do not use them from Commands or interactive editing.
     * Use ONLY for initialization and file load.
     */
    
    aperture::Shape* AddExternalShape(std::unique_ptr<aperture::Shape> shape);
    aperture::Shape* AddInternalShape(std::unique_ptr<aperture::Shape> shape);
    aperture::Shape* AddApertureShape(std::unique_ptr<aperture::Shape> shape);
    
    bool RemoveExternalShape(size_t index);
    bool RemoveInternalShape(size_t index);
    bool RemoveApertureShape(size_t index);
    bool RemoveShape(aperture::Shape* shape);
    
    aperture::TypeLimits GetShapeType(const aperture::Shape* shape) const;
    int GetShapeIndex(const aperture::Shape* shape) const;
    
    uint64_t GetVersion() const {
        return m_shapes.getVersion();
    }
    
    size_t GetShapeCount() const;
    size_t GetExternalCount() const { return m_shapes.getExternal().size(); }
    size_t GetInternalCount() const { return m_shapes.getInternal().size(); }
    size_t GetApertureCount() const { return m_shapes.getApertures().size(); }
    
    // ========================================================================
    // Hit Testing (Read-Only Access)
    // ========================================================================
    
    struct HitTestResult {
        aperture::Shape* shape = nullptr;
        aperture::TypeLimits type = aperture::TypeLimits::EXTERNAL;
        int controlPointIndex = -1;  ///< Corner/vertex index (-1 = body)
        double distance = 0.0;
        
        bool hitShape() const { return shape != nullptr; }
        bool hitControlPoint() const { return shape != nullptr && controlPointIndex >= 0; }
        bool hitBody() const { return shape != nullptr && controlPointIndex == -1 && distance >= 0; }
    };
    
    HitTestResult HitTest(const aperture::Point& worldPt, double tolerance) const;
    int HitTestControlPoints(aperture::Shape* shape, const aperture::Point& worldPt, double tolerance) const;
    bool HitTestShapeBody(aperture::Shape* shape, const aperture::Point& worldPt) const;
    
    // ========================================================================
    // Visibility Testing
    // ========================================================================
    
    bool IsVisible(const aperture::Point& worldPt) const;
    void SetVisibilityDomain(int width, int height);

private:
    aperture::ShapeCollection m_shapes;
    mutable aperture::visibility::VisibilityMaskProvider m_maskProvider;
    VisibilityDomain m_visibilityDomain;
    
    const std::vector<std::unique_ptr<aperture::Shape>>* GetContainer(aperture::TypeLimits type) const;
    std::vector<std::unique_ptr<aperture::Shape>>* GetContainer(aperture::TypeLimits type);
    
    bool FindShapeInContainer(const aperture::Shape* shape, 
                             const std::vector<std::unique_ptr<aperture::Shape>>& container,
                             size_t& outIndex) const;
};

} // namespace DigitMode
