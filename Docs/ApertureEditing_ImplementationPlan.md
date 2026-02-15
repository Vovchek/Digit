# Aperture Editing - Detailed Implementation Plan

**Version:** 1.0  
**Date:** 2024  
**Status:** For Review

---

## Executive Summary

This document provides a detailed, phase-by-phase implementation plan for completing the aperture (bounds) editing system according to the UX/UI specification in `copilot_xxx.md`.

**Current State:**
- ✅ Command pattern foundation (ReplaceShapeCommand, AddShapeCommand, RemoveShapeCommand)
- ✅ CApertureCtrls as Aperture Subsystem Coordinator
- ✅ VisibilityMaskProvider integration
- ✅ BoundsHandler with preview-only editing
- ✅ Basic handle system (EnumerateHandles, ApplyHandleDrag)

**Missing Components:**
- ❌ Draft shape creation pipeline
- ❌ Modal editing system (Select/Add/Delete modes)
- ❌ Shape rendering layer (IShapeRenderer, concrete renderers)
- ❌ Rectangle 3-point constructor
- ❌ Circle vs Ellipse distinction in creation
- ❌ Selection state management for shapes
- ❌ UI integration (deferred to final phase)

---

## Guiding Principles

### 1. **Separation of Concerns (STRICT)**
- **ApertureCore**: Pure geometry, no UI, no MFC
- **DigitMode**: Interaction logic, Commands, state machines
- **UI Layer (ImageView)**: Rendering, event routing only

### 2. **Test-Driven Development**
- Each phase includes unit tests
- Integration tests before UI wiring
- Mock objects for UI dependencies

### 3. **Incremental Integration**
- Build and test each component independently
- Integrate in layers (ApertureCore → DigitMode → UI)
- Delay CImageView changes until Phase 5

### 4. **No Architecture Violations**
- Shapes never know about rendering
- Commands never bypass ShapeCollection
- BoundsHandler never mutates shapes directly

---

## Phase 1: ApertureCore Geometry Extensions

**Goal:** Add missing geometric primitives and constructors.

**Duration:** 1-2 days  
**Dependencies:** None  
**Risk:** Low

### 1.1 Rectangle 3-Point Constructor

**Location:** `ApertureCore/include/aperturecore/geometry/Rectangle.h`

**Implementation:**
```cpp
namespace aperture {
    class Rectangle : public Shape {
    public:
        /**
         * @brief Construct rectangle from 3 points
         * @param p0 First corner
         * @param p1 Second corner (defines width axis)
         * @param p2 Third point (defines height and orientation)
         * 
         * Geometry:
         * - Vector p0→p1 defines width direction
         * - Projection of p2 onto perpendicular defines height
         * - Center is geometric center of resulting rectangle
         * - Rotation computed from width vector angle
         */
        explicit Rectangle(
            const Point& p0,
            const Point& p1,
            const Point& p2,
            TypeLimits typeLimits = TypeLimits::EXTERNAL,
            CoordinateSystem spatialSystem = CoordinateSystem::screen(),
            NormalizationState normState = NormalizationState::MEASURING
        );
        
        // ...existing constructors...
    };
}
```

**Algorithm:**
1. Compute width vector: `v_width = p1 - p0`
2. Compute perpendicular: `v_perp = rotate_90(v_width)`
3. Project p2 onto perpendicular: `height = dot(p2 - p0, normalize(v_perp))`
4. Compute center: `center = p0 + 0.5 * v_width + 0.5 * height * normalize(v_perp)`
5. Rotation: `atan2(v_width.y, v_width.x)`

**Tests Required:**
- `RectangleTest::Constructor_ThreePoints_AxisAligned`
- `RectangleTest::Constructor_ThreePoints_Rotated45`
- `RectangleTest::Constructor_ThreePoints_ArbitraryAngle`
- `RectangleTest::ThreePointRect_CornerExtraction_MatchesInput`

**Files to Create/Modify:**
- `ApertureCore/include/aperturecore/geometry/Rectangle.h` (add declaration)
- `ApertureCore/src/geometry/Rectangle.cpp` (implement constructor)
- `Tests/ApertureCoreTests/RectangleTest.cpp` (add tests)

---

### 1.2 Circle Fitting (LSM)

**Location:** `ApertureCore/include/aperturecore/geometry/Ellipse.h`

**Note:** Circle is a special case of Ellipse with equal radii.

**Implementation:**
```cpp
namespace aperture {
    class Ellipse : public Shape {
    public:
        /**
         * @brief Fit circle to perimeter points (LSM)
         * @param points Perimeter sample points (N ≥ 3)
         * @param typeLimits Visibility type
         * @return Ellipse with radiusX == radiusY (perfect circle)
         * 
         * Uses least-squares circle fitting:
         * - Minimizes sum of squared radial distances
         * - Constraint: radiusX = radiusY enforced
         * - Returns circle (not general ellipse)
         */
        static std::unique_ptr<Ellipse> FitCircle(
            const std::vector<Point>& points,
            TypeLimits typeLimits = TypeLimits::EXTERNAL,
            CoordinateSystem spatialSystem = CoordinateSystem::screen(),
            NormalizationState normState = NormalizationState::MEASURING
        );
        
        /**
         * @brief Fit general ellipse to perimeter points (LSM)
         * @param points Perimeter sample points (N ≥ 5)
         * @return Ellipse with optimized radii and rotation
         * 
         * Note: Existing constructor already does this via implicit conversion.
         *       This is an explicit static factory for clarity.
         */
        static std::unique_ptr<Ellipse> FitEllipse(
            const std::vector<Point>& points,
            TypeLimits typeLimits = TypeLimits::EXTERNAL,
            CoordinateSystem spatialSystem = CoordinateSystem::screen(),
            NormalizationState normState = NormalizationState::MEASURING
        );
        
        // ...existing constructors...
    };
}
```

**Algorithm (Circle LSM):**
1. Compute centroid: `c = mean(points)`
2. For each point: compute distance `r_i = ||p_i - c||`
3. Optimize center to minimize `Σ(r_i - r_mean)²`
4. Return Ellipse with `radiusX = radiusY = r_mean`

**Tests Required:**
- `EllipseTest::FitCircle_ThreePoints_ExactFit`
- `EllipseTest::FitCircle_FourPoints_LeastSquares`
- `EllipseTest::FitCircle_NoisyData_RobustFit`
- `EllipseTest::FitEllipse_FivePoints_Elliptical`

**Files to Create/Modify:**
- `ApertureCore/include/aperturecore/geometry/Ellipse.h` (add static methods)
- `ApertureCore/src/geometry/Ellipse.cpp` (implement fitting)
- `Tests/ApertureCoreTests/EllipseTest.cpp` (add tests)

---

### 1.3 Handle Refinements (Optional for Phase 1)

**Current State:** Ellipse and Polygon already have `EnumerateHandles` and `ApplyHandleDrag`.

**Rectangle Handles** (if not yet implemented):
- Center handle (move)
- 4 corner handles (resize)
- 4 edge midpoint handles (resize single dimension)
- 1 rotation handle (above top edge)

**Verification:**
- Test that `Rectangle::EnumerateHandles()` returns 10 handles
- Test that drag operations preserve constraints (Shift = aspect ratio, Alt = from center)

---

## Phase 2: Draft Shape Creation System

**Goal:** Add draft shape pipeline to BoundsHandler for creating new shapes via point sequences.

**Duration:** 2-3 days  
**Dependencies:** Phase 1  
**Risk:** Medium

### 2.1 DraftShape Data Structure

**Location:** `DigitMode/DraftShape.h` (NEW)

```cpp
#pragma once
#include "ApertureCore/include/aperturecore/geometry/Point.h"
#include "ApertureCore/include/aperturecore/visibility/TypeLimits.h"
#include <vector>
#include <memory>

namespace aperture { class Shape; }

namespace DigitMode {

/**
 * @brief Shape being created, not yet committed
 * 
 * Accumulates perimeter points during creation workflow.
 * Converts to committed Shape on finalization.
 */
struct DraftShape {
    enum class Kind {
        Rectangle,  ///< 3 points → Rectangle via 3-point constructor
        Ellipse,    ///< N points → Ellipse via LSM fit
        Circle,     ///< N points → Circle via constrained LSM fit
        Polygon     ///< N points → Polygon vertices
    };
    
    Kind kind = Kind::Rectangle;
    aperture::TypeLimits type = aperture::TypeLimits::EXTERNAL;
    std::vector<aperture::Point> perimeterPoints;  ///< Ordered clicks
    
    /**
     * @brief Check if draft has minimum points for commit
     */
    bool CanCommit() const;
    
    /**
     * @brief Convert draft to committed shape
     * @return Unique pointer to created shape, or nullptr if invalid
     */
    std::unique_ptr<aperture::Shape> ToShape() const;
    
    /**
     * @brief Get preview shape for rendering (updates on each point)
     */
    std::unique_ptr<aperture::Shape> GetPreview() const;
    
    /**
     * @brief Add a point to the draft
     */
    void AddPoint(const aperture::Point& pt);
    
    /**
     * @brief Clear all points
     */
    void Clear();
};

} // namespace DigitMode
```

**Implementation Notes:**
- `CanCommit()`: Rectangle requires 3 points, Ellipse/Circle require ≥3, Polygon requires ≥3
- `ToShape()`: Dispatches to appropriate constructor/factory based on `kind`
- `GetPreview()`: Returns intermediate shape for live feedback during creation

**Tests Required:**
- `DraftShapeTest::Rectangle_ThreePoints_CanCommit`
- `DraftShapeTest::Ellipse_TwoPoints_CannotCommit`
- `DraftShapeTest::ToShape_Rectangle_CreatesValidShape`
- `DraftShapeTest::ToShape_Circle_RadiiEqual`
- `DraftShapeTest::Polygon_SelfIntersection_ReturnsNull`

**Files to Create:**
- `DigitMode/DraftShape.h`
- `DigitMode/DraftShape.cpp`
- `Tests/DigitModeTests/DraftShapeTest.cpp`

---

### 2.2 Edit Mode System

**Location:** `DigitMode/EditMode.h` (NEW)

```cpp
#pragma once

namespace DigitMode {

/**
 * @brief Bounds editing modes (mutually exclusive)
 */
enum class EditMode {
    Select,       ///< Select/move/resize/rotate existing shapes
    AddRectangle, ///< Create rectangle via 3-point sequence
    AddEllipse,   ///< Create ellipse via perimeter fitting
    AddCircle,    ///< Create circle (constrained ellipse)
    AddPolygon,   ///< Create polygon via vertex clicks
    Delete        ///< Remove shape on click
};

/**
 * @brief Get display name for mode (for status bar)
 */
const char* GetEditModeName(EditMode mode);

/**
 * @brief Get cursor ID for mode (for SetCursor)
 */
int GetEditModeCursor(EditMode mode);

} // namespace DigitMode
```

**Implementation:**
- Simple enum + utility functions
- No state machine logic here (that's in BoundsHandler)

**Files to Create:**
- `DigitMode/EditMode.h`
- `DigitMode/EditMode.cpp`

---

### 2.3 BoundsHandler Extensions

**Location:** `DigitMode/BoundsHandler.h`

**Add Members:**
```cpp
class BoundsHandler {
public:
    // ...existing interface...
    
    // ========================================================================
    // Edit Mode Management
    // ========================================================================
    
    /**
     * @brief Set current edit mode
     */
    void SetEditMode(EditMode mode);
    
    /**
     * @brief Get current edit mode
     */
    EditMode GetEditMode() const { return m_editMode; }
    
    // ========================================================================
    // Draft Shape Management (Creation Modes)
    // ========================================================================
    
    /**
     * @brief Add point to draft shape (Add modes only)
     * @param worldPt Point in world coordinates
     * @return true if point accepted
     */
    bool AddDraftPoint(const aperture::Point& worldPt);
    
    /**
     * @brief Commit draft shape (create AddShapeCommand)
     * @return true if committed successfully
     */
    bool CommitDraft();
    
    /**
     * @brief Cancel draft shape
     */
    void CancelDraft();
    
    /**
     * @brief Get draft preview for rendering
     * @return Preview shape, or nullptr if no draft
     */
    const aperture::Shape* GetDraftPreview() const;
    
    /**
     * @brief Check if currently creating a draft
     */
    bool IsDrafting() const { return m_draft.has_value(); }
    
private:
    EditMode m_editMode = EditMode::Select;
    std::optional<DraftShape> m_draft;
    std::unique_ptr<aperture::Shape> m_draftPreview;  ///< Cached preview
    
    // ...existing members...
};
```

**Workflow Logic:**
- `SetEditMode()`: Clear draft if switching modes
- `AddDraftPoint()`: Append to `m_draft.perimeterPoints`, update preview
- `CommitDraft()`: Call `m_draft->ToShape()`, create `AddShapeCommand`, dispatch, clear draft
- `CancelDraft()`: Clear `m_draft`

**Tests Required:**
- `BoundsHandlerTest::SetEditMode_ClearsDraftOnSwitch`
- `BoundsHandlerTest::AddRectangle_ThreePoints_Commits`
- `BoundsHandlerTest::AddEllipse_FourPoints_UpdatesPreview`
- `BoundsHandlerTest::CancelDraft_ClearsDraft`
- `BoundsHandlerTest::CommitDraft_DispatchesAddCommand`

**Files to Modify:**
- `DigitMode/BoundsHandler.h` (add declarations)
- `DigitMode/BoundsHandler.cpp` (implement methods)
- `Tests/DigitModeTests/BoundsHandlerTest.cpp` (add tests)

---

### 2.4 Keyboard Input Handling

**Location:** `DigitMode/BoundsHandler.h`

**Add Method:**
```cpp
class BoundsHandler {
public:
    /**
     * @brief Handle keyboard input
     * @param nChar Virtual key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     * @return true if key was handled
     */
    bool OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};
```

**Implementation:**
- `VK_ESCAPE`: Cancel draft or cancel drag
- `VK_RETURN`: Commit draft (polygon/ellipse finalization)
- Return `false` if not handled (allow caller to process)

**Tests Required:**
- `BoundsHandlerTest::OnKeyDown_Escape_CancelsDraft`
- `BoundsHandlerTest::OnKeyDown_Enter_CommitsPolygon`
- `BoundsHandlerTest::OnKeyDown_Enter_NoEffect_InSelectMode`

---

## Phase 3: Shape Rendering Layer

**Goal:** Create UI-layer shape rendering system (NO changes to ApertureCore).

**Duration:** 3-4 days  
**Dependencies:** Phase 2  
**Risk:** Medium

### 3.1 ShapeDrawStyle

**Location:** `DigitMode/Rendering/ShapeDrawStyle.h` (NEW)

```cpp
#pragma once
#include "ApertureCore/include/aperturecore/visibility/TypeLimits.h"

namespace DigitMode {

/**
 * @brief Visual appearance parameters for shape rendering
 * 
 * Fully decoupled from geometry - computed by UI layer.
 */
struct ShapeDrawStyle {
    enum class State {
        Idle,      ///< Not selected, not hovered
        Hovered,   ///< Mouse over shape
        Selected,  ///< Currently selected
        Dragging,  ///< Being dragged
        Draft      ///< Draft shape (not committed)
    };
    
    State state = State::Idle;
    aperture::TypeLimits type = aperture::TypeLimits::EXTERNAL;
    bool showHandles = false;  ///< True if handles should be drawn
    int activeHandle = -1;     ///< Index of handle being dragged (-1 = none)
    
    // Computed style properties
    COLORREF GetOutlineColor() const;
    int GetOutlineWidth() const;
    int GetOutlineStyle() const;  ///< PS_SOLID, PS_DASH, etc.
    COLORREF GetFillColor() const;
    bool HasFill() const;
};

} // namespace DigitMode
```

**Color Rules (from spec):**
- `EXTERNAL`: Green outline, no fill
- `APERTURE`: Cyan outline, no fill
- `INTERNAL`: Red dashed outline, semi-transparent red fill

**State Rules:**
- `Idle`: Thin outline
- `Hovered`: Thicker outline
- `Selected`: Thick outline, handles visible
- `Dragging`: Thick outline, active handle highlighted
- `Draft`: Dashed outline, vertex dots only

**Files to Create:**
- `DigitMode/Rendering/ShapeDrawStyle.h`
- `DigitMode/Rendering/ShapeDrawStyle.cpp`

---

### 3.2 IShapeRenderer Interface

**Location:** `DigitMode/Rendering/IShapeRenderer.h` (NEW)

```cpp
#pragma once
#include <afxwin.h>

namespace aperture { class Shape; }
namespace DigitMode { struct ShapeDrawStyle; }

namespace DigitMode {

/**
 * @brief Abstract renderer for a specific shape type
 * 
 * Lives in UI layer, knows about MFC/GDI.
 * Pulls pure geometry from Shape, applies ShapeDrawStyle.
 */
class IShapeRenderer {
public:
    virtual ~IShapeRenderer() = default;
    
    /**
     * @brief Draw shape to device context
     * @param shape Shape to render (geometry only)
     * @param dc Device context (screen/printer)
     * @param style Visual appearance
     * @param worldToScreen Transform from world to screen coordinates
     */
    virtual void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const class ViewTransform& worldToScreen
    ) const = 0;
    
    /**
     * @brief Draw handles for shape (if style.showHandles == true)
     */
    virtual void DrawHandles(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const class ViewTransform& worldToScreen
    ) const = 0;
};

} // namespace DigitMode
```

**Design Notes:**
- Pure interface, no shape-specific logic
- Takes `ViewTransform` for coordinate conversion (existing utility)
- Const methods (stateless rendering)

**Files to Create:**
- `DigitMode/Rendering/IShapeRenderer.h`

---

### 3.3 Concrete Renderers

**Location:** `DigitMode/Rendering/` (NEW folder)

**Classes:**
```cpp
class RectangleRenderer : public IShapeRenderer {
    void Draw(...) const override;
    void DrawHandles(...) const override;
};

class EllipseRenderer : public IShapeRenderer {
    void Draw(...) const override;
    void DrawHandles(...) const override;
};

class PolygonRenderer : public IShapeRenderer {
    void Draw(...) const override;
    void DrawHandles(...) const override;
};
```

**Implementation Example (RectangleRenderer):**
```cpp
void RectangleRenderer::Draw(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& transform
) const {
    // 1. Downcast to Rectangle (safe, enforced by dispatcher)
    const auto& rect = static_cast<const Rectangle&>(shape);
    
    // 2. Get corners in world coordinates
    auto corners = rect.corners();
    
    // 3. Transform to screen coordinates
    CPoint screenCorners[4];
    for (int i = 0; i < 4; ++i) {
        screenCorners[i] = transform.WorldToScreen(corners[i]);
    }
    
    // 4. Create pen based on style
    CPen pen(style.GetOutlineStyle(), style.GetOutlineWidth(), style.GetOutlineColor());
    CPen* oldPen = dc.SelectObject(&pen);
    
    // 5. Create brush (if needed)
    CBrush* oldBrush = nullptr;
    CBrush brush;
    if (style.HasFill()) {
        brush.CreateSolidBrush(style.GetFillColor());
        oldBrush = dc.SelectObject(&brush);
    } else {
        oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
    }
    
    // 6. Draw polygon
    dc.Polygon(screenCorners, 4);
    
    // 7. Restore DC
    dc.SelectObject(oldPen);
    dc.SelectObject(oldBrush);
}

void RectangleRenderer::DrawHandles(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& transform
) const {
    if (!style.showHandles) return;
    
    const auto& rect = static_cast<const Rectangle&>(shape);
    
    // Enumerate handles
    std::vector<aperture::HandleDesc> handles;
    rect.EnumerateHandles(handles);
    
    // Draw each handle
    for (size_t i = 0; i < handles.size(); ++i) {
        CPoint screenPos = transform.WorldToScreen(handles[i].position);
        
        // Highlight active handle
        COLORREF color = (i == style.activeHandle) ? RGB(255, 255, 0) : RGB(0, 255, 0);
        
        // Draw handle square
        CRect handleRect(screenPos.x - 4, screenPos.y - 4, screenPos.x + 4, screenPos.y + 4);
        dc.FillSolidRect(handleRect, color);
        dc.FrameRect(handleRect, CBrush(RGB(0, 0, 0)));
    }
}
```

**Files to Create:**
- `DigitMode/Rendering/RectangleRenderer.h`
- `DigitMode/Rendering/RectangleRenderer.cpp`
- `DigitMode/Rendering/EllipseRenderer.h`
- `DigitMode/Rendering/EllipseRenderer.cpp`
- `DigitMode/Rendering/PolygonRenderer.h`
- `DigitMode/Rendering/PolygonRenderer.cpp`

---

### 3.4 ShapeDrawDispatcher

**Location:** `DigitMode/Rendering/ShapeDrawDispatcher.h` (NEW)

```cpp
#pragma once
#include "IShapeRenderer.h"
#include <memory>
#include <map>

namespace aperture { enum class ShapeKind; }

namespace DigitMode {

/**
 * @brief Central shape rendering dispatcher
 * 
 * Selects appropriate renderer based on shape type.
 * Owns all concrete renderers.
 */
class ShapeDrawDispatcher {
public:
    ShapeDrawDispatcher();
    ~ShapeDrawDispatcher();
    
    /**
     * @brief Draw shape with specified style
     * @param shape Shape to render
     * @param dc Device context
     * @param style Visual appearance
     * @param transform World-to-screen transform
     */
    void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& transform
    ) const;
    
    /**
     * @brief Draw handles for shape (if applicable)
     */
    void DrawHandles(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& transform
    ) const;
    
private:
    const IShapeRenderer* GetRenderer(aperture::ShapeKind kind) const;
    
    std::unique_ptr<RectangleRenderer> m_rectangleRenderer;
    std::unique_ptr<EllipseRenderer> m_ellipseRenderer;
    std::unique_ptr<PolygonRenderer> m_polygonRenderer;
};

} // namespace DigitMode
```

**Implementation:**
```cpp
void ShapeDrawDispatcher::Draw(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& transform
) const {
    const IShapeRenderer* renderer = GetRenderer(shape.getKind());
    if (renderer) {
        renderer->Draw(shape, dc, style, transform);
    }
}

const IShapeRenderer* ShapeDrawDispatcher::GetRenderer(aperture::ShapeKind kind) const {
    switch (kind) {
        case aperture::ShapeKind::Rectangle:
            return m_rectangleRenderer.get();
        case aperture::ShapeKind::Ellipse:
            return m_ellipseRenderer.get();
        case aperture::ShapeKind::Polygon:
            return m_polygonRenderer.get();
        default:
            return nullptr;
    }
}
```

**Files to Create:**
- `DigitMode/Rendering/ShapeDrawDispatcher.h`
- `DigitMode/Rendering/ShapeDrawDispatcher.cpp`

---

### 3.5 Rendering Tests

**Strategy:** Use mock CDC or bitmap-based tests.

**Tests Required:**
- `RectangleRendererTest::Draw_Idle_ThinGreenOutline`
- `EllipseRendererTest::Draw_Selected_ThickCyanOutline`
- `PolygonRendererTest::DrawHandles_ShowsVertexHandles`
- `ShapeDrawDispatcherTest::Draw_DispatchesToCorrectRenderer`

**Files to Create:**
- `Tests/DigitModeTests/RectangleRendererTest.cpp`
- `Tests/DigitModeTests/EllipseRendererTest.cpp`
- `Tests/DigitModeTests/ShapeDrawDispatcherTest.cpp`

---

## Phase 4: Selection State Management

**Goal:** Track which shape is selected, hovered, being dragged.

**Duration:** 2 days  
**Dependencies:** Phase 3  
**Risk:** Low

### 4.1 BoundsSelectionManager

**Location:** `DigitMode/BoundsSelectionManager.h` (NEW)

**Note:** Separate from `SelectionManager` (which handles fringe selection).

```cpp
#pragma once
#include "ApertureCore/include/aperturecore/visibility/TypeLimits.h"
#include <optional>

namespace DigitMode {

/**
 * @brief Manages selection state for aperture shapes
 * 
 * Tracks:
 * - Currently selected shape (type, index)
 * - Hovered shape (for visual feedback)
 * 
 * Does NOT own shapes (CApertureCtrls owns them).
 */
class BoundsSelectionManager {
public:
    struct SelectedShape {
        aperture::TypeLimits type;
        size_t index;
        
        bool operator==(const SelectedShape& other) const {
            return type == other.type && index == other.index;
        }
    };
    
    /**
     * @brief Select a shape
     */
    void Select(aperture::TypeLimits type, size_t index);
    
    /**
     * @brief Clear selection
     */
    void ClearSelection();
    
    /**
     * @brief Check if a shape is selected
     */
    bool IsSelected(aperture::TypeLimits type, size_t index) const;
    
    /**
     * @brief Get currently selected shape
     */
    std::optional<SelectedShape> GetSelection() const { return m_selected; }
    
    /**
     * @brief Set hovered shape
     */
    void SetHovered(aperture::TypeLimits type, size_t index);
    
    /**
     * @brief Clear hover
     */
    void ClearHover();
    
    /**
     * @brief Check if a shape is hovered
     */
    bool IsHovered(aperture::TypeLimits type, size_t index) const;
    
    /**
     * @brief Get currently hovered shape
     */
    std::optional<SelectedShape> GetHovered() const { return m_hovered; }
    
private:
    std::optional<SelectedShape> m_selected;
    std::optional<SelectedShape> m_hovered;
};

} // namespace DigitMode
```

**Files to Create:**
- `DigitMode/BoundsSelectionManager.h`
- `DigitMode/BoundsSelectionManager.cpp`
- `Tests/DigitModeTests/BoundsSelectionManagerTest.cpp`

---

### 4.2 Integrate with BoundsHandler

**Location:** `DigitMode/BoundsHandler.h`

**Add Member:**
```cpp
class BoundsHandler {
public:
    /**
     * @brief Get selection manager
     */
    BoundsSelectionManager& GetSelectionManager() { return m_selection; }
    const BoundsSelectionManager& GetSelectionManager() const { return m_selection; }
    
private:
    BoundsSelectionManager m_selection;
};
```

**Update Hit-Testing:**
- On mouse move: update hover state via `m_selection.SetHovered()`
- On left click (Select mode): update selection via `m_selection.Select()`
- On mode change: clear selection if switching out of Select mode

---

## Phase 5: UI Integration (DEFERRED)

**Goal:** Wire BoundsHandler into CImageView for actual user interaction.

**Duration:** 2-3 days  
**Dependencies:** Phases 1-4  
**Risk:** Medium-High

**Note:** This phase is intentionally delayed to allow all core components to be built and tested independently.

### 5.1 CImageView::OnDraw Integration

**Location:** `ImageTempl/ImageView.cpp`

**Add Rendering Loop:**
```cpp
void CImageView::OnDraw(CDC* pDC) {
    // ...existing bitmap/fringe rendering...
    
    // Render aperture shapes
    if (m_pBoundsHandler && m_pApertureCtrls) {
        RenderApertureShapes(pDC);
    }
}

void CImageView::RenderApertureShapes(CDC* pDC) {
    auto& shapes = m_pApertureCtrls->GetShapes();
    auto& selection = m_pBoundsHandler->GetSelectionManager();
    
    // Render EXTERNAL shapes
    for (size_t i = 0; i < shapes.getExternal().size(); ++i) {
        ShapeDrawStyle style = ComputeShapeStyle(TypeLimits::EXTERNAL, i, selection);
        m_shapeDrawDispatcher.Draw(*shapes.getExternal()[i], *pDC, style, m_transform);
    }
    
    // Render INTERNAL shapes
    for (size_t i = 0; i < shapes.getInternal().size(); ++i) {
        ShapeDrawStyle style = ComputeShapeStyle(TypeLimits::INTERNAL, i, selection);
        m_shapeDrawDispatcher.Draw(*shapes.getInternal()[i], *pDC, style, m_transform);
    }
    
    // Render APERTURE shapes
    for (size_t i = 0; i < shapes.getApertures().size(); ++i) {
        ShapeDrawStyle style = ComputeShapeStyle(TypeLimits::APERTURE, i, selection);
        m_shapeDrawDispatcher.Draw(*shapes.getApertures()[i], *pDC, style, m_transform);
    }
    
    // Render draft shape (if any)
    if (m_pBoundsHandler->IsDrafting()) {
        const auto* draft = m_pBoundsHandler->GetDraftPreview();
        if (draft) {
            ShapeDrawStyle draftStyle;
            draftStyle.state = ShapeDrawStyle::State::Draft;
            draftStyle.type = /* current add mode type */;
            m_shapeDrawDispatcher.Draw(*draft, *pDC, draftStyle, m_transform);
        }
    }
    
    // Render handles for selected shape
    auto selected = selection.GetSelection();
    if (selected.has_value()) {
        const auto& shape = GetShape(selected->type, selected->index);
        ShapeDrawStyle handleStyle;
        handleStyle.state = ShapeDrawStyle::State::Selected;
        handleStyle.showHandles = true;
        m_shapeDrawDispatcher.DrawHandles(*shape, *pDC, handleStyle, m_transform);
    }
}

ShapeDrawStyle CImageView::ComputeShapeStyle(
    TypeLimits type,
    size_t index,
    const BoundsSelectionManager& selection
) const {
    ShapeDrawStyle style;
    style.type = type;
    
    if (selection.IsSelected(type, index)) {
        style.state = m_pBoundsHandler->IsDragging() 
            ? ShapeDrawStyle::State::Dragging 
            : ShapeDrawStyle::State::Selected;
        style.showHandles = !m_pBoundsHandler->IsDragging();
    } else if (selection.IsHovered(type, index)) {
        style.state = ShapeDrawStyle::State::Hovered;
    } else {
        style.state = ShapeDrawStyle::State::Idle;
    }
    
    return style;
}
```

**Files to Modify:**
- `ImageTempl/ImageView.h` (add members, method declarations)
- `ImageTempl/ImageView.cpp` (implement rendering)

---

### 5.2 Mouse Event Routing

**Location:** `ImageTempl/ImageView.cpp`

**Update Handlers:**
```cpp
void CImageView::OnLButtonDown(UINT nFlags, CPoint point) {
    if (m_editMode == EditMode::Select) {
        // Hit-test and begin drag
        auto hit = m_pBoundsHandler->HitTest(point);
        if (hit.hit) {
            if (hit.isControlPoint()) {
                m_pBoundsHandler->BeginDrag(hit.type, hit.shapeIndex, hit.controlPointIndex, point);
            } else {
                // Select shape
                m_pBoundsHandler->GetSelectionManager().Select(hit.type, hit.shapeIndex);
            }
        }
    } else if (IsAddMode(m_editMode)) {
        // Add point to draft
        aperture::Point worldPt = m_transform.ScreenToWorld(point);
        m_pBoundsHandler->AddDraftPoint(worldPt);
    } else if (m_editMode == EditMode::Delete) {
        // Delete clicked shape
        auto hit = m_pBoundsHandler->HitTest(point);
        if (hit.hit) {
            // Create RemoveShapeCommand
        }
    }
    
    Invalidate();
}

void CImageView::OnMouseMove(UINT nFlags, CPoint point) {
    if (m_pBoundsHandler->IsDragging()) {
        m_pBoundsHandler->UpdateDrag(point);
        Invalidate();
    } else {
        // Update hover state
        auto hit = m_pBoundsHandler->HitTest(point);
        if (hit.hit) {
            m_pBoundsHandler->GetSelectionManager().SetHovered(hit.type, hit.shapeIndex);
        } else {
            m_pBoundsHandler->GetSelectionManager().ClearHover();
        }
        Invalidate();
    }
}

void CImageView::OnLButtonUp(UINT nFlags, CPoint point) {
    if (m_pBoundsHandler->IsDragging()) {
        m_pBoundsHandler->EndDrag(true);  // Commit
        Invalidate();
    }
}

void CImageView::OnRButtonUp(UINT nFlags, CPoint point) {
    if (m_pBoundsHandler->IsDrafting()) {
        // Finalize polygon or cancel draft
        if (m_pBoundsHandler->GetEditMode() == EditMode::AddPolygon) {
            m_pBoundsHandler->CommitDraft();
        } else {
            m_pBoundsHandler->CancelDraft();
        }
        Invalidate();
    }
}

void CImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
    if (m_pBoundsHandler->OnKeyDown(nChar, nRepCnt, nFlags)) {
        Invalidate();
        return;
    }
    
    // Fall through to base class
    CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}
```

**Files to Modify:**
- `ImageTempl/ImageView.cpp` (update mouse/keyboard handlers)

---

### 5.3 Mode UI (Toolbar, Status Bar, Cursor)

**Location:** `ImageTempl/ImageView.cpp` + resource files

**Toolbar:**
- Add buttons for each EditMode
- Update button states based on current mode
- Wire button handlers to `m_pBoundsHandler->SetEditMode()`

**Status Bar:**
- Show current mode name
- Show point count for draft shapes
- Show handle type during drag

**Cursor:**
- Update cursor based on mode and hover state
- Use `GetEditModeCursor()` utility

**Files to Modify:**
- `Resource.h` (add command IDs)
- `Digit.rc` (add toolbar buttons)
- `ImageTempl/ImageView.cpp` (wire handlers)

---

## Phase 6: Testing & Validation

**Goal:** Comprehensive integration testing before release.

**Duration:** 2-3 days  
**Dependencies:** Phase 5  
**Risk:** Low

### 6.1 Integration Tests

**Test Scenarios:**
1. Create rectangle via 3 clicks, verify undo/redo
2. Create ellipse via 5 clicks, verify LSM fit quality
3. Create circle, verify radiusX == radiusY
4. Create polygon, verify self-intersection rejection
5. Drag handle, verify preview updates, verify command dispatch
6. Switch modes mid-draft, verify draft is cancelled
7. Delete shape, verify undo restores
8. Multi-shape overlap, verify selection cycling

**Files to Create:**
- `Tests/IntegrationTests/ApertureEditingIntegrationTest.cpp`

---

### 6.2 Performance Testing

**Benchmarks:**
- Rendering 100+ shapes at 60fps
- Hit-testing with dense shape collections
- Undo/redo stack with 1000+ commands

**Optimization Targets:**
- Spatial indexing for hit-testing (if needed)
- Dirty rectangle tracking for rendering
- Command memory pooling (if needed)

---

### 6.3 Visual Regression Tests

**Strategy:**
- Capture screenshots of various states
- Compare against baseline images
- Detect rendering regressions

**Tools:**
- Manual inspection + screenshot archiving
- Future: Automated image comparison

---

## Risk Mitigation

### High-Risk Areas

1. **Rectangle 3-Point Constructor Math**
   - **Risk:** Incorrect center/rotation calculation
   - **Mitigation:** Extensive unit tests with known geometries

2. **Circle vs Ellipse LSM Fitting**
   - **Risk:** Numerical instability, poor fits
   - **Mitigation:** Test with synthetic data, validate residuals

3. **Rendering Performance**
   - **Risk:** Slow rendering with many shapes
   - **Mitigation:** Profile early, optimize critical paths

4. **Undo/Redo Edge Cases**
   - **Risk:** Stale pointers after shape removal
   - **Mitigation:** Commands store deep copies, never pointers

### Medium-Risk Areas

1. **Mode Switching**
   - **Risk:** State leaks between modes
   - **Mitigation:** Explicit state clearing in `SetEditMode()`

2. **Coordinate Transformations**
   - **Risk:** Screen/world conversion errors
   - **Mitigation:** Reuse existing `ViewTransform`, add tests

3. **Handle Hit-Testing**
   - **Risk:** Tiny targets, user frustration
   - **Mitigation:** Generous tolerance, visual feedback

---

## Success Criteria

### Phase 1 (ApertureCore)
- ✅ Rectangle 3-point constructor tests pass
- ✅ Circle LSM fitting produces equal radii
- ✅ All shape constructors documented

### Phase 2 (Draft Shapes)
- ✅ DraftShape can create all shape types
- ✅ BoundsHandler modes switch correctly
- ✅ Keyboard input handled properly

### Phase 3 (Rendering)
- ✅ All shapes render with correct colors
- ✅ State-based appearance works
- ✅ Handles drawn only when selected

### Phase 4 (Selection)
- ✅ Selection state persists correctly
- ✅ Hover updates in real-time
- ✅ Multi-shape overlap handled

### Phase 5 (UI Integration)
- ✅ Mouse events routed correctly
- ✅ Rendering integrated into OnDraw
- ✅ Mode UI functional

### Phase 6 (Testing)
- ✅ All integration tests pass
- ✅ No performance regressions
- ✅ Visual regression baseline established

---

## Timeline Estimate

| Phase | Duration | Start After | Complexity |
|-------|----------|-------------|------------|
| Phase 1: ApertureCore Extensions | 1-2 days | Immediately | Low |
| Phase 2: Draft Shapes | 2-3 days | Phase 1 | Medium |
| Phase 3: Rendering Layer | 3-4 days | Phase 2 | Medium |
| Phase 4: Selection Management | 2 days | Phase 3 | Low |
| Phase 5: UI Integration | 2-3 days | Phase 4 | Medium-High |
| Phase 6: Testing & Validation | 2-3 days | Phase 5 | Low |
| **Total** | **12-17 days** | | |

**Critical Path:** Phase 1 → 2 → 3 → 5 → 6  
**Parallelizable:** Phase 4 can start during Phase 3

---

## Open Questions

1. **Circle Tool Justification:** Do users actually need a separate circle mode, or can they just use Shift+Drag on ellipse handles?
   - **Answer:** Spec says explicit circle mode for perimeter-based creation (no center click)

2. **Self-Intersection Detection:** How strict should polygon validation be?
   - **Answer:** Reject on commit, show visual feedback during draft

3. **Undo Stack Size:** Should there be a limit on command history?
   - **Answer:** Defer to existing CommandDispatcher configuration

4. **Shape Type Toggle:** Should users be able to change EXTERNAL ↔ INTERNAL ↔ APERTURE post-creation?
   - **Answer:** Not in spec, defer to future feature

---

## Appendix A: File Structure

```
Digit/
├── ApertureCore/
│   ├── include/aperturecore/geometry/
│   │   ├── Rectangle.h         [MODIFY: add 3-point constructor]
│   │   └── Ellipse.h           [MODIFY: add FitCircle/FitEllipse static methods]
│   └── src/geometry/
│       ├── Rectangle.cpp       [MODIFY: implement 3-point constructor]
│       └── Ellipse.cpp         [MODIFY: implement LSM fitting]
│
├── DigitMode/
│   ├── DraftShape.h            [NEW]
│   ├── DraftShape.cpp          [NEW]
│   ├── EditMode.h              [NEW]
│   ├── EditMode.cpp            [NEW]
│   ├── BoundsHandler.h         [MODIFY: add draft + mode management]
│   ├── BoundsHandler.cpp       [MODIFY: implement draft logic]
│   ├── BoundsSelectionManager.h [NEW]
│   ├── BoundsSelectionManager.cpp [NEW]
│   └── Rendering/
│       ├── ShapeDrawStyle.h    [NEW]
│       ├── ShapeDrawStyle.cpp  [NEW]
│       ├── IShapeRenderer.h    [NEW]
│       ├── RectangleRenderer.h [NEW]
│       ├── RectangleRenderer.cpp [NEW]
│       ├── EllipseRenderer.h   [NEW]
│       ├── EllipseRenderer.cpp [NEW]
│       ├── PolygonRenderer.h   [NEW]
│       ├── PolygonRenderer.cpp [NEW]
│       ├── ShapeDrawDispatcher.h [NEW]
│       └── ShapeDrawDispatcher.cpp [NEW]
│
├── ImageTempl/
│   ├── ImageView.h             [MODIFY: add rendering members]
│   └── ImageView.cpp           [MODIFY: integrate OnDraw, mouse events]
│
└── Tests/
    ├── ApertureCoreTests/
    │   ├── RectangleTest.cpp   [MODIFY: add 3-point tests]
    │   └── EllipseTest.cpp     [MODIFY: add LSM tests]
    ├── DigitModeTests/
    │   ├── DraftShapeTest.cpp  [NEW]
    │   ├── BoundsHandlerTest.cpp [MODIFY: add draft tests]
    │   ├── BoundsSelectionManagerTest.cpp [NEW]
    │   ├── RectangleRendererTest.cpp [NEW]
    │   ├── EllipseRendererTest.cpp [NEW]
    │   └── ShapeDrawDispatcherTest.cpp [NEW]
    └── IntegrationTests/
        └── ApertureEditingIntegrationTest.cpp [NEW]
```

---

## Appendix B: Key Architecture Invariants

**MUST NOT VIOLATE:**
1. Shapes never contain rendering code
2. Commands never bypass ShapeCollection
3. BoundsHandler never mutates shapes directly
4. ApertureCore never depends on MFC
5. Rendering layer never stores shape state
6. Draft shapes never enter ShapeCollection

**MUST ENFORCE:**
1. All shape edits go through Commands
2. NotifyShapeModified() called after every mutation
3. Preview-only editing in BoundsHandler
4. Stateless renderers
5. Explicit mode switching
6. Deep copies in Commands

---

## Appendix C: Validation Checklist

**Before submitting each phase:**
- [ ] All unit tests pass
- [ ] No warnings in build output
- [ ] Code reviewed against spec
- [ ] Architecture invariants verified
- [ ] Documentation updated
- [ ] No ApertureCore → MFC dependencies
- [ ] No direct shape mutations outside Commands

---

**End of Implementation Plan**
