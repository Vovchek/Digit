/**
 * @file CApertureCtrls.cpp
 * @brief Implementation of Aperture Subsystem Coordinator
 * 
 * Invariant:
 * All interactive shape edits go through Commands.
 * CApertureCtrls coordinates consistency.
 * ShapeCollection contains geometry only.
 * Visibility mask is cached and invalidated centrally.
 */
#include "stdafx.h"
#include "CApertureCtrls.h"
#include "ImageTempl/ViewTransform.h"
#include "DigitMode/Rendering/ShapeDrawStyle.h"
#include <algorithm>

namespace DigitMode {

// ========================================================================
// Construction / Initialization
// ========================================================================

void CApertureCtrls::Init()
{
    Clear();
}

void CApertureCtrls::Clear()
{
    // ShapeCollection doesn't have a clear() method, so we replace it
    m_shapes = aperture::ShapeCollection();
}

// ========================================================================
// Shape Management (Semantic - Initialization & File Load ONLY)
// ========================================================================

// NOTE: These methods are NOT undo-safe.
// Do not use them from Commands or interactive editing.
// Use ONLY for initialization and file load.

aperture::Shape* CApertureCtrls::AddExternalShape(std::unique_ptr<aperture::Shape> shape)
{
    if (!shape) {
        return nullptr;
    }
    
    auto* ptr = shape.get();
    m_shapes.addExternal(std::move(shape));
    NotifyShapeModified();
    return ptr;
}

aperture::Shape* CApertureCtrls::AddInternalShape(std::unique_ptr<aperture::Shape> shape)
{
    if (!shape) {
        return nullptr;
    }
    
    auto* ptr = shape.get();
    m_shapes.addInternal(std::move(shape));
    NotifyShapeModified();
    return ptr;
}

aperture::Shape* CApertureCtrls::AddApertureShape(std::unique_ptr<aperture::Shape> shape)
{
    if (!shape) {
        return nullptr;
    }
    
    auto* ptr = shape.get();
    m_shapes.addAperture(std::move(shape));
    NotifyShapeModified();
    return ptr;
}

bool CApertureCtrls::RemoveExternalShape(size_t index)
{
    auto& container = const_cast<std::vector<std::unique_ptr<aperture::Shape>>&>(
        m_shapes.getExternal()
    );
    
    if (index >= container.size()) {
        return false;
    }
    
    container.erase(container.begin() + index);
    NotifyShapeModified();
    
    return true;
}

bool CApertureCtrls::RemoveInternalShape(size_t index)
{
    auto& container = const_cast<std::vector<std::unique_ptr<aperture::Shape>>&>(
        m_shapes.getInternal()
    );
    
    if (index >= container.size()) {
        return false;
    }
    
    container.erase(container.begin() + index);
    NotifyShapeModified();
    
    return true;
}

bool CApertureCtrls::RemoveApertureShape(size_t index)
{
    auto& container = const_cast<std::vector<std::unique_ptr<aperture::Shape>>&>(
        m_shapes.getApertures()
    );
    
    if (index >= container.size()) {
        return false;
    }
    
    container.erase(container.begin() + index);
    NotifyShapeModified();
    
    return true;
}

bool CApertureCtrls::RemoveShape(aperture::Shape* shape)
{
    if (!shape) {
        return false;
    }
    
    // Try to find and remove from each container
    size_t index;
    
    if (FindShapeInContainer(shape, m_shapes.getExternal(), index)) {
        return RemoveExternalShape(index);
    }
    
    if (FindShapeInContainer(shape, m_shapes.getInternal(), index)) {
        return RemoveInternalShape(index);
    }
    
    if (FindShapeInContainer(shape, m_shapes.getApertures(), index)) {
        return RemoveApertureShape(index);
    }
    
    return false;
}

aperture::TypeLimits CApertureCtrls::GetShapeType(const aperture::Shape* shape) const
{
    if (!shape) {
        return aperture::TypeLimits::EXTERNAL;
    }
    
    size_t dummy;
    
    if (FindShapeInContainer(shape, m_shapes.getExternal(), dummy)) {
        return aperture::TypeLimits::EXTERNAL;
    }
    
    if (FindShapeInContainer(shape, m_shapes.getInternal(), dummy)) {
        return aperture::TypeLimits::INTERNAL;
    }
    
    if (FindShapeInContainer(shape, m_shapes.getApertures(), dummy)) {
        return aperture::TypeLimits::APERTURE;
    }
    
    return aperture::TypeLimits::EXTERNAL;
}

int CApertureCtrls::GetShapeIndex(const aperture::Shape* shape) const
{
    if (!shape) {
        return -1;
    }
    
    size_t index;
    
    if (FindShapeInContainer(shape, m_shapes.getExternal(), index)) {
        return static_cast<int>(index);
    }
    
    if (FindShapeInContainer(shape, m_shapes.getInternal(), index)) {
        return static_cast<int>(index);
    }
    
    if (FindShapeInContainer(shape, m_shapes.getApertures(), index)) {
        return static_cast<int>(index);
    }
    
    return -1;
}

size_t CApertureCtrls::GetShapeCount() const
{
    return GetExternalCount() + GetInternalCount() + GetApertureCount();
}

// ========================================================================
// Hit Testing
// ========================================================================

CApertureCtrls::HitTestResult CApertureCtrls::HitTest(
    const aperture::Point& worldPt, 
    double tolerance) const
{
    HitTestResult result;
    
    // Helper lambda to test all shapes in a container
    auto testContainer = [&](const std::vector<std::unique_ptr<aperture::Shape>>& container,
                            aperture::TypeLimits type) {
        for (const auto& shape : container) {
            // Test control points (handles) first - highest priority
            int handleIndex = HitTestControlPoints(shape.get(), worldPt, tolerance);
            if (handleIndex >= 0) {
                result.shape = shape.get();
                result.type = type;
                result.controlPointIndex = handleIndex;  // Handle hit
                result.distance = 0.0;
                return true;  // Found handle hit
            }
            
            // Test shape body (contour) as fallback
            // Body hit is treated as handle #0 (Move handle for shape translation)
            if (shape->isOnContour(worldPt, tolerance)) {
                result.shape = shape.get();
                result.type = type;
                result.controlPointIndex = 0;  // Body hit = handle #0 (Move)
                result.distance = 0.0;
                return true;  // Found body hit
            }
        }
        return false;
    };
    
    // Test in order: external, internal, aperture
    if (testContainer(m_shapes.getExternal(), aperture::TypeLimits::EXTERNAL)) {
        return result;
    }
    if (testContainer(m_shapes.getInternal(), aperture::TypeLimits::INTERNAL)) {
        return result;
    }
    if (testContainer(m_shapes.getApertures(), aperture::TypeLimits::APERTURE)) {
        return result;
    }
    
    // No hit
    return result;
}

int CApertureCtrls::HitTestControlPoints(
    aperture::Shape* shape,
    const aperture::Point& worldPt,
    double tolerance) const
{
    if (!shape) {
        return -1;
    }
    
    // Enumerate all handles for this shape
    std::vector<aperture::HandleDesc> handles;
    shape->EnumerateHandles(handles);
    
    // Find the closest handle within tolerance
    int closestHandleIndex = -1;
    double closestDistance = tolerance;
    
    for (size_t i = 0; i < handles.size(); ++i) {
        const auto& handle = handles[i];
        
        // Calculate distance from test point to handle position
        double dx = worldPt.x - handle.localPos.x;
        double dy = worldPt.y - handle.localPos.y;
        double distance = std::sqrt(dx * dx + dy * dy);
        
        // Keep track of closest handle within tolerance
        if (distance < closestDistance) {
            closestDistance = distance;
            closestHandleIndex = static_cast<int>(i);
        }
    }
    
    return closestHandleIndex;
}

bool CApertureCtrls::HitTestShapeBody(
    aperture::Shape* shape,
    const aperture::Point& worldPt,
    double tolerance) const
{
    if (!shape) {
        return false;
    }
    
    return shape->isOnContour(worldPt, tolerance);
}

// ========================================================================
// Visibility Testing
// ========================================================================

bool CApertureCtrls::IsVisible(const aperture::Point& worldPt) const
{
    // Use VisibilityChecker for proper 3-type algorithm
    aperture::VisibilityChecker checker(m_shapes);
    return checker.isVisible(worldPt);
}

void CApertureCtrls::SetVisibilityDomain(int width, int height)
{
    if (m_visibilityDomain.width == width &&
        m_visibilityDomain.height == height)
        return;

    m_visibilityDomain = { width, height };

    m_maskProvider.UpdateImageSize(width, height);
}

// ========================================================================
// Rendering (Phase 3)
// ========================================================================

void CApertureCtrls::DrawShapes(
    CDC& dc,
    const ViewTransform& worldToScreen,
    const aperture::Shape* selectedShape,
    const aperture::Shape* hoveredShape,
    int activeHandleIndex) const
{
    using namespace aperture;
    
    // Helper lambda to render a collection
    auto renderCollection = [&](
        const std::vector<std::unique_ptr<Shape>>& shapes,
        TypeLimits type)
    {
        for (const auto& shapePtr : shapes) {
            if (!shapePtr) continue;
            
            // Compute style based on selection and hover state
            DigitMode::ShapeDrawStyle style;
            style.type = type;
            
            // Determine state and handle visibility
            bool isSelected = (shapePtr.get() == selectedShape);
            bool isHovered = (shapePtr.get() == hoveredShape);
            
            if (isSelected) {
                // Selected: show handles, highlight active handle if dragging
                style.state = DigitMode::ShapeDrawStyle::State::Selected;
                style.showHandles = true;
                style.activeHandleIndex = activeHandleIndex;  // -1 = none, 0+ = specific handle
            } else if (isHovered) {
                // Hovered (but not selected): show handles for easier clicking
                style.state = DigitMode::ShapeDrawStyle::State::Idle;  // Still idle, just showing handles
                style.showHandles = true;
                style.activeHandleIndex = activeHandleIndex;  // Highlight hovered handle
            } else {
                // Neither selected nor hovered: no handles
                style.state = DigitMode::ShapeDrawStyle::State::Idle;
                style.showHandles = false;
                style.activeHandleIndex = -1;
            }
            
            // Render shape
            m_dispatcher.Draw(*shapePtr, dc, style, worldToScreen);
            
            // Render handles if enabled
            if (style.showHandles) {
                m_dispatcher.DrawHandles(*shapePtr, dc, style, worldToScreen);
            }
        }
    };
    
    // Render in order: EXTERNAL → APERTURE → INTERNAL
    // This ensures proper visual layering
    renderCollection(m_shapes.getExternal(), TypeLimits::EXTERNAL);
    renderCollection(m_shapes.getApertures(), TypeLimits::APERTURE);
    renderCollection(m_shapes.getInternal(), TypeLimits::INTERNAL);
}


// ========================================================================
// Internal Helpers
// ========================================================================

const std::vector<std::unique_ptr<aperture::Shape>>* 
CApertureCtrls::GetContainer(aperture::TypeLimits type) const
{
    switch (type) {
        case aperture::TypeLimits::EXTERNAL:
            return &m_shapes.getExternal();
        case aperture::TypeLimits::INTERNAL:
            return &m_shapes.getInternal();
        case aperture::TypeLimits::APERTURE:
            return &m_shapes.getApertures();
        default:
            return nullptr;
    }
}

std::vector<std::unique_ptr<aperture::Shape>>* 
CApertureCtrls::GetContainer(aperture::TypeLimits type)
{
    // Const-cast version
    return const_cast<std::vector<std::unique_ptr<aperture::Shape>>*>(
        const_cast<const CApertureCtrls*>(this)->GetContainer(type)
    );
}

bool CApertureCtrls::FindShapeInContainer(
    const aperture::Shape* shape,
    const std::vector<std::unique_ptr<aperture::Shape>>& container,
    size_t& outIndex) const
{
    for (size_t i = 0; i < container.size(); ++i) {
        if (container[i].get() == shape) {
            outIndex = i;
            return true;
        }
    }
    return false;
}

} // namespace DigitMode
