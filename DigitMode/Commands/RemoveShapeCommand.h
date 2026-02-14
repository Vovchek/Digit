/**
 * @file RemoveShapeCommand.h
 * @brief Command to remove a shape from ShapeCollection
 * 
 * ## Command Pattern Contract:
 * - Execute() removes shape via direct access and backs it up
 * - Undo() reinserts shape at original index
 * - Backup is deep copy for safe restoration
 * - MUST call apertureCtrls.NotifyShapeModified() after mutation
 * 
 * ## Usage:
 * @code{.cpp}
 * auto cmd = std::make_unique<RemoveShapeCommand>(
 *     apertureCtrls, TypeLimits::EXTERNAL, 0
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
 * @brief Remove a shape from ShapeCollection
 * 
 * IMPORTANT:
 * Commands modify ShapeCollection directly via index-based access.
 * Commands MUST call apertureCtrls.NotifyShapeModified() after mutation.
 * Commands MUST NOT use semantic APIs like RemoveExternalShape().
 */
class RemoveShapeCommand : public Command {
public:
    /**
     * @brief Construct remove command
     * @param apertureCtrls Aperture subsystem coordinator
     * @param type Shape type (EXTERNAL/INTERNAL/APERTURE)
     * @param index Index in type-specific container
     */
    RemoveShapeCommand(
        CApertureCtrls& apertureCtrls,
        aperture::TypeLimits type,
        size_t index);

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Remove Shape"; }

private:
    CApertureCtrls& m_apertureCtrls;
    aperture::TypeLimits m_type;
    size_t m_index;
    std::unique_ptr<aperture::Shape> m_backup;  ///< Deep copy for restoration

    // Helper to get mutable container via direct access
    std::vector<std::unique_ptr<aperture::Shape>>& GetContainer();
};

} // namespace DigitMode
