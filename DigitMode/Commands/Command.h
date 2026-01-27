#pragma once

#include <string>

namespace DigitMode {

/**
 * @brief Abstract base class for all commands in the UX v1.0 architecture
 * 
 * Command Pattern Implementation:
 * - All user actions are encapsulated as Command objects
 * - Supports undo/redo through Execute() and Undo()
 * - One user intent = one command = one undo step
 * 
 * @see CommandDispatcher for execution and history management
 */
class Command {
public:
    /**
     * @brief Execute the command (apply changes to data model)
     */
    virtual void Execute() = 0;

    /**
     * @brief Undo the command (revert changes to previous state)
     */
    virtual void Undo() = 0;

    /**
     * @brief Redo the command (default: calls Execute())
     * 
     * Override if redo behavior differs from execute
     */
    virtual void Redo() { Execute(); }

    /**
     * @brief Get human-readable command name for debugging/UI
     * @return Command name (e.g., "Add Dot", "Renumber Segment")
     */
    virtual std::string GetName() const { return "Command"; }

    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Command() = default;
};

} // namespace DigitMode
