/**
 * @file ReplaceShapeCommand.h
 * @brief Command to replace a shape in ShapeCollection with a modified version
 * 
 * CORE COMMAND for all shape editing operations.
 * All drag/resize/rotate operations delegate to this command.
 * 
 * ## Command Pattern Contract:
 * - Execute() replaces shape at (type, index) with after-state
 * - Undo() restores before-state
 * - Shapes are deep-copied (no mutation in place)
 * - MUST call apertureCtrls.NotifyShapeModified() after mutation
 * 
 * ## Usage:
 * @code{.cpp}
 * // User drags a handle, BoundsHandler creates preview
 * auto before = shape->clone();  // Before drag
 * auto after = preview->clone(); // After drag
 * 
 * auto cmd = std::make_unique<ReplaceShapeCommand>(
 *     apertureCtrls, TypeLimits::EXTERNAL, 0, 
 *     std::move(before), std::move(after)
 * );
 * dispatcher.Execute(std::move(cmd));
 * @endcode
 */
#pragma once

#include "Command.h"
#include "ApertureCore/include/aperturecore/visibility/TypeLimits.h"
#include "ApertureCore/include/aperturecore/geometry/Shape.h"
#include <memory>

// Forward declaration
namespace DigitMode {
    class CApertureCtrls;
}

namespace DigitMode {

/**
 * @brief Replace a shape in ShapeCollection (foundation for all shape edits)
 * 
 * IMPORTANT:
 * Commands modify ShapeCollection directly via index-based access.
 * Commands MUST call apertureCtrls.NotifyShapeModified() after mutation.
 * Commands MUST NOT use semantic APIs like AddExternalShape().
 */
class ReplaceShapeCommand : public Command {
public:
    /**
     * @brief Construct replace command
     * @param apertureCtrls Aperture subsystem coordinator
     * @param type Shape type (EXTERNAL/INTERNAL/APERTURE)
     * @param index Index in type-specific container
     * @param before Deep copy of shape before modification
     * @param after Deep copy of shape after modification
     */
    ReplaceShapeCommand(
        CApertureCtrls& apertureCtrls,
        aperture::TypeLimits type,
        size_t index,
        std::unique_ptr<aperture::Shape> before,
        std::unique_ptr<aperture::Shape> after);

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Replace Shape"; }

private:
    CApertureCtrls& m_apertureCtrls;
    aperture::TypeLimits m_type;
    size_t m_index;

    std::unique_ptr<aperture::Shape> m_before;
    std::unique_ptr<aperture::Shape> m_after;

    // Helper to get mutable container via direct access
    std::vector<std::unique_ptr<aperture::Shape>>& GetContainer();
};

} // namespace DigitMode
