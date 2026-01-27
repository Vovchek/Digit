#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/CommandDispatcher.h"
#include "DigitMode/Commands/Command.h"
#include <memory>

using namespace DigitMode;

/**
 * @brief Mock command for testing
 */
class MockCommand : public Command {
private:
    int& executeCount;
    int& undoCount;
    int& redoCount;
    std::string name;

public:
    MockCommand(int& execCount, int& undoCount, int& redoCount, const std::string& cmdName = "MockCommand")
        : executeCount(execCount), undoCount(undoCount), redoCount(redoCount), name(cmdName) {}

    void Execute() override {
        executeCount++;
    }

    void Undo() override {
        undoCount++;
    }

    void Redo() override {
        redoCount++;
    }

    std::string GetName() const override {
        return name;
    }
};

/**
 * @brief Mock command with state for testing undo/redo correctness
 */
class StateCommand : public Command {
private:
    int& targetValue;
    int newValue;
    int oldValue;
    std::string name;

public:
    StateCommand(int& target, int newVal, const std::string& cmdName = "StateCommand")
        : targetValue(target), newValue(newVal), oldValue(target), name(cmdName) {}

    void Execute() override {
        oldValue = targetValue;
        targetValue = newValue;
    }

    void Undo() override {
        targetValue = oldValue;
    }

    std::string GetName() const override {
        return name;
    }
};

/**
 * @brief Test fixture for CommandDispatcher
 */
class CommandDispatcherTest : public ::testing::Test {
protected:
    CommandDispatcher dispatcher;
    int executeCount = 0;
    int undoCount = 0;
    int redoCount = 0;

    void SetUp() override {
        executeCount = 0;
        undoCount = 0;
        redoCount = 0;
    }

    void TearDown() override {
        // unique_ptr handles cleanup automatically
    }
};

// ===== Basic Execution Tests =====

TEST_F(CommandDispatcherTest, InitialStateIsEmpty) {
    EXPECT_FALSE(dispatcher.CanUndo());
    EXPECT_FALSE(dispatcher.CanRedo());
    EXPECT_EQ(0, dispatcher.GetUndoStackSize());
    EXPECT_EQ(0, dispatcher.GetRedoStackSize());
}

TEST_F(CommandDispatcherTest, ExecuteCallsCommand) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    dispatcher.Execute(std::move(cmd));

    EXPECT_EQ(1, executeCount);
    EXPECT_EQ(0, undoCount);
    EXPECT_EQ(0, redoCount);
}

TEST_F(CommandDispatcherTest, ExecuteAddsToUndoStack) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    dispatcher.Execute(std::move(cmd));

    EXPECT_TRUE(dispatcher.CanUndo());
    EXPECT_FALSE(dispatcher.CanRedo());
    EXPECT_EQ(1, dispatcher.GetUndoStackSize());
    EXPECT_EQ(0, dispatcher.GetRedoStackSize());
}

TEST_F(CommandDispatcherTest, MultipleExecutesStackCommands) {
    for (int i = 0; i < 5; i++) {
        auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
        dispatcher.Execute(std::move(cmd));
    }

    EXPECT_EQ(5, executeCount);
    EXPECT_EQ(5, dispatcher.GetUndoStackSize());
    EXPECT_TRUE(dispatcher.CanUndo());
}

// ===== Undo Tests =====

TEST_F(CommandDispatcherTest, UndoCallsCommandUndo) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    dispatcher.Execute(std::move(cmd));

    dispatcher.Undo();

    EXPECT_EQ(1, undoCount);
}

TEST_F(CommandDispatcherTest, UndoMovesToRedoStack) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    dispatcher.Execute(std::move(cmd));

    dispatcher.Undo();

    EXPECT_FALSE(dispatcher.CanUndo());
    EXPECT_TRUE(dispatcher.CanRedo());
    EXPECT_EQ(0, dispatcher.GetUndoStackSize());
    EXPECT_EQ(1, dispatcher.GetRedoStackSize());
}

TEST_F(CommandDispatcherTest, UndoOnEmptyStackDoesNothing) {
    dispatcher.Undo();  // Should not crash

    EXPECT_FALSE(dispatcher.CanUndo());
    EXPECT_EQ(0, undoCount);
}

TEST_F(CommandDispatcherTest, MultipleUndosReversesOrder) {
    int value = 0;

    auto cmd1 = std::make_unique<StateCommand>(value, 1, "Cmd1");
    auto cmd2 = std::make_unique<StateCommand>(value, 2, "Cmd2");
    auto cmd3 = std::make_unique<StateCommand>(value, 3, "Cmd3");

    dispatcher.Execute(std::move(cmd1));
    dispatcher.Execute(std::move(cmd2));
    dispatcher.Execute(std::move(cmd3));

    EXPECT_EQ(3, value);

    dispatcher.Undo();
    EXPECT_EQ(2, value);

    dispatcher.Undo();
    EXPECT_EQ(1, value);

    dispatcher.Undo();
    EXPECT_EQ(0, value);
}

// ===== Redo Tests =====

TEST_F(CommandDispatcherTest, RedoCallsCommandRedo) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    dispatcher.Execute(std::move(cmd));
    dispatcher.Undo();

    dispatcher.Redo();

    EXPECT_EQ(1, redoCount);
}

TEST_F(CommandDispatcherTest, RedoMovesBackToUndoStack) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    dispatcher.Execute(std::move(cmd));
    dispatcher.Undo();

    dispatcher.Redo();

    EXPECT_TRUE(dispatcher.CanUndo());
    EXPECT_FALSE(dispatcher.CanRedo());
    EXPECT_EQ(1, dispatcher.GetUndoStackSize());
    EXPECT_EQ(0, dispatcher.GetRedoStackSize());
}

TEST_F(CommandDispatcherTest, RedoOnEmptyStackDoesNothing) {
    dispatcher.Redo();  // Should not crash

    EXPECT_FALSE(dispatcher.CanRedo());
    EXPECT_EQ(0, redoCount);
}

TEST_F(CommandDispatcherTest, MultipleRedosRestoresOrder) {
    int value = 0;

    auto cmd1 = std::make_unique<StateCommand>(value, 1);
    auto cmd2 = std::make_unique<StateCommand>(value, 2);

    dispatcher.Execute(std::move(cmd1));
    dispatcher.Execute(std::move(cmd2));
    EXPECT_EQ(2, value);

    dispatcher.Undo();
    dispatcher.Undo();
    EXPECT_EQ(0, value);

    dispatcher.Redo();
    EXPECT_EQ(1, value);

    dispatcher.Redo();
    EXPECT_EQ(2, value);
}

// ===== Redo Stack Clearing =====

TEST_F(CommandDispatcherTest, NewCommandClearsRedoStack) {
    auto cmd1 = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    auto cmd2 = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);

    dispatcher.Execute(std::move(cmd1));
    dispatcher.Undo();

    EXPECT_TRUE(dispatcher.CanRedo());
    EXPECT_EQ(1, dispatcher.GetRedoStackSize());

    // Execute new command
    dispatcher.Execute(std::move(cmd2));

    EXPECT_FALSE(dispatcher.CanRedo());  // Redo stack should be cleared
    EXPECT_EQ(0, dispatcher.GetRedoStackSize());
}

// ===== Label Tests =====

TEST_F(CommandDispatcherTest, GetUndoLabelReturnsCommandName) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount, "Test Command");
    dispatcher.Execute(std::move(cmd));

    std::string label = dispatcher.GetUndoLabel();
    EXPECT_EQ("Undo Test Command", label);
}

TEST_F(CommandDispatcherTest, GetRedoLabelReturnsCommandName) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount, "Test Command");
    dispatcher.Execute(std::move(cmd));
    dispatcher.Undo();

    std::string label = dispatcher.GetRedoLabel();
    EXPECT_EQ("Redo Test Command", label);
}

TEST_F(CommandDispatcherTest, GetUndoLabelWhenEmptyReturnsDefault) {
    std::string label = dispatcher.GetUndoLabel();
    EXPECT_EQ("Undo", label);
}

TEST_F(CommandDispatcherTest, GetRedoLabelWhenEmptyReturnsDefault) {
    std::string label = dispatcher.GetRedoLabel();
    EXPECT_EQ("Redo", label);
}

// ===== Complex Workflow Tests =====

TEST_F(CommandDispatcherTest, ComplexUndoRedoWorkflow) {
    int value = 0;

    // Execute 3 commands
    dispatcher.Execute(std::make_unique<StateCommand>(value, 1, "Cmd1"));
    dispatcher.Execute(std::make_unique<StateCommand>(value, 2, "Cmd2"));
    dispatcher.Execute(std::make_unique<StateCommand>(value, 3, "Cmd3"));
    EXPECT_EQ(3, value);
    EXPECT_EQ(3, dispatcher.GetUndoStackSize());

    // Undo 2 commands
    dispatcher.Undo();
    dispatcher.Undo();
    EXPECT_EQ(1, value);
    EXPECT_EQ(1, dispatcher.GetUndoStackSize());
    EXPECT_EQ(2, dispatcher.GetRedoStackSize());

    // Redo 1 command
    dispatcher.Redo();
    EXPECT_EQ(2, value);
    EXPECT_EQ(2, dispatcher.GetUndoStackSize());
    EXPECT_EQ(1, dispatcher.GetRedoStackSize());

    // Execute new command (clears redo stack)
    dispatcher.Execute(std::make_unique<StateCommand>(value, 4, "Cmd4"));
    EXPECT_EQ(4, value);
    EXPECT_EQ(3, dispatcher.GetUndoStackSize());
    EXPECT_EQ(0, dispatcher.GetRedoStackSize());
}

// ===== State Correctness Tests =====

TEST_F(CommandDispatcherTest, UndoRedoPreservesState) {
    int value = 0;

    auto cmd = std::make_unique<StateCommand>(value, 42);
    dispatcher.Execute(std::move(cmd));
    EXPECT_EQ(42, value);

    dispatcher.Undo();
    EXPECT_EQ(0, value);

    dispatcher.Redo();
    EXPECT_EQ(42, value);
}

TEST_F(CommandDispatcherTest, MultipleCommandsCorrectState) {
    int value = 0;

    dispatcher.Execute(std::make_unique<StateCommand>(value, 10));
    dispatcher.Execute(std::make_unique<StateCommand>(value, 20));
    dispatcher.Execute(std::make_unique<StateCommand>(value, 30));

    EXPECT_EQ(30, value);

    dispatcher.Undo();
    dispatcher.Undo();
    dispatcher.Undo();

    EXPECT_EQ(0, value);  // Back to original

    dispatcher.Redo();
    dispatcher.Redo();

    EXPECT_EQ(20, value);  // Partially restored
}

// ===== Performance Tests =====

TEST_F(CommandDispatcherTest, LargeCommandStackPerformance) {
    int dummyValue = 0;

    // Execute 1000 commands
    for (int i = 0; i < 1000; i++) {
        auto cmd = std::make_unique<StateCommand>(dummyValue, i);
        dispatcher.Execute(std::move(cmd));
    }

    EXPECT_EQ(1000, dispatcher.GetUndoStackSize());
    EXPECT_EQ(999, dummyValue);

    // Undo all
    for (int i = 0; i < 1000; i++) {
        dispatcher.Undo();
    }

    EXPECT_EQ(0, dispatcher.GetUndoStackSize());
    EXPECT_EQ(1000, dispatcher.GetRedoStackSize());
    EXPECT_EQ(0, dummyValue);
}

// ===== Memory Management Tests =====

TEST_F(CommandDispatcherTest, CommandsAreProperlyDestroyed) {
    // unique_ptr should automatically clean up
    // This test verifies no crashes occur

    for (int i = 0; i < 100; i++) {
        auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
        dispatcher.Execute(std::move(cmd));
    }

    // Destructor should clean up all commands without crash
}

TEST_F(CommandDispatcherTest, ClearRedoStackFreesMemory) {
    for (int i = 0; i < 10; i++) {
        auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
        dispatcher.Execute(std::move(cmd));
    }

    for (int i = 0; i < 10; i++) {
        dispatcher.Undo();
    }

    EXPECT_EQ(10, dispatcher.GetRedoStackSize());

    // Execute new command (should clear redo stack)
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    dispatcher.Execute(std::move(cmd));

    EXPECT_EQ(0, dispatcher.GetRedoStackSize());
    // Memory should be freed (no leak)
}

// ===== Edge Cases =====

TEST_F(CommandDispatcherTest, UndoRedoUndoRedoWorks) {
    int value = 0;
    auto cmd = std::make_unique<StateCommand>(value, 5);
    dispatcher.Execute(std::move(cmd));

    dispatcher.Undo();
    EXPECT_EQ(0, value);

    dispatcher.Redo();
    EXPECT_EQ(5, value);

    dispatcher.Undo();
    EXPECT_EQ(0, value);

    dispatcher.Redo();
    EXPECT_EQ(5, value);
}

TEST_F(CommandDispatcherTest, CanUndoCanRedoConsistency) {
    auto cmd = std::make_unique<MockCommand>(executeCount, undoCount, redoCount);
    dispatcher.Execute(std::move(cmd));

    EXPECT_TRUE(dispatcher.CanUndo());
    EXPECT_FALSE(dispatcher.CanRedo());

    dispatcher.Undo();

    EXPECT_FALSE(dispatcher.CanUndo());
    EXPECT_TRUE(dispatcher.CanRedo());

    dispatcher.Redo();

    EXPECT_TRUE(dispatcher.CanUndo());
    EXPECT_FALSE(dispatcher.CanRedo());
}
