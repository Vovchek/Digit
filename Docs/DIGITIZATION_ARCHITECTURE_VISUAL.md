# Digitization Algorithm - Final Architecture Diagram

## Complete Data Flow

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        CDigitInfo::Auto()                                   │
│  - Gets image data, aperture mask, parameters                              │
│  - Orchestrates the complete pipeline                                       │
└────────────────────────────┬────────────────────────────────────────────────┘
                             │
                             ↓
            ┌────────────────────────────────┐
            │  DigitizationInput (POD)       │
            ├────────────────────────────────┤
            │ - bitmap data                  │
            │ - image dimensions             │
            │ - visibility mask provider     │
            │ - aperture center (point)      │
            │ - algorithm parameters         │
            └────────────────────────────────┘
                             │
                             ↓
        ╔════════════════════════════════════════════════════════════════╗
        ║           PURE ALGORITHM (StandardDigitizer)                  ║
        ║     No MFC dependencies, fully testable, replaceable          ║
        ╠════════════════════════════════════════════════════════════════╣
        ║                                                                ║
        ║  ┌──────────────────────────────────────────────────────────┐ ║
        ║  │ Stage 1: RedCenterDetector                               │ ║
        ║  │ - Scans image lines horizontally                         │ ║
        ║  │ - Queries visibility mask for valid pixel ranges        │ ║
        ║  │ - Detects intensity extrema (FC_MAX / FC_MIN / MINMAX)  │ ║
        ║  │ INPUT: bitmap + mask                                    │ ║
        ║  │ OUTPUT: vector<ExtremumPoint> (red dots)               │ ║
        ║  └──────────────────────────────────────────────────────────┘ ║
        ║                             ↓                                  ║
        ║  ┌──────────────────────────────────────────────────────────┐ ║
        ║  │ Stage 2: FringeConnector                                 │ ║
        ║  │ - Connects extrema from scan line to scan line           │ ║
        ║  │ - Uses visibility mask for continuity validation        │ ║
        ║  │ - Builds polylines representing fringes                 │ ║
        ║  │ INPUT: extrema + mask                                   │ ║
        ║  │ OUTPUT: vector<FringePolyline>                         │ ║
        ║  └──────────────────────────────────────────────────────────┘ ║
        ║                             ↓                                  ║
        ║  ┌──────────────────────────────────────────────────────────┐ ║
        ║  │ Stage 3: FringeNumberer                                  │ ║
        ║  │ - Calculates fringe spacing from center                  │ ║
        ║  │ - Assigns sequential numbers to polylines                │ ║
        ║  │ - Identifies main fringe index                           │ ║
        ║  │ INPUT: polylines + aperture center (point)              │ ║
        ║  │ OUTPUT: vector<NumberedFringe> (PURE STL, NO MFC)      │ ║
        ║  └──────────────────────────────────────────────────────────┘ ║
        ║                                                                ║
        ╚════════════════════════════════════════════════════════════════╝
                             │
                             ↓
            ┌────────────────────────────────────┐
            │ DigitizationOutput (POD)           │
            ├────────────────────────────────────┤
            │ ✓ redCenters (for visualization)   │
            │ ✓ fringes (vector<NumberedFringe>) │
            │ ✓ averageFringeStep                │
            │ ✓ mainFringeNumber                 │
            └────────────────────────────────────┘
                             │
                             ↓
        ┌────────────────────────────────────────────────┐
        │   Stage 4: FringeSegmentAdapter (Bridge)      │
        │   ─────────────────────────────────────────    │
        │   Converts pure output to MFC-compatible      │
        │   INPUT: vector<NumberedFringe>               │
        │   OUTPUT: vector<CFringeSegment>              │
        └────────────────────────────────────────────────┘
                             │
                             ↓
        ┌────────────────────────────────────────────────┐
        │      CDigitInfo Legacy Storage (MFC)          │
        ├────────────────────────────────────────────────┤
        │ - Fringes (vector<CFringeSegment>)            │
        │ - HidenDots (red centers from Stage 1)        │
        │ - MainFringeNumber                            │
        │ - Dots (legacy, if needed)                    │
        └────────────────────────────────────────────────┘
```

---

## Data Structure Hierarchy

```
POD Input Structures (Pure STL)
├── DigitizationInput
│   └── Contains: bitmap, mask provider, aperture center, parameters
└── No MFC types

Pure Algorithm Output (Stages 1-3)
├── ExtremumPoint
│   ├── position (CDPoint)
│   ├── scanLineIndex
│   ├── intensity
│   └── extremumType
├── FringePolyline
│   ├── points (vector<CDPoint>)
│   └── index
└── NumberedFringe
    ├── points (vector<CDPoint>)
    ├── number (double)
    └── segmentIndex

Adapter Output (Stage 4)
└── CFringeSegment (MFC-compatible, contains CObject inheritance)
    ├── number
    ├── index
    └── points (vector<CDPoint>)

Legacy MFC Storage (CDigitInfo)
├── Fringes (vector<CFringeSegment>)
├── HidenDots (vector<CDPoint>)
├── MainFringeNumber (double)
└── Dots (vector<CDotInfo> - legacy, deprecated)
```

---

## Key Architectural Boundaries

```
┌────────────────────────────────────────────────────────────────┐
│                   PURE ALGORITHM BOUNDARY                      │
│                                                                │
│  • NO MFC includes                                             │
│  • NO CObject, CArray, CDC, CPoint usage                      │
│  • ONLY: std::vector, double, int, CDPoint (geometry)         │
│  • Fully testable with synthetic data and mocks               │
│  • Replaceable with ML, adaptive, or parallel variants        │
│                                                                │
│  Stages 1-3: RedCenterDetector, FringeConnector, FringeNumber │
└────────────────────────────────────────────────────────────────┘
                            ↕
                     [FringeSegmentAdapter]
                            ↕
┌────────────────────────────────────────────────────────────────┐
│                   MFC STORAGE BOUNDARY                         │
│                                                                │
│  • CFringeSegment (CObject-derived)                           │
│  • CDigitInfo members (CArray, etc.)                          │
│  • Integration with MFC rendering and serialization           │
│  • Legacy compatibility                                        │
│                                                                │
│  Only adapter touches CFringeSegment                          │
└────────────────────────────────────────────────────────────────┘
```

---

## Red Dots Visualization

```
Algorithm Detection Level → User Visualization
─────────────────────────────────────────────

Stage 1 Output
    ↓
ExtremumPoint (detected extrema)
    {position: (x, y), intensity: 255, scanLineIndex: 42, ...}
    ↓
DigitizationOutput::redCenters
    ↓
Stage 4 Adapter + CDigitInfo::ConvertOutputToLegacyModel()
    ↓
CDigitInfo::HidenDots
    ↓
[Visualization Layer]
    ├─ Draw red dots at each extrema position
    ├─ Show detection results before connection
    └─ Aid debugging: "Did algorithm find the right peaks?"
```

---

## Strategy Extension Points

The architecture supports future algorithm variants without modifying the bridge:

```
Current: StandardDigitizer
    └─ Stage 1-3 as designed

Future: AdaptiveDigitizer
    └─ Stage 1: RedCenterDetector (maybe auto-tuned)
    └─ Stage 2: FringeConnector (same interface)
    └─ Stage 3: FringeNumberer (same interface)
    └─ Output: DigitizationOutput (same contract)
    └─ Bridge: FringeSegmentAdapter (unchanged!)

Future: MLBasedDigitizer
    └─ Completely different Stages 1-3
    └─ Input: DigitizationInput (same contract)
    └─ Output: DigitizationOutput (same contract)
    └─ Bridge: FringeSegmentAdapter (unchanged!)
```

---

## No ZAPSections

```
REMOVED: ZAPLine concept
─────────────────────────

OLD Architecture:
  Stage 4: ZAPSectionBuilder
    └─ Generated reference horizontal lines
    └─ Stored in CDigitInfo::ZapLines
    └─ Used for: fringe numbering reference (now obsolete)

NEW Architecture:
  Stage 4: FringeSegmentAdapter
    └─ Converts pure output to MFC format
    └─ NO ZAPLines generated
    └─ Fringe numbering happens in pure Stage 3
    └─ No need for reference lines in future
```

---

## Testing Strategy

```
Unit Tests (Each Stage Independently)
├── RedCenterDetectorTest
│   └─ Test with synthetic image + mock mask
├── FringeConnectorTest
│   └─ Test with synthetic extrema + mock mask
├── FringeNumbererTest
│   └─ Test with synthetic polylines
└── FringeSegmentAdapterTest
    └─ Test pure → MFC conversion

Integration Tests
├── StandardDigitizerTest
│   └─ Complete pipeline with synthetic data
└── DigitizationIntegrationTest
    └─ End-to-end with real interferogram

Regression Tests
└─ CDigitInfo::Auto() output matches pre-refactoring
```

---

## Summary Table

| Aspect | Old | New | Benefit |
|--------|-----|-----|---------|
| **Mask system** | buf_line (2D array) | VisibilityMaskAccessor | Query-based, no pre-allocation |
| **Algorithm structure** | Monolithic CDigitInfo methods | Pure StandardDigitizer + 3 stages | Testable, replaceable |
| **MFC boundary** | Mixed throughout | Stage 4 adapter | Pure algorithm isolated |
| **Red dots** | Hidden in legacy code | Explicit in output | Transparent visualization |
| **ZAP sections** | Generated in Stage 4 | Removed | Simplified, deprecated concept |
| **Output model** | CDotInfo (flat array) | NumberedFringe (STL) | Pure, structured |
| **Final storage** | Dots array | CFringeSegment vector | Segment-primary model |
| **Future variants** | Hard to replace | Easy to add | Strategy pattern ready |

