/**
 * @file RemoveShapeCommand.cpp
 * @brief Implementation of RemoveShapeCommand
 * 
 * Commands modify ShapeCollection directly via index-based access.
 * Commands MUST call apertureCtrls.NotifyShapeModified() after mutation.
 * Commands MUST NOT use semantic APIs like RemoveExternalShape().
 */
#include "stdafx.h"
#include "RemoveShapeCommand.h"
#include "../Controls/CApertureCtrls.h"

namespace DigitMode {

RemoveShapeCommand::RemoveShapeCommand(
    CApertureCtrls& apertureCtrls,
    aperture::TypeLimits type,
    size_t index)
    : m_apertureCtrls(apertureCtrls)
    , m_type(type)
    , m_index(index)
    , m_backup(nullptr)
{
}

void RemoveShapeCommand::Execute()
{
    auto& container = GetContainer();
    
    if (m_index >= container.size()) {
        return;  // Invalid index
    }
    
    // Backup shape for undo (deep copy)
    m_backup = container[m_index]->clone();
    
    // Remove shape from collection (direct container access)
    container.erase(container.begin() + m_index);
    
    // MANDATORY: Notify coordinator to invalidate cached mask
    m_apertureCtrls.NotifyShapeModified();
}

void RemoveShapeCommand::Undo()
{
    if (!m_backup) {
        return;  // Nothing to restore
    }
    
    auto& container = GetContainer();
    
    // Reinsert shape at original index (direct container access)
    if (m_index <= container.size()) {
        container.insert(container.begin() + m_index, m_backup->clone());
        
        // MANDATORY: Notify coordinator to invalidate cached mask
        m_apertureCtrls.NotifyShapeModified();
    }
}

std::vector<std::unique_ptr<aperture::Shape>>& RemoveShapeCommand::GetContainer()
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
