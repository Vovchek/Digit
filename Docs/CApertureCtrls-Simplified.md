# CApertureCtrls Simplified Architecture

## ✅ Simplifications Made

### 1. **ShapeHandle Simplified**
**Before** (overcomplicated):
```cpp
struct ShapeHandle {
    size_t index = 0;
    aperture::TypeLimits type;
    uint64_t version = 0;  // ❌ Redundant! Version belongs to collection
    
    bool isValid() const { return version > 0; }
    void invalidate() { version = 0; }
};
```

**After** (simple):
```cpp
struct ShapeHandle {
    size_t index = 0;
    aperture::TypeLimits type = aperture::TypeLimits::EXTERNAL;
    
    bool operator==(const ShapeHandle& other) const {
        return index == other.index && type == other.type;
    }
};
```

**Why**: 
- Handle is just an index + type, nothing more
- Validity is checked against **collection version**, not handle version
- No need for `isValid()` / `invalidate()` - just check bounds

---

### 2. **"Handle" Terminology Clarified**
**Confusion**: "Handle" has two meanings in code
1. **Programming handle**: Resource identifier (like HANDLE in Win32)
2. **UI handle**: Small square you drag with mouse (corner, vertex)

**Solution**: Renamed UI "handles" to **"control points"**
```cpp
// Old (confusing):
int HitTestShapeHandles(...);  // What kind of handle?
int handleIndex = -1;          // Programming or UI handle?

// New (clear):
int HitTestControlPoints(...); // UI drag points (corners, vertices)
int controlPointIndex = -1;    // Corner/vertex index
```

**Control Point** = Corner square (rectangle) or vertex point (polygon) you drag to resize/reshape.

---

### 3. **Version Belongs to Collection**
**Before** (confused ownership):
```cpp
// Each handle tracked "when it was created"
ShapeHandle handle;
handle.version = m_shapes.getVersion();

// Stale check:
if (handle.version != m_shapes.getVersion()) { /* stale */ }
```

**After** (clear ownership):
```cpp
// Collection has version, handles are just indices
uint64_t version = apertureCtrls.GetVersion();

// Validity check:
if (handle.index >= container->size()) { /* invalid */ }
```

**Why**:
- Version tracks **collection changes** (add/remove/modify)
- Handles don't need their own version
- Simpler: just check if index is in range

---

### 4. **Editing State Simplified**
**Before**:
```cpp
bool IsEditing() const { return m_editHandle.isValid(); }
```

**After**:
```cpp
bool m_isEditing = false;  // Explicit flag
bool IsEditing() const { return m_isEditing; }
```

**Why**: More explicit, no magic "isValid()" check

---

## 📊 Simplified API

### Shape Management
```cpp
// Add shapes (returns simple handle: {index, type})
ShapeHandle h = apertureCtrls.AddExternalShape(shape);

// Access shapes
const Shape* s = apertureCtrls.GetShape(h);
Shape* s = apertureCtrls.GetShapeForEdit(h);

// Collection version
uint64_t v = apertureCtrls.GetVersion();  // Increments on add/remove/modify
```

### Hit-Testing (Control Points = Drag Squares)
```cpp
// Test if point hits any shape or control point
HitTestResult r = apertureCtrls.HitTest(worldPt, tolerance);

if (r.hitControlPoint()) {
    // User clicked corner/vertex drag square
    int corner = r.controlPointIndex;  // 0-3 for rect, 0-N for polygon
}

if (r.hitBody()) {
    // User clicked inside shape (not on control point)
}

// Test specific shape's control points
int corner = apertureCtrls.HitTestControlPoints(handle, worldPt, tolerance);
// Returns: 0-3 for rectangle corners, 0-N for polygon vertices
```

### Editing
```cpp
// Begin drag (controlPointIndex = which corner/vertex)
apertureCtrls.BeginEdit(handle, controlPointIndex);

// Update during drag
apertureCtrls.UpdateEdit(worldDelta);

// Finish
apertureCtrls.CommitEdit();  // or CancelEdit()
```

---

## 🎯 Key Design Principles

### ✅ Keep It Simple
- **Handle** = just index + type
- **Version** = collection property, not handle property
- **Control Point** = UI drag square (corner, vertex)

### ✅ Clear Ownership
- Collection owns version
- Collection owns shapes
- Handles are lightweight references

### ✅ No Over-Engineering
- No complex validation logic in handles
- No version tracking per handle
- Simple bounds checking

---

## 🔄 Version Semantics

### What Increments Version?
```cpp
AddExternalShape()      // ✅ Version++
AddInternalShape()      // ✅ Version++
AddApertureShape()      // ✅ Version++
RemoveShape()           // ✅ Version++
NotifyShapeModified()   // ✅ Version++ (geometry changed)
```

### How to Use Version
```cpp
// Cache version to detect changes
uint64_t cachedVersion = apertureCtrls.GetVersion();

// ... do work ...

if (apertureCtrls.GetVersion() != cachedVersion) {
    // Collection changed - invalidate caches, redraw, etc.
}
```

### What Doesn't Change Version?
```cpp
GetShape()              // ❌ Read-only
GetShapeForEdit()       // ❌ Just returns pointer
BeginEdit()             // ❌ Editing in progress
UpdateEdit()            // ❌ Preview only
CommitEdit()            // ✅ Calls NotifyShapeModified()
```

---

## 📖 Control Point Terminology

### Rectangle Control Points
```
0 (TL) ─────────── 1 (TR)
  │                 │
  │                 │
  │                 │
3 (BL) ─────────── 2 (BR)
```

Index: 0=TopLeft, 1=TopRight, 2=BottomRight, 3=BottomLeft

### Polygon Control Points
```
     2
    ╱ ╲
   ╱   ╲
  1     3
 ╱       ╲
0────────4
```

Index: 0 to N-1 (vertices in order)

### Ellipse Control Points
```
        1 (Top)
        │
3 (L) ─ C ─ 2 (R)
        │
        0 (Bottom)
```

Index: 0=Bottom, 1=Top, 2=Right, 3=Left (bounding box corners)

---

## 🧪 Test Updates

### Old Test (Complex)
```cpp
TEST_F(..., GetShape_StaleHandle_ReturnsNull) {
    ShapeHandle h = apertureCtrls.AddExternalShape(shape1);
    apertureCtrls.AddExternalShape(shape2);  // Version changes!
    
    // Handle becomes stale (version mismatch)
    EXPECT_EQ(nullptr, apertureCtrls.GetShape(h));  // ❌ Wrong!
}
```

### New Test (Simple)
```cpp
TEST_F(..., GetShape_AfterAddingMore_StillValid) {
    ShapeHandle h = apertureCtrls.AddExternalShape(shape1);
    apertureCtrls.AddExternalShape(shape2);  // Version changes, but...
    
    // Handle still valid (just index 0)
    EXPECT_NE(nullptr, apertureCtrls.GetShape(h));  // ✅ Correct!
}
```

**Why**: Handles don't become "stale" just because collection version changed. They become invalid only if removed or index out of bounds.

---

## 🎉 Summary

| Aspect | Before | After |
|--------|--------|-------|
| **ShapeHandle** | 3 fields + version tracking | 2 fields (index + type) |
| **Validation** | Version mismatch check | Bounds check |
| **"Handle" meaning** | Ambiguous (programming vs UI) | Clear: control point = drag square |
| **Version ownership** | Confused (handle vs collection) | Clear: collection owns version |
| **IsEditing()** | Magic `m_editHandle.isValid()` | Explicit `m_isEditing` flag |

**Result**: Simpler, clearer, easier to use and test.
