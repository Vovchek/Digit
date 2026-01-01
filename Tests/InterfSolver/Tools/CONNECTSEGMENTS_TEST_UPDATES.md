# ConnectSegments Test Updates for Degenerate Polygon Filtering

## Summary

`ConnectSegments` now filters out degenerate polygons using `Plg.isDegenerate()` check. This affects several tests that expect degenerate results (like single-point or two-point "polygons").

## Tests That Need Updates

### 1. **Degenerate Input Tests** - Should produce NO output

These tests currently expect output but should be updated to expect NO contours since the inputs are degenerate:

```cpp
TEST_F(ConnectSegmentsTest, SinglePointSegment_HandlesGracefully) {
    // OLD: EXPECT_EQ(arrCont.GetSize(), 1);
    // NEW: Single point is degenerate ? should be filtered
    EXPECT_EQ(arrCont.GetSize(), 0) 
        << "Single-point segment should be filtered as degenerate";
}

TEST_F(ConnectSegmentsTest, TwoPointSegment_CreatesPolygon) {
    // OLD: ASSERT_EQ(arrCont.GetSize(), 1);
    // NEW: Two points (line) is degenerate ? should be filtered
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Two-point segment (line) should be filtered as degenerate";
}
```

### 2. **Disconnected Segment Tests** - May produce fewer contours

These tests expect multiple contours from disconnected segments, but if individual segments are degenerate (2 points each), they'll be filtered:

```cpp
TEST_F(ConnectSegmentsTest, TwoDisconnectedSegments_ProducesTwoPolygons) {
    // Each segment is 2 points ? degenerate
    // OLD: EXPECT_EQ(arrCont.GetSize(), 2);
    // NEW: Both segments filtered as degenerate
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Two-point segments should be filtered as degenerate";
}

TEST_F(ConnectSegmentsTest, EndpointsBeyondTolerance_AreNotConnected) {
    // Two 2-point segments that don't connect
    // OLD: EXPECT_EQ(arrCont.GetSize(), 2);
    // NEW: Both filtered as degenerate
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Disconnected 2-point segments should be filtered as degenerate";
}

TEST_F(ConnectSegmentsTest, ManyDisconnectedSegments_ProducesManyContours) {
    // 50 separate 2-point segments
    // OLD: EXPECT_EQ(arrCont.GetSize(), 50);
    // NEW: All filtered as degenerate
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Disconnected 2-point segments should all be filtered as degenerate";
}
```

### 3. **Connected Segment Tests** - May still produce degenerate results

Tests where segments connect but result in collinear points (zero area):

```cpp
TEST_F(ConnectSegmentsTest, ManySmallSegments_FormSingleContour) {
    // 10 segments connecting along a straight line
    // Result: ~11 collinear points ? ZERO AREA ? degenerate
    
    // OLD:
    // EXPECT_EQ(arrCont.GetSize(), 1);
    // EXPECT_GE(arrCont[0].GetSize(), 11);
    
    // NEW:
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Collinear segments form zero-area polygon ? should be filtered as degenerate";
}

TEST_F(ConnectSegmentsTest, ManySegments_HandlesEfficiently) {
    // 100 sequential segments along a line
    // Result: collinear points ? ZERO AREA ? degenerate
    
    // OLD: EXPECT_EQ(arrCont.GetSize(), 1);
    // NEW:
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "100 collinear segments form zero-area polygon ? should be filtered";
}
```

### 4. **Single Segment Tests** - Depends on segment complexity

```cpp
TEST_F(ConnectSegmentsTest, SingleSegment_ProducesOnePolygon) {
    // 3 points: (0,0), (1,0), (1,1)
    // If non-collinear AND has area ? valid
    // Current test should PASS if segment has area
    
    // Keep as-is if 3 points form triangle
    // Update if points are collinear
}
```

## Recommended Test Strategy

### Option A: **Update Existing Tests** (Document current behavior)

Update tests to reflect the NEW behavior with degenerate filtering:

```cpp
// Tests that now expect ZERO contours due to filtering:
- SinglePointSegment_HandlesGracefully ? expects 0
- TwoPointSegment_CreatesPolygon ? expects 0
- TwoDisconnectedSegments_ProducesTwoPolygons ? expects 0
- EndpointsBeyondTolerance_AreNotConnected ? expects 0
- ManySmallSegments_FormSingleContour ? expects 0
- ManySegments_HandlesEfficiently ? expects 0
- ManyDisconnectedSegments_ProducesManyContours ? expects 0
```

### Option B: **Add New Degenerate-Specific Tests** (Better approach)

Keep existing tests for valid polygons, ADD new tests specifically for degenerate filtering:

```cpp
// ============================================================================
// Degenerate Polygon Filtering Tests
// ============================================================================

TEST_F(ConnectSegmentsTest, Degenerate_SinglePoint_IsFiltered) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.0, 0.0) }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Single-point segment should be filtered as degenerate";
}

TEST_F(ConnectSegmentsTest, Degenerate_TwoPoints_IsFiltered) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Two-point line should be filtered as degenerate";
}

TEST_F(ConnectSegmentsTest, Degenerate_CollinearPoints_IsFiltered) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Many segments forming a straight line (zero area)
    for (int i = 0; i < 10; i++) {
        arrBLn.Add(CreateBrokenLine({
            XYPoint(i * 1.0, 0.0),
            XYPoint((i + 1) * 1.0, 0.0)
        }));
    }
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Collinear segments form zero-area polygon ? should be filtered";
}

TEST_F(ConnectSegmentsTest, Degenerate_ZeroAreaTriangle_IsFiltered) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Triangle with collinear points (zero area)
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.0, 0.0), XYPoint(1.0, 0.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(1.0, 0.0), XYPoint(2.0, 0.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(2.0, 0.0), XYPoint(0.0, 0.0) }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Triangle with collinear points (zero area) should be filtered";
}

TEST_F(ConnectSegmentsTest, ValidTriangle_NotFiltered) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Valid triangle with NON-collinear points (has area)
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.0, 0.0), XYPoint(1.0, 0.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(1.0, 0.0), XYPoint(0.5, 1.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.5, 1.0), XYPoint(0.0, 0.0) }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 1)
        << "Valid triangle with area should NOT be filtered";
    EXPECT_FALSE(arrCont[0].isDegenerate());
}
```

## Tests That Should Still PASS (Non-Degenerate Results)

These tests produce valid polygons with area > 0 and should continue to pass:

? **ThreeSegments_FormClosedTriangle** - Valid triangle with area  
? **FourSegments_FormClosedRectangle** - Valid rectangle with area  
? **SegmentsInRandomOrder_StillConnect** - Valid rectangle  
? **MultipleContours_TwoSeparateTriangles** - Two valid triangles  
? **BrokenCircle_ReconnectsSegments** - Valid circular contour  
? **Star_FormsSingleContour** - Valid star shape  
? **Hexagon_SixSegments_FormsClosed** - Valid hexagon  
? **Tolerance tests** - If they produce triangles/rectangles with area  

## Tests Requiring Code Changes

### Update with proper non-collinear segments:

```cpp
TEST_F(ConnectSegmentsTest, ManySmallSegments_FormSingleContour) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.1;
    
    // OLD: Create segments along a LINE (zero area)
    // NEW: Create segments forming a SPIRAL or PATH with area
    
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.0, 0.0), XYPoint(1.0, 0.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(1.0, 0.0), XYPoint(1.0, 1.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(1.0, 1.0), XYPoint(0.0, 1.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.0, 1.0), XYPoint(0.0, 0.0) }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    EXPECT_EQ(arrCont.GetSize(), 1) << "Rectangle segments should form valid polygon";
    EXPECT_GE(arrCont[0].GetSize(), 4);
    EXPECT_FALSE(arrCont[0].isDegenerate());
}
```

## Summary of Changes Needed

| Test Name | Old Expectation | New Expectation | Reason |
|-----------|----------------|-----------------|--------|
| SinglePointSegment_HandlesGracefully | 1 contour | 0 contours | Single point is degenerate |
| TwoPointSegment_CreatesPolygon | 1 contour | 0 contours | Line (2 points) is degenerate |
| TwoDisconnectedSegments_ProducesTwoPolygons | 2 contours | 0 contours | Each segment is degenerate |
| EndpointsBeyondTolerance_AreNotConnected | 2 contours | 0 contours | Each segment is degenerate |
| ManySmallSegments_FormSingleContour | 1 contour | 0 contours OR redesign test | Collinear points ? zero area |
| ManySegments_HandlesEfficiently | 1 contour | 0 contours OR redesign test | Collinear points ? zero area |
| ManyDisconnectedSegments_ProducesManyContours | 50 contours | 0 contours | Each segment is degenerate |

## Implementation Priority

1. **HIGH**: Fix `ManySmallSegments_FormSingleContour` (currently failing assertion)
2. **HIGH**: Fix degenerate input tests (SinglePoint, TwoPoint)
3. **MEDIUM**: Add dedicated degenerate filtering tests
4. **LOW**: Update documentation/comments for affected tests

