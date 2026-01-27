#pragma once

#include "Commands/Command.h"
#include <vector>
#include <memory>
#include <string>

namespace DigitMode {

/**
 * @brief Manages command execution and undo/redo history
 * 
 * Design Principles:
 * - One user intent = one command = one undo step
 * - Commands are executed immediately and added to undo stack
 * - New command clears redo stack (breaks redo chain)
 * - Uses std::unique_ptr for automatic memory management
 * 
 * @note All commands must be heap-allocated and passed as unique_ptr
 */
class CommandDispatcher {
private:
    std::vector<std::unique_ptr<Command>> undoStack;
    std::vector<std::unique_ptr<Command>> redoStack;

public:
    /**
     * @brief Default constructor
     */
    CommandDispatcher() = default;

    /**
     * @brief Destructor (unique_ptr handles cleanup automatically)
     */
    ~CommandDispatcher() = default;

    // Disable copy (unique_ptr is not copyable)
    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;

    // ===== Core Operations =====

    /**
     * @brief Execute a command and add to undo history
     * @param cmd Command to execute (ownership transferred)
     * 
     * Side effects:
     * - Executes command immediately
     * - Adds to undo stack
     * - Clears redo stack (new command breaks redo chain)
     * - Triggers UI update (via OnStateChanged)
     */
    void Execute(std::unique_ptr<Command> cmd);

    /**
     * @brief Undo the last command
     * 
     * Moves command from undo stack to redo stack
     */
    void Undo();

    /**
     * @brief Redo the last undone command
     * 
     * Moves command from redo stack to undo stack
     */
    void Redo();

    // ===== State Queries =====

    /**
     * @brief Check if undo is available
     */
    bool CanUndo() const { return !undoStack.empty(); }

    /**
     * @brief Check if redo is available
     */
    bool CanRedo() const { return !redoStack.empty(); }

    /**
     * @brief Get label for undo action (for menu/tooltip)
     * @return String like "Undo Add Dot"
     */
    std::string GetUndoLabel() const;

    /**
     * @brief Get label for redo action (for menu/tooltip)
     * @return String like "Redo Add Dot"
     */
    std::string GetRedoLabel() const;

    /**
     * @brief Get number of commands in undo stack
     */
    size_t GetUndoStackSize() const { return undoStack.size(); }

    /**
     * @brief Get number of commands in redo stack
     */
    size_t GetRedoStackSize() const { return redoStack.size(); }

private:
    /**
     * @brief Notify UI of state change (for updating undo/redo buttons)
     * 
     * Override this in derived class or use callback pattern
     */
    virtual void OnStateChanged() {
        // TODO: Notify view to update menu/toolbar state
    }
};

} // namespace DigitMode
