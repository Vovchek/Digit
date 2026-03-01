# Coordinate System Marking Summary

## Overview
This document summarizes the coordinate system reviews conducted on the Digit project mouse input and coordinate-dependent code. Each parameter or method dealing with coordinates has been marked with either **Screen** or **World** to clarify which coordinate system is being used.

---

## Coordinate Systems Defined

### Screen Coordinates
- Used by **Windows mouse events** (WM_LBUTTONDOWN, WM_MOUSEMOVE, etc.)
- Measured in **client/viewport pixels** relative to the window's upper-left corner
- Type: `CPoint` (integer coordinates)
- Examples: Mouse cursor position, window dimensions

### World Coordinates
- Used by **domain logic** (fringe segments, geometric operations)
- Measured in **image/document pixels** after view transform conversion
- Type: `CDPoint` (double precision coordinates)
- Examples: Fringe points, shape vertices, aperture boundaries

---

## Files Updated with Coordinate System Annotations

### 1. **DigitMode\CFringeSegment.h**
Hit testing methods marked with coordinate systems:
- `FindNearestPoint(CPoint screenP /* Screen */, ...)`  
- `IsPointOnPolyline(CPoint P /* Screen */, ...)`

### 2. **Utils\Tracker.h**
Tracker methods dealing with mouse input:
- `Track(CWnd* pW, CDC* pDC, CPoint P /* Screen */, ...)`
- `IsPointInside(CDC* pDC, CPoint l_point /* Screen */)`

### 3. **DigitMode\FringeInputHandler.h**
IInputHandler interface methods (receive screen coordinates from Windows):
- `OnMouseDown(UINT flags, CPoint pt /* Screen */)`
- `OnMouseMove(UINT flags, CPoint pt /* Screen */)`
- `OnMouseUp(UINT flags, CPoint pt /* Screen */)`
- `OnMouseWheel(UINT flags, short delta, CPoint pt /* Screen */)`

### 4. **DigitMode\FringeInputHandler.cpp**
Implementation shows conversion point:
```cpp
CDPoint worldPt = ...;
if (m_pTransform) {
    CPoint worldPtScreen = m_pTransform->ScreenToWorld(pt /* Screen */);
    worldPt = CDPoint(worldPtScreen.x, worldPtScreen.y);
}
// Now passes worldPt /* World */ to domain handlers
m_inputHandler.OnLButtonDown(..., worldPt /* World */, ...);
```

### 5. **DigitMode\BoundsInputHandler.h**
Similar to FringeInputHandler:
- `OnMouseDown(UINT flags, CPoint pt /* Screen */)`
- `OnMouseMove(UINT flags, CPoint pt /* Screen */)`
- `OnMouseUp(UINT flags, CPoint pt /* Screen */)`
- `OnMouseWheel(UINT flags, short delta, CPoint pt /* Screen */)`
- Helper methods: `HandleSelectModeMouseDown(UINT flags, CPoint pt /* Screen */)`

### 6. **DigitMode\HitTester.h**
Hit testing operates on world coordinates:
- `HitTest(CDPoint P /* World */, ...)`
- `DotDistance(CDPoint P /* World */, CDPoint dot /* World */)`
- `DistanceToSegment(CDPoint P /* World */, CDPoint A /* World */, CDPoint B /* World */)`

### 7. **DigitMode\InputHandler.h**
Domain logic methods working with world coordinates:
- **Pan/Zoom** (screen coordinates):
  - `OnMouseWheel(const CPoint& pt /* Screen */, ...)`
  - `BeginPan(const CPoint& pt /* Screen */)`
  - `ContinuePan(const CPoint& pt /* Screen */, ViewTransform* view)`

- **Drawing** (world coordinates):
  - `StartNewSegment(CDPoint P /* World */, ...)`
  - `OnMouseMove(CDPoint pt /* World */, ...)`
  - `OnLButtonDown(UINT flags, CDPoint pt /* World */, ...)`
  - `OnRButtonDown(UINT flags, CDPoint pt /* World */, ...)`

- **Drag Lifecycle** (world coordinates):
  - `BeginDotDrag(int segIdx, int dotIdx, CDPoint start /* World */, ...)`
  - `BeginEdgeDrag(int segIdx, int edgeStartIdx, CDPoint start /* World */, ...)`

### 8. **DigitMode\BoundsHandler.h**
Mixed coordinate systems depending on operation:
- **Screen-based hit testing**:
  - `HitTest(const CPoint& screenPt /* Screen */, ...)`
  - `UpdateHoveredHandle(const CPoint& screenPt /* Screen */)`
  - `BeginDrag(..., const CPoint& screenStart /* Screen */)`
  - `UpdateDrag(const CPoint& screenCurrent /* Screen */)`

- **Coordinate conversions** (Screen ↔ World):
  - `ScreenToWorldDouble(const CPoint& screenPt /* Screen */) → CPoint2d /* World */`
  - `WorldToAperturePoint(const CPoint2d& worldPt /* World */)`
  - `ScreenToAperturePoint(const CPoint& screenPt /* Screen */) → aperture::Point /* World */`

- **Draft operations** (world coordinates):
  - `AddDraftPoint(const aperture::Point& worldPt /* World */)`

### 9. **ImageTempl\ImageView.h**
Helper methods for mouse-driven operations:
- `DrawMouseMoveCrossedLines(CPoint P /* Screen */)`
- `BeginLine(CPoint P /* Screen */)`
- `EndLine(CPoint P2 /* Screen */)`
- `DrawMouseMoveMeasureLine(CPoint P2 /* Screen */)`
- `BeginDragDot(CPoint P /* Screen */)`
- `DragDot(CPoint P /* Screen */, ...)`
- `DropDot(CPoint P /* Screen */)`
- `BeginDragZapSection(CPoint P /* Screen */)`
- `DragZapSection(CPoint P /* Screen */, ...)`
- `DropZapSection(CPoint P /* Screen */)`

### 10. **MGTools\Include\Graph\2DGraph.h**
Graph view coordinate conversions:
- `MatToWndCoor(int iArea, double x /* World */, double y /* World */, long int& xC /* Screen */, long int& yC /* Screen */)`
- `WndCoorToMat(int iArea, CPoint P /* Screen */, double &x /* World */, double &y /* World */)`
- `OnRButtonDown(UINT nFlags, CPoint point /* Screen */)`

### 11. **ImageTempl\ViewTransform.h**
Core coordinate transformation library:
- **Zoom operations** (screen-based):
  - `ZoomAt(const CPoint& clientPt /* Screen */, double factor)`
  - `ZoomAtClient(const CPoint& clientPt /* Screen */, ...)`

- **Zoom to fit**:
  - `ZoomToFit(const CRect& imageRect /* World */, const CRect& clientRect /* Screen */)`

- **Pan**:
  - `PanBy(const CPoint& deltaScreen /* Screen */)`

- **Core conversions**:
  - `ScreenToWorld(const CPoint& pt /* Screen */) → CPoint2d /* World */`
  - `WorldToScreen(const CPoint2d& wpt /* World */) → CPoint /* Screen */`

---

## Coordinate Flow Diagram

```
Windows Mouse Event (WM_LBUTTONDOWN, etc.)
        ↓
    CPoint (Screen)
        ↓
FringeInputHandler::OnMouseDown(CPoint pt /* Screen */)
        ↓
Convert: ViewTransform::ScreenToWorld(pt) → CPoint2d /* World */
        ↓
InputHandler::OnLButtonDown(CDPoint pt /* World */)
        ↓
Domain Logic (CFringeSegment, HitTester, etc.)
        ↓
All operations use CDPoint /* World */
```

---

## Best Practices

1. **Always mark parameters** in headers when dealing with coordinates
2. **Conversion points** should be clearly visible (typically in input handlers)
3. **Domain logic** should work exclusively with World coordinates
4. **UI layer** (views, input handlers) should work with Screen coordinates and convert at boundaries
5. **Use comments** like `/* Screen */` and `/* World */` consistently for clarity

---

## Files NOT Modified
- `InterfSolver\Tools\XYPoint.h` - This is a generic point structure without mouse input context
- Legacy code in `Utils\Tracker.cpp` - Already marked where needed, implementation uses DC context
- Other graph tools and scientific computation utilities

---

## Verification Steps

To verify these markings are correct:
1. Build the solution to check for syntax errors
2. Review mouse event handlers to ensure conversion happens at boundaries
3. Trace a mouse event from Windows through the entire input pipeline
4. Ensure domain logic (fringe editing, hit testing) never receives Screen coordinates directly
5. Ensure UI code (drawing, hit testing against user input) converts to appropriate system before use

