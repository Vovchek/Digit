/// <summary>
/// Google Test suite for CDigitInfo::CreateNumLines() fringe numbering algorithm
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/DigitInfo.h"
#include "DigitMode/SectionInfo.h"

// ============================================================================
// MOCK IMPLEMENTATIONS - Minimal stubs to satisfy linker
// These avoid pulling in full production code dependencies
// ============================================================================

CZapLineInfo::CZapLineInfo(): Number(0), Removed(1), iSec(-1) {
    // Empty stub
}

CZapLineInfo::~CZapLineInfo() {
    // Empty stub
}

// CDotInfo mock implementation
CDotInfo::CDotInfo()
{
    Init();
}

CDotInfo::~CDotInfo()
{
}

void CDotInfo::Init()
{
    iZapSec = -1;
    Number = -1000.;
    P.x = P.y = 0.;
}

CDotInfo& CDotInfo::operator=(const CDotInfo& rhs)
{
    if (this == &rhs) return *this;
    iZapSec = rhs.iZapSec;
    Number = rhs.Number;
    P = rhs.P;
    return *this;
}

void CDotInfo::Draw(CDC* pDC, int dotSide, BOOL mainDot/*FALSE*/)
{
    // Empty stub
}

// CNumLine mock implementation
CNumLine::CNumLine() : Number(-1000.0), Included(FALSE), redX(0.0) {
    segmL.Init();  // CRITICAL: Initialize the segment!
}

CNumLine::~CNumLine() {
    // Empty stub
}

void CNumLine::Init() {
    Number = -1000.0;
    Included = FALSE;
    redX = 0.0;
}

CNumLine& CNumLine::operator=(const CNumLine& rhs) {
    if (this != &rhs) {
        Number = rhs.Number;
        segmL = rhs.segmL;  // Make sure this copies properly
        Included = rhs.Included;
        redX = rhs.redX;
    }
    return *this;
}

// CSectionInfo mock implementation
CSectionInfo::CSectionInfo() : aveStep(0.0), Color(RGB(255, 0, 0)), 
                                MainLine(FALSE), VisibleRedDots(TRUE) {
    // Empty stub
}

CSectionInfo::~CSectionInfo() {
    // Empty stub
}

void CSectionInfo::Init() {
    NumLines.RemoveAll();
    aveStep = 0.0;
    Color = RGB(255, 0, 0);
    MainLine = FALSE;
    VisibleRedDots = TRUE;
}

CSectionInfo& CSectionInfo::operator=(const CSectionInfo& rhs) {
    if (this != &rhs) {
        L = rhs.L;
        NumLines.Copy(rhs.NumLines);
        aveStep = rhs.aveStep;
        Color = rhs.Color;
        MainLine = rhs.MainLine;
        VisibleRedDots = rhs.VisibleRedDots;
    }
    return *this;
}

void CSectionInfo::Draw(CDC* pDC, int MainFringeNumber) {
    // Not needed for CreateNumLines tests
}

void CSectionInfo::Sort() {
    // Not needed for CreateNumLines tests
}

bool CSectionInfo::Form(int iy, unsigned char* line, int nx, int ny, int **buf_line) {
    return false; // Not needed for CreateNumLines tests
}

void CSectionInfo::CalcAveStep() {
    // Not needed for CreateNumLines tests
}

// CDigitInfo mock implementation
CDigitInfo::CDigitInfo() : buf_line(nullptr), ny_buf_line(0), 
                           HandSetZapLines(FALSE), idxDragZapLine(-1), 
                           idxDragDot(-1), idxMainSection(0), idxMainDot(-1),
                           MainFringeNumber(0.0), CurrentNumber(0.0), 
                           SecSegm(0.0), CorrectionSecSegm(0.0), numStep(1.0),
                           isInsideScreen(FALSE), ScaleFactor(1.0), Rotation(0.0) {
    // Empty stub
}

CDigitInfo::~CDigitInfo() {
    Delete_buf_line();
}

void CDigitInfo::Init() {
    Sections.RemoveAll();
    ZapLines.RemoveAll();
    Dots.RemoveAll();
    HidenDots.RemoveAll();
    
    HandSetZapLines = FALSE;
    idxDragZapLine = -1;
    idxDragDot = -1;
    idxMainSection = 0;
    idxMainDot = -1;
    MainFringeNumber = 0.0;
    CurrentNumber = 0.0;
    SecSegm = 0.0;
    CorrectionSecSegm = 0.0;
    numStep = 1.0;
    isInsideScreen = FALSE;
    ScaleFactor = 1.0;
    Rotation = 0.0;
    Comments.Empty();
    
    Delete_buf_line();
}

void CDigitInfo::Init_buf_line(int ny, int n) {
    // Not needed for CreateNumLines tests
}

void CDigitInfo::Delete_buf_line() {
    if (buf_line) {
        for (int i = 0; i < ny_buf_line; i++) {
            delete[] buf_line[i];
        }
        delete[] buf_line;
        buf_line = nullptr;
    }
    ny_buf_line = 0;
}

// Add ONLY the CreateNumLines() implementation from production code
// Copy the actual implementation here or link to DigitInfo.cpp

// ============================================================================
// END MOCK IMPLEMENTATIONS
// ============================================================================

// ============================================================================
// Production Code to be tested Inclusion
#include "DigitMode\SelectNumber.cxx"
#include "DigitMode\CreateNumLines.cxx"
// ============================================================================

/// <summary>
/// Test fixture for CreateNumLines() with mock data setup
/// </summary>
class CreateNumLinesTest : public ::testing::Test {
protected:
    CDigitInfo digitInfo;
    
    void SetUp() override {
        digitInfo.Init();
        digitInfo.numStep = 1.0;
        digitInfo.CorrectionSecSegm = 0.3;
    }

    /// <summary>
    /// Creates a simple test scenario with uniform fringe spacing
    /// </summary>
    void SetupUniformSections(int numSections, int fringesPerSection, double spacing) {
        digitInfo.Sections.SetSize(numSections);
        digitInfo.SecSegm = spacing;
        digitInfo.idxMainSection = numSections / 2;

        for (int i = 0; i < numSections; i++) {
            digitInfo.Sections[i].NumLines.SetSize(fringesPerSection);
            digitInfo.Sections[i].aveStep = spacing;
            
            for (int j = 0; j < fringesPerSection; j++) {
                double x = 100.0 + j * spacing;
                digitInfo.Sections[i].NumLines[j].redX = x;
                digitInfo.Sections[i].NumLines[j].Included = FALSE;
                
                // Initialize segmL so SelectNumber() can use it
                double tolerance = spacing * digitInfo.CorrectionSecSegm;
                digitInfo.Sections[i].NumLines[j].segmL.P1.x = x - tolerance;
                digitInfo.Sections[i].NumLines[j].segmL.P2.x = x + tolerance;
            }
        }
    }

    /// <summary>
    /// Creates test scenario that reproduces the debugger bug:
    /// Section with more fringes than refNumLines capacity
    /// </summary>
    void SetupBugScenario() {
        // Reproduce actual crash: Section 38 has 48+ fringes, refNumLines=47
        digitInfo.Sections.SetSize(50);
        digitInfo.SecSegm = 11.625;
        digitInfo.CorrectionSecSegm = 0.3;
        digitInfo.idxMainSection = 39;

        // Main section: 28 fringes
        digitInfo.Sections[39].NumLines.SetSize(28);
        digitInfo.Sections[39].aveStep = 11.625;
        for (int i = 0; i < 28; i++) {
            digitInfo.Sections[39].NumLines[i].redX = 100.0 + i * 11.625;
        }

        // Section 38: 48 fringes (exceeds calculated maxSize)
        digitInfo.Sections[38].NumLines.SetSize(48);
        digitInfo.Sections[38].aveStep = 11.625;
        for (int i = 0; i < 48; i++) {
            digitInfo.Sections[38].NumLines[i].redX = 103.5 + i * 11.625;
        }

        // Section 37: 30 fringes
        digitInfo.Sections[37].NumLines.SetSize(30);
        digitInfo.Sections[37].aveStep = 11.625;
        for (int i = 0; i < 30; i++) {
            digitInfo.Sections[37].NumLines[i].redX = 105.0 + i * 11.625;
        }
    }

    /// <summary>
    /// Creates edge case: fringes appearing/disappearing at boundaries
    /// </summary>
    void SetupDynamicFringeScenario() {
        digitInfo.Sections.SetSize(5);
        digitInfo.SecSegm = 10.0;
        digitInfo.idxMainSection = 2;

        // Center section: 10 fringes
        digitInfo.Sections[2].NumLines.SetSize(10);
        digitInfo.Sections[2].aveStep = 10.0;
        for (int i = 0; i < 10; i++) {
            digitInfo.Sections[2].NumLines[i].redX = 100.0 + i * 10.0;
        }

        // Upper section: 8 fringes (2 disappear)
        digitInfo.Sections[1].NumLines.SetSize(8);
        digitInfo.Sections[1].aveStep = 10.0;
        for (int i = 0; i < 8; i++) {
            digitInfo.Sections[1].NumLines[i].redX = 110.0 + i * 10.0;
        }

        // Lower section: 12 fringes (2 new at edges)
        digitInfo.Sections[3].NumLines.SetSize(12);
        digitInfo.Sections[3].aveStep = 10.0;
        for (int i = 0; i < 12; i++) {
            digitInfo.Sections[3].NumLines[i].redX = 90.0 + i * 10.0;
        }
    }
};

/// <summary>
/// Test: Basic sequential numbering of main section
/// </summary>
TEST_F(CreateNumLinesTest, MainSectionSequentialNumbering) {
    SetupUniformSections(5, 10, 10.0);
    
    digitInfo.CreateNumLines();

    // Verify main section numbered 0, 1, 2, ..., 9
    ASSERT_EQ(digitInfo.Sections[2].NumLines.GetSize(), 10);
    for (int i = 0; i < 10; i++) {
        EXPECT_DOUBLE_EQ(digitInfo.Sections[2].NumLines[i].Number, static_cast<double>(i));
        EXPECT_TRUE(digitInfo.Sections[2].NumLines[i].Included);
    }
}

/// <summary>
/// Test: Upward propagation maintains numbering continuity
/// </summary>
TEST_F(CreateNumLinesTest, UpwardPropagation) {
    SetupUniformSections(5, 10, 10.0);
    
    digitInfo.CreateNumLines();

    // Note: CreateNumLines uses sparse arrays, so size may be > original
    // We need to verify numbered fringes match, not array size
    
    // Collect numbered fringes from both sections
    std::vector<double> section1Numbers;
    std::vector<double> section2Numbers;
    
    for (int i = 0; i < digitInfo.Sections[1].NumLines.GetSize(); i++) {
        if (digitInfo.Sections[1].NumLines[i].Included) {
            section1Numbers.push_back(digitInfo.Sections[1].NumLines[i].Number);
        }
    }
    
    for (int i = 0; i < digitInfo.Sections[2].NumLines.GetSize(); i++) {
        if (digitInfo.Sections[2].NumLines[i].Included) {
            section2Numbers.push_back(digitInfo.Sections[2].NumLines[i].Number);
        }
    }
    
    // Verify same count of numbered fringes
    ASSERT_EQ(section1Numbers.size(), 10) << "Section 1 should have 10 numbered fringes";
    ASSERT_EQ(section2Numbers.size(), 10) << "Section 2 should have 10 numbered fringes";
    
    // Verify numbers match
    for (size_t i = 0; i < section1Numbers.size(); i++) {
        EXPECT_DOUBLE_EQ(section1Numbers[i], section2Numbers[i]) 
            << "Fringe " << i << " numbers should match";
    }
}

/// <summary>
/// Test: Downward propagation maintains numbering continuity
/// </summary>
TEST_F(CreateNumLinesTest, DownwardPropagation) {
    SetupUniformSections(5, 10, 10.0);
    
    digitInfo.CreateNumLines();

    // Note: CreateNumLines uses sparse arrays, so size may be > original
    // We need to verify numbered fringes match, not array size
    
    // Collect numbered fringes from both sections
    std::vector<double> section3Numbers;
    std::vector<double> section2Numbers;
    
    for (int i = 0; i < digitInfo.Sections[3].NumLines.GetSize(); i++) {
        if (digitInfo.Sections[3].NumLines[i].Included) {
            section3Numbers.push_back(digitInfo.Sections[3].NumLines[i].Number);
        }
    }
    
    for (int i = 0; i < digitInfo.Sections[2].NumLines.GetSize(); i++) {
        if (digitInfo.Sections[2].NumLines[i].Included) {
            section2Numbers.push_back(digitInfo.Sections[2].NumLines[i].Number);
        }
    }
    
    // Verify same count of numbered fringes
    ASSERT_EQ(section3Numbers.size(), 10) << "Section 3 should have 10 numbered fringes";
    ASSERT_EQ(section2Numbers.size(), 10) << "Section 2 should have 10 numbered fringes";
    
    // Verify numbers match
    for (size_t i = 0; i < section3Numbers.size(); i++) {
        EXPECT_DOUBLE_EQ(section3Numbers[i], section2Numbers[i]) 
            << "Fringe " << i << " numbers should match";
    }
}

/// <summary>
/// Test: Bug scenario - idxL exceeds refNumLines capacity
/// This test reproduces the ASSERT failure from debugging session
/// </summary>
TEST_F(CreateNumLinesTest, BugScenario_IndexOutOfBounds) {
    SetupBugScenario();
    
    // This should NOT crash after fix
    ASSERT_NO_THROW(digitInfo.CreateNumLines());
    
    // Verify all sections have valid numbering
    for (int i = 0; i < digitInfo.Sections.GetSize(); i++) {
        for (int j = 0; j < digitInfo.Sections[i].NumLines.GetSize(); j++) {
            EXPECT_TRUE(digitInfo.Sections[i].NumLines[j].Included);
            EXPECT_NE(digitInfo.Sections[i].NumLines[j].Number, -1000.0);
        }
    }
}

/// <summary>
/// Test: New fringes at left edge get decreasing numbers
/// </summary>
TEST_F(CreateNumLinesTest, NewFringesAtLeftEdge) {
    SetupDynamicFringeScenario();
    
    digitInfo.CreateNumLines();

    // Section 3 should have fringes with negative numbers at left edge
    ASSERT_GT(digitInfo.Sections[3].NumLines.GetSize(), 10);
    EXPECT_LT(digitInfo.Sections[3].NumLines[0].Number, 0.0);
}

/// <summary>
/// Test: New fringes at right edge get increasing numbers
/// </summary>
TEST_F(CreateNumLinesTest, NewFringesAtRightEdge) {
    SetupDynamicFringeScenario();
    
    digitInfo.CreateNumLines();

    // Section 3 should have fringes with numbers > 9 at right edge
    int lastIdx = digitInfo.Sections[3].NumLines.GetSize() - 1;
    EXPECT_GT(digitInfo.Sections[3].NumLines[lastIdx].Number, 9.0);
}

/// <summary>
/// Test: Orphan fringes (between range but no match) are handled
/// </summary>
TEST_F(CreateNumLinesTest, OrphanFringeHandling) {
    // Setup scenario with fringe that appears mid-range
    SetupUniformSections(3, 10, 10.0);
    
    // Add orphan fringe in section 1
    digitInfo.Sections[1].NumLines.SetSize(11);
    for (int i = 0; i < 10; i++) {
        digitInfo.Sections[1].NumLines[i].redX = 100.0 + i * 10.0;
    }
    digitInfo.Sections[1].NumLines[10].redX = 145.0; // Between fringes 4 and 5
    
    digitInfo.CreateNumLines();

    // TODO: Verify orphan is handled (currently marked with rr=0)
    // Current implementation doesn't assign number to orphans
}

/// <summary>
/// Test: Section-specific step sizes are respected
/// </summary>
TEST_F(CreateNumLinesTest, SectionSpecificStepSize) {
    SetupUniformSections(3, 10, 10.0);
    
    // Override section 1 with different step
    digitInfo.Sections[1].aveStep = 12.0;
    
    digitInfo.CreateNumLines();

    // Verify tolerance calculation uses section-specific step
    // (Indirect test - verify no crashes and reasonable results)
    EXPECT_EQ(digitInfo.Sections[1].NumLines.GetSize(), 10);
}