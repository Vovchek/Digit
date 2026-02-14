/**
 * @file CApertureCtrls.cpp
 * @brief Implementation of modern aperture controls
 */
#include "stdafx.h"
#include "CApertureCtrls.h"
#include <algorithm>

namespace DigitMode {

// ========================================================================
// Construction / Initialization
// ========================================================================

CApertureCtrls::CApertureCtrls()
{
    Init();
}

void CApertureCtrls::Init()
{
    Clear();
}

void CApertureCtrls::Clear()
{
    // Cancel any active edit
    if (IsEditing()) {
        CancelEdit();
    }
    
    // ShapeCollection doesn't have a clear() method, so we replace it
    m_shapes = aperture::ShapeCollection();
}

// ========================================================================
// Shape Management
// ========================================================================

ShapeHandle CApertureCtrls::AddExternalShape(std::unique_ptr<aperture::Shape> shape)
{
    if (!shape) {
        return ShapeHandle{0, aperture::TypeLimits::EXTERNAL};
    }
    
    size_t index = m_shapes.getExternal().size();
    m_shapes.addExternal(std::move(shape));
    
    return ShapeHandle{index, aperture::TypeLimits::EXTERNAL};
}

ShapeHandle CApertureCtrls::AddInternalShape(std::unique_ptr<aperture::Shape> shape)
{
    if (!shape) {
        return ShapeHandle{0, aperture::TypeLimits::INTERNAL};
    }
    
    size_t index = m_shapes.getInternal().size();
    m_shapes.addInternal(std::move(shape));
    
    return ShapeHandle{index, aperture::TypeLimits::INTERNAL};
}

ShapeHandle CApertureCtrls::AddApertureShape(std::unique_ptr<aperture::Shape> shape)
{
    if (!shape) {
        return ShapeHandle{0, aperture::TypeLimits::APERTURE};
    }
    
    size_t index = m_shapes.getApertures().size();
    m_shapes.addAperture(std::move(shape));
    
    return ShapeHandle{index, aperture::TypeLimits::APERTURE};
}

bool CApertureCtrls::RemoveShape(const ShapeHandle& handle)
{
    // TODO: Implement removal
    // ShapeCollection doesn't have remove yet, will need to add
    // For now, return false
    return false;
}

const aperture::Shape* CApertureCtrls::GetShape(const ShapeHandle& handle) const
{
    if (!IsHandleValid(handle)) {
        return nullptr;
    }
    
    const auto* container = GetContainer(handle.type);
    if (!container || handle.index >= container->size()) {
        return nullptr;
    }
    
    return (*container)[handle.index].get();
}

aperture::Shape* CApertureCtrls::GetShapeForEdit(const ShapeHandle& handle)
{
    if (!IsHandleValid(handle)) {
        return nullptr;
    }
    
    auto* container = GetContainer(handle.type);
    if (!container || handle.index >= container->size()) {
        return nullptr;
    }
    
    return (*container)[handle.index].get();
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
    result.distance = -1.0; // No hit
    
    // Helper lambda to test all shapes in a container
    auto testContainer = [&](const std::vector<std::unique_ptr<aperture::Shape>>& container,
                            aperture::TypeLimits type) {
        for (size_t i = 0; i < container.size(); ++i) {
            const auto& shape = container[i];
            
            // TODO: Test control points first (corners, vertices)
            // For now, just test shape body
            
            if (shape->isInside(worldPt)) {
                result.shape = ShapeHandle{i, type};
                result.controlPointIndex = -1;  // Body hit
                result.distance = 0.0;
                return true;  // Found exact hit
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
    const ShapeHandle& handle,
    const aperture::Point& worldPt,
    double tolerance) const
{
    const auto* shape = GetShape(handle);
    if (!shape) {
        return -1;
    }
    
    // TODO: Implement control point hit testing
    // Will need to query shape for corner/vertex positions
    // For rectangle: test 4 corners
    // For polygon: test N vertices
    
    return -1;  // Not implemented yet
}

bool CApertureCtrls::HitTestShapeBody(
    const ShapeHandle& handle,
    const aperture::Point& worldPt) const
{
    const auto* shape = GetShape(handle);
    if (!shape) {
        return false;
    }
    
    return shape->isInside(worldPt);
}

// ========================================================================
// Editing State
// ========================================================================

void CApertureCtrls::BeginEdit(const ShapeHandle& handle, int controlPointIndex)
{
    if (!IsHandleValid(handle)) {
        return;
    }
    
    // Cancel any existing edit
    if (IsEditing()) {
        CancelEdit();
    }
    
    // Get shape to edit
    const auto* shape = GetShape(handle);
    if (!shape) {
        return;
    }
    
    // Save snapshot for cancel
    m_editSnapshot = shape->clone();
    m_editHandle = handle;
    m_editControlPointIndex = controlPointIndex;
    m_isEditing = true;
}

void CApertureCtrls::UpdateEdit(const aperture::Point& worldDelta)
{
    if (!IsEditing()) {
        return;
    }
    
    auto* shape = GetShapeForEdit(m_editHandle);
    if (!shape) {
        CancelEdit();
        return;
    }
    
    // TODO: Implement delta-based shape modification
    // For now, just notify
    NotifyShapeModified();
}

void CApertureCtrls::CommitEdit()
{
    if (!IsEditing()) {
        return;
    }
    
    // Discard snapshot and clear edit state
    m_editSnapshot.reset();
    m_isEditing = false;
    m_editControlPointIndex = -1;
}

void CApertureCtrls::CancelEdit()
{
    if (!IsEditing()) {
        return;
    }
    
    // Restore snapshot
    if (m_editSnapshot) {
        auto* shape = GetShapeForEdit(m_editHandle);
        if (shape) {
            // TODO: Copy snapshot back to original
            // This requires replacing the unique_ptr in the container
        }
        m_editSnapshot.reset();
    }
    
    m_isEditing = false;
    m_editControlPointIndex = -1;
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

// ========================================================================
// Internal Helpers
// ========================================================================

bool CApertureCtrls::IsHandleValid(const ShapeHandle& handle) const
{
    const auto* container = GetContainer(handle.type);
    if (!container) {
        return false;
    }
    
    return handle.index < container->size();
}

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

} // namespace DigitMode
