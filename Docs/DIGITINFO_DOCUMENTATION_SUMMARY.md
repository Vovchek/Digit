# CDigitInfo Class Documentation - Summary

## Overview
Comprehensive Doxygen-style documentation has been added to the CDigitInfo class, providing clear domain-specific meanings for all member variables and methods.

## Documentation Coverage

### Class-Level Documentation ✅
- **Domain Concepts**: Detailed explanations of interferogram terminology
- **Data Model**: Segment-primary architecture explained
- **Workflow**: Step-by-step automatic digitization process
- **Transition Strategy**: Legacy vs. new model clarification

### Member Variables (Fully Documented) ✅

#### Aperture/Obstruction Mask
- `buf_line` - 2D array defining valid pixel regions
- `ny_buf_line` - Array dimensions
- **Purpose**: Masks for fringe detection boundaries

#### Scan Line Analysis
- `Sections` - Horizontal scan sections with extrema
- `HidenDots` - Detected intensity extrema (intermediate)
- **Purpose**: Initial fringe center detection

#### ZAP Section Data
- `ZapLines` - Horizontal reference scan lines
- `HandSetZapLines` - Manual vs. auto-generated flag
- `idxDragZapLine` - Currently dragged section
- **Purpose**: Fringe identification and numbering

#### Legacy Model (Transitional)
- `Dots` - Flat array of all dots (DEPRECATED)
- `idxDragDot` - Dragged dot index (DEPRECATED)
- `idxMainDot` - Selected dot index (DEPRECATED)
- **Status**: Being phased out

#### Segment-Primary Model (Current)
- `Fringes` - Vector of CFringeSegment objects (PRIMARY)
- `m_bUseFringeModel` - Model selection flag
- `idxDraggedPoint` - Dragged point (iFringe, iPoint)
- `idxMainPoint` - Selected point (iFringe, iPoint)
- **Status**: CURRENT, recommended model

#### Selection Management
- `selectionManager` - Navigate mode selection (UX v1.0)
- **Features**: Hierarchical selection, box select, glow effect

#### Fringe Numbering
- `CurrentNumber` - Next fringe number to assign
- `numStep` - Increment step (0.5, 1.0, etc.)
- `MainFringeNumber` - Reference fringe
- **Purpose**: Sequential numbering control

#### Analysis Parameters
- `idxMainSection` - Reference scan line
- `SecSegm` - Average fringe spacing (pixels)
- `CorrectionSecSegm` - Spacing tolerance
- **Purpose**: Quality metrics and validation

#### Geometry Flags
- `isInsideScreen` - TRUE if obstruction present
- **Purpose**: Affects numbering strategy

#### Metadata
- `Comments` - User annotations
- `ScaleFactor` - Pixel-to-physical units
- `Rotation` - Rotation angle (degrees)
- **Purpose**: Measurement context

### Methods (Fully Documented) ✅

#### Memory Management (3 methods)
- `Init()` - Initialize to default state
- `Init_buf_line()` - Allocate aperture mask
- `Delete_buf_line()` - Free mask memory

#### Automatic Digitization (10 methods)
- `Auto()` - Complete workflow (MAIN ENTRY POINT)
- `CreateBufLine*()` - Aperture/obstruction masks (4 variants)
- `CreateRedCenters()` - Detect extrema
- `SelectFringeStep()` - Calculate spacing
- `SelectMainSection()` - Choose reference line
- `CreateNumLines()` - Build polylines
- `CorrectNumbers()` - Fix numbering issues

#### Manual Editing (12 methods)
- `AddDot()`, `RemoveDot()`, `RemoveFringe()`
- `AddZapSection()`, `DeleteZapSection()`
- `RenumFringe()`, `RenumDot()`
- `NumberPlus()`, `NumberMinus()`
- `SectionLeft()`, `SectionRight()`

#### Fringe Construction (6 methods)
- `CreateNumLines()` - Connect extrema
- `NumberingLine()` - Assign numbers
- `SelectMainFringe()` - Choose reference
- `CorrectNumbers()` - Fix inconsistencies
- `SelectNumber()` - Validate number assignment

#### ZAP Section Management (6 methods)
- `CreateZAPSections()` - Generate horizontal lines
- `SortZapLines()` - Order by position
- `PutDotsOnZAPSections()` - Associate fringes

#### Dot Navigation (15 methods)
- `SelectMainDot()` - Select primary dot
- `LockDot()` / `IsLockedDot()` - Drag operations
- `GetDot()`, `GetFirstDotInSection()`, `GetNextDotInSection()`
- `IsDotUnderCursor()` - Hit testing
- `GetFringeDots()` - Query by number

#### Keyboard Navigation (1 method)
- `OnKeyDown()` - Arrow key navigation
  - Left/Right: Along fringe
  - Up/Down: Between fringes

#### File I/O (8 methods)
- `Load()` / `Save()` - Auto-detect format
- `LoadZAP()` / `SaveZAP()` - Legacy format
- `LoadFRN()` / `SaveFRN()` - Modern format
- `CollectNumberingInterferogramInfo()` - Export prep
- `ExamineNumberingInterferogramInfo()` - Import validation

#### Segment-Primary Model (10 methods)
- `CreateFringe()` / `DeleteFringe()` - Lifecycle
- `GetFringe()` - Accessor (const & non-const)
- `FindFringesByNumber()` - Query logical fringe
- `AddPointToFringe()` / `InsertPointInFringe()` - Geometry
- `RemovePointFromFringe()` / `MovePointInFringe()`
- `FindPointUnderCursor()` - Hit testing

## Documentation Quality

### Domain Clarity ✅
Each member variable includes:
- **Purpose**: What it represents in domain
- **Usage**: When/how it's used
- **Lifecycle**: When created/destroyed
- **Relationships**: What it connects to

### Workflow Documentation ✅
Key workflows explained:
- **Automatic Digitization**: 9-step process from image to numbered fringes
- **Manual Editing**: User-driven modifications
- **Navigation**: Keyboard and mouse interaction

### Model Transition ✅
Clear guidance on:
- **Legacy model**: Dots array (deprecated)
- **Current model**: Fringes vector (recommended)
- **Flag**: `m_bUseFringeModel` controls selection
- **Sync methods**: `SyncDotsToFringes()` / `SyncFringesToDots()`

### Technical Details ✅
Documented:
- **Algorithms**: Red center detection, fringe connection
- **Data structures**: buf_line format, SelectedPoint
- **Performance**: Complexity notes where relevant
- **Safety**: Bounds checking, nullptr returns

## Doxygen Tags Used

### Structural Tags
- `@brief` - Short description
- `@param` - Parameter description
- `@return` - Return value
- `@see` - Cross-references

### Documentation Tags
- `@section` - Major topics in class header
- `@deprecated` - Legacy features
- `@todo` - Future improvements
- `@warning` - Cautionary notes

### Formatting Tags
- `**Bold**` - Emphasis
- *Italic* - Variables
- `Code` - Inline code
- Code blocks - Multi-line examples

## Domain Terminology Glossary

### Optical Concepts
- **Interferogram**: Pattern of light/dark fringes from interference
- **Fringe**: Continuous curve of constant optical path difference
- **Phase**: Optical phase represented by fringe number

### Detection Concepts
- **Red Center**: Local intensity extremum (fringe center candidate)
- **Extremum**: Maximum or minimum in intensity profile
- **Scan Line**: Horizontal slice through image

### Geometric Concepts
- **Aperture**: Outer boundary (usually circular)
- **Obstruction**: Inner boundary (central hole)
- **Segment**: Continuous polyline (primary object)
- **Polyline**: Ordered sequence of connected points

### Numbering Concepts
- **Number**: Fringe identifier (e.g., 1.0, 1.5, 2.0)
- **Step**: Increment between sequential fringes
- **Main Fringe**: Reference fringe for numbering origin

## Usage Examples in Documentation

### Example 1: Creating a Fringe
```cpp
// Create new fringe with number 2.5
int iFringe = digitInfo.CreateFringe(2.5, -1);

// Add points to build polyline
digitInfo.AddPointToFringe(iFringe, CDPoint(100, 200));
digitInfo.AddPointToFringe(iFringe, CDPoint(105, 210));
digitInfo.AddPointToFringe(iFringe, CDPoint(110, 220));
```

### Example 2: Finding Fringes by Number
```cpp
// Find all segments belonging to fringe 3.0
std::vector<int> indices;
digitInfo.FindFringesByNumber(3.0, indices);

// Now indices contains all segments with Number=3.0
for (int idx : indices) {
    CFringeSegment* pFringe = digitInfo.GetFringe(idx);
    // Process each segment...
}
```

### Example 3: Buffer Line Structure
```cpp
// buf_line[scanline][boundary] structure:
// [0] = left aperture edge
// [1] = right aperture edge  
// [2] = left obstruction edge (-1 if none)
// [3] = right obstruction edge (-1 if none)

int leftAperture = buf_line[100][0];
int rightAperture = buf_line[100][1];
bool hasObstruction = (buf_line[100][2] != -1);
```

## Documentation Benefits

### For New Developers ✅
- **Domain understanding**: Clear explanation of interferogram concepts
- **Architecture**: Segment-primary model vs. legacy
- **Entry points**: Where to start (`Auto()`, `CreateFringe()`)
- **Workflows**: Step-by-step processes

### For Maintenance ✅
- **Purpose clarity**: Why each variable exists
- **Relationships**: How components connect
- **Lifecycle**: When things are created/destroyed
- **Edge cases**: Warnings and limitations

### For API Users ✅
- **Method signatures**: Parameter meanings
- **Return values**: What to expect
- **Examples**: How to use properly
- **Cross-references**: Related methods via `@see`

### For Code Review ✅
- **Intent**: Clear purpose statements
- **Deprecation**: What to avoid
- **Best practices**: Recommended approaches
- **Warnings**: Known pitfalls

## Future Documentation Tasks

### Additional Areas to Document
- [ ] CFringeSegment class (referenced extensively)
- [ ] CSectionInfo class (scan line data)
- [ ] CZapLineInfo class (horizontal sections)
- [ ] CDotInfo class (legacy model)
- [ ] NUMBERING_INTERFEROGRAM_INFO structure

### Documentation Improvements
- [ ] Add sequence diagrams for workflows
- [ ] Add state diagrams for mode transitions
- [ ] Add UML class diagrams
- [ ] Add algorithm pseudocode

### Examples to Add
- [ ] Complete Auto() workflow example
- [ ] Manual fringe editing workflow
- [ ] File I/O examples
- [ ] Error handling patterns

## Build Status
✅ **Documentation added successfully**  
✅ **No compilation errors**  
✅ **Duplicate selectionManager removed**  
✅ **Ready for Doxygen generation**

## Doxygen Generation

To generate HTML documentation:
```bash
# In project root directory
doxygen Doxyfile
```

Expected output:
- **HTML docs**: `docs/html/index.html`
- **Class index**: All CDigitInfo members with descriptions
- **Call graphs**: Method relationships (if GraphViz installed)
- **Cross-references**: Clickable `@see` links

---

**Documentation complete! CDigitInfo class now has comprehensive domain-specific documentation suitable for developers, maintainers, and API users.** 📚✨
