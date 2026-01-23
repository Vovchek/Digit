#include "gtest/gtest.h"
#include "../DigitMode/DigitInfo.h"
#include "../InterfSolver/Tools/ReadWriteData.h"
#include "../MGTools/Include/Utils/BaseDataType.h"
#include <cmath>

// Mock implementations of required global functions for testing
// These would normally come from the application framework

static CDocument* g_pMockDoc = nullptr;
static CImageCtrls* g_pMockImageCtrls = nullptr;
static CBoundCtrls* g_pMockBoundCtrls = nullptr;
static CControls* g_pMockControls = nullptr;

CDocument* GetWIActiveDocument() { return g_pMockDoc; }
CImageCtrls* GetImageCtrls() { return g_pMockImageCtrls; }
CBoundCtrls* GetBoundCtrls() { return g_pMockBoundCtrls; }
CControls* GetControls() { return g_pMockControls; }

// ===== Test Fixture =====

class CFringeFileIOTest : public ::testing::Test {
protected:
    CDigitInfo digitInfo;
    CImageCtrls imageCtrls;
    CBoundCtrls boundCtrls;
    CControls controls;
    
    void SetUp() override {
        g_pMockImageCtrls = &imageCtrls;
        g_pMockBoundCtrls = &boundCtrls;
        g_pMockControls = &controls;
        
        // Initialize mock controls
        imageCtrls.ImageSize.cx = 512;
        imageCtrls.ImageSize.cy = 512;
        imageCtrls.ImageFileName = _T("test.bmp");
        
        digitInfo.Init();
    }
    
    void TearDown() override {
        digitInfo.Clear();
        g_pMockImageCtrls = nullptr;
        g_pMockBoundCtrls = nullptr;
        g_pMockControls = nullptr;
    }
    
    // Helper: Create test data with known structure
    void CreateTestFringes() {
        digitInfo.m_bUseFringeModel = TRUE;
        digitInfo.Fringes.RemoveAll();
        
        // Fringe 0.0: horizontal line at y=100
        int iF0 = digitInfo.CreateFringe(0.0);
        digitInfo.AddPointToFringe(iF0, CDPoint(10, 100));
        digitInfo.AddPointToFringe(iF0, CDPoint(20, 100));
        digitInfo.AddPointToFringe(iF0, CDPoint(30, 100));
        
        // Fringe 1.0: horizontal line at y=200
        int iF1 = digitInfo.CreateFringe(1.0);
        digitInfo.AddPointToFringe(iF1, CDPoint(15, 200));
        digitInfo.AddPointToFringe(iF1, CDPoint(25, 200));
        
        // Fringe 2.0: horizontal line at y=300
        int iF2 = digitInfo.CreateFringe(2.0);
        digitInfo.AddPointToFringe(iF2, CDPoint(12, 300));
        digitInfo.AddPointToFringe(iF2, CDPoint(22, 300));
        digitInfo.AddPointToFringe(iF2, CDPoint(32, 300));
        digitInfo.AddPointToFringe(iF2, CDPoint(42, 300));
    }
};

// ===== Collection Tests (Saving) =====

TEST_F(CFringeFileIOTest, CollectFromFringesPopulatesIntInfo) {
    CreateTestFringes();
    
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    BOOL result = digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(9, intInfo.DigitDat.GetSize()); // 3 + 2 + 4 points
}

TEST_F(CFringeFileIOTest, CollectPreservesAllPoints) {
    CreateTestFringes();
    
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    // Verify all X coordinates present
    EXPECT_TRUE(intInfo.DigitDat.XPnt.GetSize() == 9);
    
    // Check some specific values
    bool found10 = false, found20 = false, found30 = false;
    for (int i = 0; i < intInfo.DigitDat.XPnt.GetSize(); i++) {
        if (fabs(intInfo.DigitDat.XPnt[i] - 10.0) < 0.01) found10 = true;
        if (fabs(intInfo.DigitDat.XPnt[i] - 20.0) < 0.01) found20 = true;
        if (fabs(intInfo.DigitDat.XPnt[i] - 30.0) < 0.01) found30 = true;
    }
    
    EXPECT_TRUE(found10);
    EXPECT_TRUE(found20);
    EXPECT_TRUE(found30);
}

TEST_F(CFringeFileIOTest, CollectPreservesFringeNumbers) {
    CreateTestFringes();
    
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    // Count points per fringe number
    int count0 = 0, count1 = 0, count2 = 0;
    for (int i = 0; i < intInfo.DigitDat.FPnt.GetSize(); i++) {
        if (fabs(intInfo.DigitDat.FPnt[i] - 0.0) < 0.01) count0++;
        if (fabs(intInfo.DigitDat.FPnt[i] - 1.0) < 0.01) count1++;
        if (fabs(intInfo.DigitDat.FPnt[i] - 2.0) < 0.01) count2++;
    }
    
    EXPECT_EQ(3, count0);
    EXPECT_EQ(2, count1);
    EXPECT_EQ(4, count2);
}

TEST_F(CFringeFileIOTest, CollectEmptyFringesReturnsFalse) {
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.Fringes.RemoveAll();
    
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    BOOL result = digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    EXPECT_FALSE(result);
}

TEST_F(CFringeFileIOTest, CollectFromDotsWorksAsOld) {
    digitInfo.m_bUseFringeModel = FALSE;
    
    CDotInfo dot1, dot2;
    dot1.P = CDPoint(10, 100);
    dot1.Number = 0.5;
    dot2.P = CDPoint(20, 200);
    dot2.Number = 1.5;
    
    digitInfo.Dots.Add(dot1);
    digitInfo.Dots.Add(dot2);
    
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    BOOL result = digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(2, intInfo.DigitDat.GetSize());
}

// ===== Examination Tests (Loading) =====

TEST_F(CFringeFileIOTest, ExamineCreatesCorrectNumberOfFringes) {
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    
    // Create test data: 3 points at y=100 (fringe 0), 2 points at y=200 (fringe 1)
    intInfo.DigitDat.XPnt.Add(10); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.XPnt.Add(20); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.XPnt.Add(30); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.XPnt.Add(15); intInfo.DigitDat.YPnt.Add(200); intInfo.DigitDat.FPnt.Add(1.0);
    intInfo.DigitDat.XPnt.Add(25); intInfo.DigitDat.YPnt.Add(200); intInfo.DigitDat.FPnt.Add(1.0);
    intInfo.DigitDat.Properties.SetSize(5);
    
    digitInfo.m_bUseFringeModel = TRUE;
    BOOL result = digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(2, digitInfo.Fringes.GetSize());
}

TEST_F(CFringeFileIOTest, ExamineSortsPointsByX) {
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    
    // Add points in reverse X order
    intInfo.DigitDat.XPnt.Add(30); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.XPnt.Add(10); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.XPnt.Add(20); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.Properties.SetSize(3);
    
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    ASSERT_EQ(1, digitInfo.Fringes.GetSize());
    ASSERT_EQ(3, digitInfo.Fringes[0].GetPointCount());
    
    // Should be sorted by X
    EXPECT_EQ(10.0, digitInfo.Fringes[0].GetPoint(0).x);
    EXPECT_EQ(20.0, digitInfo.Fringes[0].GetPoint(1).x);
    EXPECT_EQ(30.0, digitInfo.Fringes[0].GetPoint(2).x);
}

TEST_F(CFringeFileIOTest, ExamineSeparatesFringesByY) {
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    
    // Same fringe number, different Y coordinates -> separate fringes
    intInfo.DigitDat.XPnt.Add(10); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.5);
    intInfo.DigitDat.XPnt.Add(20); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.5);
    intInfo.DigitDat.XPnt.Add(10); intInfo.DigitDat.YPnt.Add(200); intInfo.DigitDat.FPnt.Add(0.5);
    intInfo.DigitDat.XPnt.Add(20); intInfo.DigitDat.YPnt.Add(200); intInfo.DigitDat.FPnt.Add(0.5);
    intInfo.DigitDat.Properties.SetSize(4);
    
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    // Should create 2 separate fringe segments (different Y)
    EXPECT_EQ(2, digitInfo.Fringes.GetSize());
    
    EXPECT_EQ(0.5, digitInfo.Fringes[0].GetNumber());
    EXPECT_EQ(0.5, digitInfo.Fringes[1].GetNumber());
}

TEST_F(CFringeFileIOTest, ExamineSyncsToDots) {
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    
    intInfo.DigitDat.XPnt.Add(10); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.XPnt.Add(20); intInfo.DigitDat.YPnt.Add(100); intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.Properties.SetSize(2);
    
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    // Should also populate Dots for backward compatibility
    EXPECT_EQ(2, digitInfo.Dots.GetSize());
    EXPECT_EQ(10.0, digitInfo.Dots[0].P.x);
    EXPECT_EQ(0.0, digitInfo.Dots[0].Number);
}

TEST_F(CFringeFileIOTest, ExamineEmptyDataReturnsFalse) {
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    
    digitInfo.m_bUseFringeModel = TRUE;
    BOOL result = digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    EXPECT_FALSE(result);
}

// ===== Round-Trip Tests =====

TEST_F(CFringeFileIOTest, RoundTripPreservesPointCount) {
    CreateTestFringes();
    
    // Collect to IntInfo
    NUMBERING_INTERFEROGRAM_INFO intInfo1;
    digitInfo.CollectNumberingInterferogramInfo(intInfo1);
    int originalCount = intInfo1.DigitDat.GetSize();
    
    // Clear and re-examine
    digitInfo.Clear();
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo1);
    
    // Collect again
    NUMBERING_INTERFEROGRAM_INFO intInfo2;
    digitInfo.CollectNumberingInterferogramInfo(intInfo2);
    
    EXPECT_EQ(originalCount, intInfo2.DigitDat.GetSize());
}

TEST_F(CFringeFileIOTest, RoundTripPreservesCoordinates) {
    CreateTestFringes();
    
    // Collect
    NUMBERING_INTERFEROGRAM_INFO intInfo1;
    digitInfo.CollectNumberingInterferogramInfo(intInfo1);
    
    // Store original data
    CArrayDouble origX, origY, origF;
    origX.Copy(intInfo1.DigitDat.XPnt);
    origY.Copy(intInfo1.DigitDat.YPnt);
    origF.Copy(intInfo1.DigitDat.FPnt);
    
    // Clear and reload
    digitInfo.Clear();
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo1);
    
    // Collect again
    NUMBERING_INTERFEROGRAM_INFO intInfo2;
    digitInfo.CollectNumberingInterferogramInfo(intInfo2);
    
    // Compare (order might differ, so check all values exist)
    ASSERT_EQ(origX.GetSize(), intInfo2.DigitDat.XPnt.GetSize());
    
    for (int i = 0; i < origX.GetSize(); i++) {
        bool found = false;
        for (int j = 0; j < intInfo2.DigitDat.XPnt.GetSize(); j++) {
            if (fabs(origX[i] - intInfo2.DigitDat.XPnt[j]) < 0.01 &&
                fabs(origY[i] - intInfo2.DigitDat.YPnt[j]) < 0.01 &&
                fabs(origF[i] - intInfo2.DigitDat.FPnt[j]) < 0.01) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Point (" << origX[i] << "," << origY[i] << ") with F=" << origF[i] << " not found after round-trip";
    }
}

TEST_F(CFringeFileIOTest, RoundTripPreservesFringeStructure) {
    CreateTestFringes();
    int originalFringeCount = digitInfo.Fringes.GetSize();
    
    // Collect
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    // Clear and reload
    digitInfo.Clear();
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    EXPECT_EQ(originalFringeCount, digitInfo.Fringes.GetSize());
}

TEST_F(CFringeFileIOTest, RoundTripWithSinglePoint) {
    digitInfo.m_bUseFringeModel = TRUE;
    int iF = digitInfo.CreateFringe(0.0);
    digitInfo.AddPointToFringe(iF, CDPoint(100, 100));
    
    NUMBERING_INTERFEROGRAM_INFO intInfo1;
    digitInfo.CollectNumberingInterferogramInfo(intInfo1);
    
    digitInfo.Clear();
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo1);
    
    EXPECT_EQ(1, digitInfo.Fringes.GetSize());
    EXPECT_EQ(1, digitInfo.Fringes[0].GetPointCount());
    EXPECT_EQ(100.0, digitInfo.Fringes[0].GetPoint(0).x);
}

TEST_F(CFringeFileIOTest, RoundTripWithManyFringes) {
    digitInfo.m_bUseFringeModel = TRUE;
    
    // Create 10 fringes with varying point counts
    for (int f = 0; f < 10; f++) {
        int iF = digitInfo.CreateFringe(f * 0.5);
        for (int p = 0; p < f + 2; p++) {
            digitInfo.AddPointToFringe(iF, CDPoint(p * 10, f * 50));
        }
    }
    
    int originalFringeCount = digitInfo.Fringes.GetSize();
    
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    digitInfo.Clear();
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    EXPECT_EQ(originalFringeCount, digitInfo.Fringes.GetSize());
}

// ===== Dots Model Compatibility Tests =====

TEST_F(CFringeFileIOTest, DotsModelCollectStillWorks) {
    digitInfo.m_bUseFringeModel = FALSE;
    
    CDotInfo dot;
    dot.P = CDPoint(50, 150);
    dot.Number = 1.5;
    digitInfo.Dots.Add(dot);
    
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    BOOL result = digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(1, intInfo.DigitDat.GetSize());
}

TEST_F(CFringeFileIOTest, DotsModelExamineStillWorks) {
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    intInfo.DigitDat.XPnt.Add(25);
    intInfo.DigitDat.YPnt.Add(75);
    intInfo.DigitDat.FPnt.Add(2.5);
    intInfo.DigitDat.Properties.Add(0.0);
    
    digitInfo.m_bUseFringeModel = FALSE;
    BOOL result = digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(1, digitInfo.Dots.GetSize());
    EXPECT_EQ(25.0, digitInfo.Dots[0].P.x);
    EXPECT_EQ(2.5, digitInfo.Dots[0].Number);
}

// ===== Metadata Tests =====

TEST_F(CFringeFileIOTest, CollectPreservesMetadata) {
    digitInfo.Comments = _T("Test fringe data");
    digitInfo.ScaleFactor = 2.5;
    digitInfo.Rotation = 45.0;
    
    CreateTestFringes();
    
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    digitInfo.CollectNumberingInterferogramInfo(intInfo);
    
    EXPECT_EQ(CString(_T("Test fringe data")), intInfo.Title);
    EXPECT_EQ(2.5, intInfo.ScaleFactor);
    EXPECT_EQ(45.0, intInfo.FiScan);
}

TEST_F(CFringeFileIOTest, ExamineRestoresMetadata) {
    NUMBERING_INTERFEROGRAM_INFO intInfo;
    intInfo.Title = _T("Loaded data");
    intInfo.ScaleFactor = 1.25;
    intInfo.FiScan = 90.0;
    intInfo.DigitDat.XPnt.Add(10);
    intInfo.DigitDat.YPnt.Add(10);
    intInfo.DigitDat.FPnt.Add(0.0);
    intInfo.DigitDat.Properties.Add(0.0);
    
    digitInfo.m_bUseFringeModel = TRUE;
    digitInfo.ExamineNumberingInterferogramInfo(intInfo);
    
    CString comments;
    double scale, rotation;
    digitInfo.GetComments(comments);
    digitInfo.GetScaleFactor(scale);
    digitInfo.GetRotation(rotation);
    
    EXPECT_EQ(CString(_T("Loaded data")), comments);
    EXPECT_EQ(1.25, scale);
    EXPECT_EQ(90.0, rotation);
}
