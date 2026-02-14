/**
 * @file ReplaceShapeCommand.cpp
 * @brief Implementation of ReplaceShapeCommand
 * 
 * Commands modify ShapeCollection directly via index-based access.
 * Commands MUST call apertureCtrls.NotifyShapeModified() after mutation.
 */
#include "stdafx.h"
#include "ReplaceShapeCommand.h"
#include "../Controls/CApertureCtrls.h"

namespace DigitMode {

ReplaceShapeCommand::ReplaceShapeCommand(
    CApertureCtrls& apertureCtrls,
    aperture::TypeLimits type,
    size_t index,
    std::unique_ptr<aperture::Shape> before,
    std::unique_ptr<aperture::Shape> after)
    : m_apertureCtrls(apertureCtrls)
    , m_type(type)
    , m_index(index)
    , m_before(std::move(before))
    , m_after(std::move(after))
{
}

void ReplaceShapeCommand::Execute()
{
    auto& container = GetContainer();
    
    if (m_index >= container.size()) {
        return;  // Invalid index
    }
    
    // Replace shape with after-state (direct container access)
    container[m_index] = m_after->clone();
    
    // MANDATORY: Notify coordinator to invalidate cached mask
    m_apertureCtrls.NotifyShapeModified();
}

void ReplaceShapeCommand::Undo()
{
    auto& container = GetContainer();
    
    if (m_index >= container.size()) {
        return;  // Invalid index
    }
    
    // Restore before-state (direct container access)
    container[m_index] = m_before->clone();
    
    // MANDATORY: Notify coordinator to invalidate cached mask
    m_apertureCtrls.NotifyShapeModified();
}

std::vector<std::unique_ptr<aperture::Shape>>& ReplaceShapeCommand::GetContainer()
{
    auto& shapes = m_apertureCtrls.GetShapes();
    
    switch (m_type) {
        case aperture::TypeLimits::EXTERNAL:
            return const_cast<std::vector<std::unique_ptr<aperture::Shape>>&>(
                shapes.getExternal()
            );
        case aperture::TypeLimits::INTERNAL:
            return const_cast<std::vector<std::unique_ptr<aperture::Shape>>&>(
                shapes.getInternal()
            );
        case aperture::TypeLimits::APERTURE:
            return const_cast<std::vector<std::unique_ptr<aperture::Shape>>&>(
                shapes.getApertures()
            );
        default:
            // Fallback to external (should never happen)
            return const_cast<std::vector<std::unique_ptr<aperture::Shape>>&>(
                shapes.getExternal()
            );
    }
}

} // namespace DigitMode
