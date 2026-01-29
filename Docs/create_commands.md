
---

# 📌 COPILOT INSTRUCTION — Command-Derived Classes (Digit)

You are implementing **Command-derived classes** for the Digit editor.

This layer is responsible **only for undoable mutations of the document model**.

---

## 1. Fixed Base Class (DO NOT CHANGE)

All commands must derive from this exact base class:

```cpp
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
     * Override only if redo differs from execute
     */
    virtual void Redo() { Execute(); }

    /**
     * @brief Get human-readable command name for debugging/UI
     */
    virtual std::string GetName() const { return "Command"; }

    virtual ~Command() = default;
};
```

---

## 2. Document Model (AUTHORITATIVE)

* All geometry lives in the document class:

```cpp
class CDigitInfo {
public:
    std::vector<CFringeSegment> Fringes;
};
```

* `CFringeSegment` is the **primary geometric object**
* A **fringe** is a **logical grouping of segments by Number**
* Commands operate **only on `CDigitInfo::Fringes`**

---

## 3. Absolute Architectural Rules (MANDATORY)

Commands **MUST**:

* Operate only on `CDigitInfo`
* Represent **exactly one user intent**
* Be fully undoable
* Store **minimal state required for Undo**

Commands **MUST NOT**:

❌ Access UI, views, cursors, toolbars
❌ Read mouse or keyboard state
❌ Query editor mode
❌ Perform hit testing
❌ Modify selection
❌ Snapshot the entire document
❌ Use global state or singletons

> If a class needs mouse position or modifier keys, it is **not** a Command.

---

## 4. Identity & Indexing Rules

* Commands reference segments and dots by:

  * **segment index** in `CDigitInfo::Fringes`
  * **dot index** inside a segment
* Commands must assume indices are valid when executed
* Commands must restore indices exactly on Undo

---

## 5. Command Catalog (CANONICAL)

Copilot must implement **only these commands** unless explicitly instructed otherwise.

---

### 5.1 CreateSegmentCommand

**Intent:** Create a new segment.

```cpp
class CreateSegmentCommand : public Command {
public:
    CreateSegmentCommand(
        CDigitInfo& doc,
        const std::vector<CPoint2d>& points,
        double number
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Create Segment"; }

private:
    CDigitInfo& m_doc;
    std::vector<CPoint2d> m_points;
    double m_number;
    size_t m_createdIndex;
};
```

**Undo:** remove created segment
**Performance:** O(1) append / erase

---

### 5.2 ExtendSegmentCommand

**Intent:** Add one dot to a segment end.

```cpp
class ExtendSegmentCommand : public Command {
public:
    ExtendSegmentCommand(
        CDigitInfo& doc,
        size_t segmentIndex,
        bool atHead,
        const CPoint2d& point
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Extend Segment"; }

private:
    CDigitInfo& m_doc;
    size_t m_segmentIndex;
    bool m_atHead;
    CPoint2d m_point;
};
```

**Undo:** remove inserted dot
**Performance:** O(n) where n = dots in segment (acceptable)

---

### 5.3 MoveDotCommand

**Intent:** Move one dot.

```cpp
class MoveDotCommand : public Command {
public:
    MoveDotCommand(
        CDigitInfo& doc,
        size_t segmentIndex,
        size_t dotIndex,
        const CPoint2d& oldPos,
        const CPoint2d& newPos
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Move Dot"; }

private:
    CDigitInfo& m_doc;
    size_t m_segmentIndex;
    size_t m_dotIndex;
    CPoint2d m_oldPos;
    CPoint2d m_newPos;
};
```

**Performance:** O(1)

---

### 5.4 DeleteDotCommand

**Intent:** Remove one dot from a segment.

```cpp
class DeleteDotCommand : public Command {
public:
    DeleteDotCommand(
        CDigitInfo& doc,
        size_t segmentIndex,
        size_t dotIndex
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Delete Dot"; }

private:
    CDigitInfo& m_doc;
    size_t m_segmentIndex;
    size_t m_dotIndex;
    CPoint2d m_removedPoint;
};
```

**Undo:** reinsert dot at original index
**Performance:** O(n) insert/erase

---

### 5.5 SplitSegmentCommand

**Intent:** Split one segment into two.

```cpp
class SplitSegmentCommand : public Command {
public:
    SplitSegmentCommand(
        CDigitInfo& doc,
        size_t segmentIndex,
        size_t splitDotIndex
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Split Segment"; }

private:
    CDigitInfo& m_doc;
    size_t m_originalIndex;
    size_t m_newIndex;
    std::vector<CPoint2d> m_secondPart;
};
```

**Undo:** merge segments back
**Performance:** copies only tail of dot list

---

### 5.6 ConnectSegmentsCommand

**Intent:** Connect two segments into one.

```cpp
class ConnectSegmentsCommand : public Command {
public:
    ConnectSegmentsCommand(
        CDigitInfo& doc,
        size_t segA,
        bool endA,
        size_t segB,
        bool endB
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Connect Segments"; }

private:
    CDigitInfo& m_doc;
    size_t m_segA;
    size_t m_segB;
    bool m_endA;
    bool m_endB;

    std::vector<CPoint2d> m_segAPoints;
    std::vector<CPoint2d> m_segBPoints;
};
```

**Undo:** restore both original segments
**Performance:** O(n+m) for merge, acceptable

---

### 5.7 RenumberSegmentsCommand

**Intent:** Change Number on one or more segments.

```cpp
class RenumberSegmentsCommand : public Command {
public:
    RenumberSegmentsCommand(
        CDigitInfo& doc,
        const std::vector<size_t>& segmentIndices,
        double newNumber
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Renumber Segments"; }

private:
    CDigitInfo& m_doc;
    std::vector<size_t> m_indices;
    std::vector<double> m_oldNumbers;
    double m_newNumber;
};
```

**Undo:** restore previous Numbers
**Performance:** O(k), k = affected segments

---

### 5.8 DeleteSegmentsCommand

**Intent:** Delete one or more segments (incl. “Delete Fringe”).

```cpp
class DeleteSegmentsCommand : public Command {
public:
    DeleteSegmentsCommand(
        CDigitInfo& doc,
        const std::vector<size_t>& segmentIndices
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Delete Segments"; }

private:
    CDigitInfo& m_doc;

    struct RemovedSegment {
        size_t index;
        CFringeSegment segment;
    };

    std::vector<RemovedSegment> m_removed;
};
```

**Undo:** reinsert segments at original indices
**Performance:** O(k) inserts

---

## 6. Undo / Redo Guarantees

* `Undo()` must restore **bit-exact document state**
* `Redo()` must reapply the same mutation
* Commands must be deterministic
* No command may depend on external state

---

## 7. Final Mental Model (for Copilot)

> A Command is a **pure document mutation with memory**.
> It knows **what** to change — never **why** or **how it was triggered**.

---

### END OF INSTRUCTION

