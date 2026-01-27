#include "stdafx.h"
#include "CommandDispatcher.h"
#include <cassert>

namespace DigitMode {

void CommandDispatcher::Execute(std::unique_ptr<Command> cmd) {
    assert(cmd != nullptr && "Cannot execute null command");

    // Execute command immediately
    cmd->Execute();

    // Add to undo stack
    undoStack.push_back(std::move(cmd));

    // Clear redo stack (new command breaks redo chain)
    redoStack.clear();

    // Notify UI
    OnStateChanged();
}

void CommandDispatcher::Undo() {
    if (undoStack.empty()) return;

    // Get command from top of undo stack
    Command* cmd = undoStack.back().get();
    cmd->Undo();

    // Move to redo stack
    redoStack.push_back(std::move(undoStack.back()));
    undoStack.pop_back();

    // Notify UI
    OnStateChanged();
}

void CommandDispatcher::Redo() {
    if (redoStack.empty()) return;

    // Get command from top of redo stack
    Command* cmd = redoStack.back().get();
    cmd->Redo();

    // Move to undo stack
    undoStack.push_back(std::move(redoStack.back()));
    redoStack.pop_back();

    // Notify UI
    OnStateChanged();
}

std::string CommandDispatcher::GetUndoLabel() const {
    if (undoStack.empty()) return "Undo";

    Command* cmd = undoStack.back().get();
    return "Undo " + cmd->GetName();
}

std::string CommandDispatcher::GetRedoLabel() const {
    if (redoStack.empty()) return "Redo";

    Command* cmd = redoStack.back().get();
    return "Redo " + cmd->GetName();
}

} // namespace DigitMode
