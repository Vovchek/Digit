
---

# Copilot Instruction

## Extend Aperture Core with Visibility Mask Builder & Provider

You are extending the **aperture / visibility core module**.
Do **not** touch editor tools, views, or legacy controllers yet.
Focus only on **core data + derived visibility artifacts**.

---

## 1. Existing Core (assumed, do not redesign)

The following already exist and must remain conceptually unchanged:

### Geometry

* `Point`
* `Bounds`
* `CoordinateSystem`
* `NormalizationState`

### Shapes

* `Shape` (abstract base)
* `Ellipse`
* `Rectangle`
* `Polygon`

### Visibility primitives

* `TypeLimits`
  (`EXTERNAL`, `INTERNAL`, `APERTURE`)

### Collections

* `ShapeCollection`

  * stores shapes grouped by `TypeLimits`
  * supports iteration and editing

### VisibilityChecker

* answers: *“Is this point visible given current shapes?”*
* must remain **stateless**
* must NOT cache
* must NOT loop pixels

---

## 2. New Concepts to Introduce (Core Extension)

You must introduce **three new core-level entities**:

1. `VisibilityMask`
2. `VisibilityMaskBuilder`
3. `VisibilityMaskProvider`

These must live **next to** the visibility module (same layer), not in UI or editor code.

---

## 3. VisibilityMask (Data Object)

### Purpose

A **pure data container** representing pixel visibility for a bitmap.

### Responsibilities

* store per-pixel visibility
* know its own dimensions
* store version metadata (for cache validation)

### Requirements

* Use **flat storage**, not `vector<vector<bool>>`
* Pixel space only (no transforms, no zoom)

### Conceptual interface

```cpp
class VisibilityMask {
public:
    int width;
    int height;

    // 0 = invisible, 1 = visible
    std::vector<uint8_t> data;

    // Version stamps used for cache validation
    uint64_t shapeVersion;
    uint64_t imageVersion;

    bool IsVisible(int x, int y) const;
};
```

Rules:

* No geometry logic
* No rebuild logic
* No threading
* No dependency on UI or documents

---

## 4. VisibilityMaskBuilder (Heavy CPU, Stateless)

### Purpose

Build a `VisibilityMask` from shapes and bitmap dimensions.

### Responsibilities

* iterate pixels
* query `VisibilityChecker`
* fill mask buffer
* produce a *new* `VisibilityMask`

### Constraints

* **Stateless**
* No caching
* No ownership of shapes
* No global state

### Conceptual interface

```cpp
class VisibilityMaskBuilder {
public:
    static VisibilityMask Build(
        const ShapeCollection& shapes,
        int width,
        int height);
};
```

Implementation rules:

* For each pixel `(x,y)`:

  * call `VisibilityChecker::IsVisible(x,y)`
* No shortcuts, no caching
* This is the *only* place with per-pixel loops

---

## 5. VisibilityMaskProvider (Cache + Validity Gate)

### Purpose

Provide **reliable access** to a valid visibility mask at any time.

This is the **single access point** for mask consumers.

---

### Responsibilities

* own the cached `VisibilityMask`
* track whether it is stale
* rebuild lazily on demand
* hide all complexity from callers

### Must NOT

* expose dirty flags
* require callers to invalidate
* depend on UI or editor tools

---

### Dependencies

The provider depends on:

* `ShapeCollection` (for geometry + version)
* bitmap size + image version (passed or queried)

These are **dependencies, not ownership**.

---

### Conceptual interface

```cpp
class VisibilityMaskProvider {
public:
    VisibilityMaskProvider(
        const ShapeCollection& shapes,
        int imageWidth,
        int imageHeight);

    const VisibilityMask& GetMask();

    void UpdateImageSize(int width, int height);
};
```

---

### Internal logic (mandatory behavior)

Inside `GetMask()`:

1. Query:

   * `ShapeCollection::GetVersion()`
   * current image version (or size change)
2. Compare with stored version stamps in cached mask
3. If mismatch OR no mask exists:

   * rebuild using `VisibilityMaskBuilder`
4. Return cached mask

This guarantees:

> **Every call returns a fresh, valid mask**

---

## 6. Versioning Contract (Critical)

### ShapeCollection

Must expose:

```cpp
uint64_t GetVersion() const;
```

Version increments on:

* add shape
* remove shape
* modify shape geometry

---

### Image

Image version increments on:

* image load
* resize
* reallocation

The provider **does not care why**, only that version changed.

---

## 7. Threading (Explicitly Deferred)

For now:

* `GetMask()` may rebuild synchronously
* No background threads
* No async logic

Design must **allow future async**, but not implement it yet.

---

## 8. What NOT to do (strict)

* Do NOT add visibility logic to `Shape`
* Do NOT add caching to `VisibilityChecker`
* Do NOT let editor tools touch the mask
* Do NOT store mask in the View
* Do NOT use `vector<vector<bool>>`
* Do NOT introduce coordinate transforms here

---

## 9. Expected Usage Pattern (Target)

Later, consumers will do:

```cpp
const VisibilityMask& mask =
    visibilityMaskProvider.GetMask();

if (mask.IsVisible(x,y)) {
    // process pixel
}
```

They never:

* ask “is it valid?”
* trigger rebuilds
* know about shapes or apertures

---

## 10. Design Invariant (Important)

> **VisibilityMaskProvider is the only authority on mask validity.**

If another part of code tries to manage mask state, that is a bug.

---

## 11. Acceptance Criteria

The implementation is correct if:

* Mask rebuilds automatically after shape edit
* Mask rebuilds automatically after image resize
* Multiple calls return the same cached instance
* No visibility logic exists in shapes
* No editor code references mask internals

---

