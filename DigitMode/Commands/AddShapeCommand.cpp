/**
 * @file AddShapeCommand.cpp
 * @brief Implementation of AddShapeCommand
 * 
 * Commands modify ShapeCollection directly via index-based access.
 * Commands MUST call apertureCtrls.NotifyShapeModified() after mutation.
 * Commands MUST NOT use semantic APIs like AddExternalShape().
 */
#include "stdafx.h"
#include "AddShapeCommand.h"
#include "../Controls/CApertureCtrls.h"

namespace DigitMode {

AddShapeCommand::AddShapeCommand(
    CApertureCtrls& apertureCtrls,
    aperture::TypeLimits type,
    std::unique_ptr<aperture::Shape> shape)
    : m_apertureCtrls(apertureCtrls)
    , m_type(type)
    , m_indexInserted(0)
    , m_shape(std::move(shape))
{
}

void AddShapeCommand::Execute()
{
    auto& container = GetContainer();
    
    // Store index before insertion for undo
    m_indexInserted = container.size();
    
    // Add shape to collection (direct container access)
    container.push_back(m_shape->clone());
    
    // MANDATORY: Notify coordinator to invalidate cached mask
    m_apertureCtrls.NotifyShapeModified();
}

void AddShapeCommand::Undo()
{
    auto& container = GetContainer();
    
    if (m_indexInserted < container.size()) {
        // Remove the shape that was added (direct container access)
        container.erase(container.begin() + m_indexInserted);
        
        // MANDATORY: Notify coordinator to invalidate cached mask
        m_apertureCtrls.NotifyShapeModified();
    }
}

std::vector<std::unique_ptr<aperture::Shape>>& AddShapeCommand::GetContainer()
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
