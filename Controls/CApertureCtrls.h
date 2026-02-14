/**
 * @file CApertureCtrls.h
 * @brief Modern aperture controls using ApertureCore
 * 
 * Pure modern shape-based API for aperture editing.
 * No legacy CRect/BOUND_TYPE coupling.
 * 
 * ## Design Goals
 * - Shape-based editing aligned with fringe editor
 * - Visibility testing via VisibilityChecker
 * - Minimal MFC coupling
 */
#pragma once

#include "ApertureCore\include\aperturecore\visibility\ShapeCollection.h"
#include "ApertureCore\include\aperturecore\visibility\VisibilityChecker.h"
#include "ApertureCore\include\aperturecore\geometry\Shape.h"
#include "ApertureCore\include\aperturecore\geometry\Ellipse.h"
#include "ApertureCore\include\aperturecore\geometry\Rectangle.h"
#include "ApertureCore\include\aperturecore\geometry\Polygon.h"
#include <memory>
#include <vector>

namespace DigitMode {

/**
 * @brief Simple identifier for a shape in the collection
 */
struct ShapeHandle {
    size_t index = 0;
    aperture::TypeLimits type = aperture::TypeLimits::EXTERNAL;
    
    bool operator==(const ShapeHandle& other) const {
        return index == other.index && type == other.type;
    }
};

/**
 * @brief Modern aperture controls using ApertureCore
 * 
 * Pure shape-based API - no legacy CRect/BOUND_TYPE methods.
 */
class CApertureCtrls {
public:
    CApertureCtrls();
    ~CApertureCtrls() = default;
    
    void Init();
    void Clear();
    
    // ========================================================================
    // Shape Management
    // ========================================================================
    
    ShapeHandle AddExternalShape(std::unique_ptr<aperture::Shape> shape);
    ShapeHandle AddInternalShape(std::unique_ptr<aperture::Shape> shape);
    ShapeHandle AddApertureShape(std::unique_ptr<aperture::Shape> shape);
    bool RemoveShape(const ShapeHandle& handle);
    
    const aperture::Shape* GetShape(const ShapeHandle& handle) const;
    aperture::Shape* GetShapeForEdit(const ShapeHandle& handle);
    
    void NotifyShapeModified() {
        m_shapes.notifyShapeModified();
    }
    
    uint64_t GetVersion() const {
        return m_shapes.getVersion();
    }
    
    size_t GetShapeCount() const;
    size_t GetExternalCount() const { return m_shapes.getExternal().size(); }
    size_t GetInternalCount() const { return m_shapes.getInternal().size(); }
    size_t GetApertureCount() const { return m_shapes.getApertures().size(); }
    
    // ========================================================================
    // Hit Testing (Modern API)
    // ========================================================================
    
    struct HitTestResult {
        ShapeHandle shape;
        int controlPointIndex = -1;  ///< Corner/vertex index (-1 = body)
        double distance = 0.0;
        
        bool hitShape() const { return controlPointIndex != -1 || distance >= 0; }
        bool hitControlPoint() const { return controlPointIndex >= 0; }
        bool hitBody() const { return controlPointIndex == -1 && distance >= 0; }
    };
    
    HitTestResult HitTest(const aperture::Point& worldPt, double tolerance) const;
    int HitTestControlPoints(const ShapeHandle& handle, const aperture::Point& worldPt, double tolerance) const;
    bool HitTestShapeBody(const ShapeHandle& handle, const aperture::Point& worldPt) const;
    
    // ========================================================================
    // Editing State
    // ========================================================================
    
    void BeginEdit(const ShapeHandle& handle, int controlPointIndex = -1);
    void UpdateEdit(const aperture::Point& worldDelta);
    void CommitEdit();
    void CancelEdit();
    
    bool IsEditing() const { return m_isEditing; }
    const ShapeHandle& GetEditHandle() const { return m_editHandle; }
    
    // ========================================================================
    // Visibility Testing
    // ========================================================================
    
    bool IsVisible(const aperture::Point& worldPt) const;
    
    const aperture::ShapeCollection& GetShapes() const { return m_shapes; }
    aperture::ShapeCollection& GetShapes() { return m_shapes; }
    
private:
    aperture::ShapeCollection m_shapes;
    
    // Editing state
    bool m_isEditing = false;
    ShapeHandle m_editHandle;
    int m_editControlPointIndex = -1;
    std::unique_ptr<aperture::Shape> m_editSnapshot;
    
    bool IsHandleValid(const ShapeHandle& handle) const;
    const std::vector<std::unique_ptr<aperture::Shape>>* GetContainer(aperture::TypeLimits type) const;
    std::vector<std::unique_ptr<aperture::Shape>>* GetContainer(aperture::TypeLimits type);
};

} // namespace DigitMode
