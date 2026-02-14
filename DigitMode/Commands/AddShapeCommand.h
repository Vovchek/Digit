/**
 * @file AddShapeCommand.h
 * @brief Command to add a new shape to ShapeCollection
 * 
 * ## Command Pattern Contract:
 * - Execute() adds shape to collection via direct access
 * - Undo() removes the added shape
 * - Stores insertion index for precise undo
 * - MUST call apertureCtrls.NotifyShapeModified() after mutation
 * 
 * ## Usage:
 * @code{.cpp}
 * auto shape = std::make_unique<Rectangle>(100, 100, 200, 200);
 * 
 * auto cmd = std::make_unique<AddShapeCommand>(
 *     apertureCtrls, TypeLimits::EXTERNAL, std::move(shape)
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
 * @brief Add a new shape to ShapeCollection
 * 
 * IMPORTANT:
 * Commands modify ShapeCollection directly via index-based access.
 * Commands MUST call apertureCtrls.NotifyShapeModified() after mutation.
 * Commands MUST NOT use semantic APIs like AddExternalShape().
 */
class AddShapeCommand : public Command {
public:
    /**
     * @brief Construct add command
     * @param apertureCtrls Aperture subsystem coordinator
     * @param type Shape type (EXTERNAL/INTERNAL/APERTURE)
     * @param shape Shape to add (ownership transferred)
     */
    AddShapeCommand(
        CApertureCtrls& apertureCtrls,
        aperture::TypeLimits type,
        std::unique_ptr<aperture::Shape> shape);

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Add Shape"; }

private:
    CApertureCtrls& m_apertureCtrls;
    aperture::TypeLimits m_type;
    size_t m_indexInserted;  ///< Index where shape was inserted (for undo)
    std::unique_ptr<aperture::Shape> m_shape;

    // Helper to get mutable container via direct access
    std::vector<std::unique_ptr<aperture::Shape>>& GetContainer();
};

} // namespace DigitMode
