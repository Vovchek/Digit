/**
 * @file ApertureCtrlsTest.cpp
 * @brief Unit tests for CApertureCtrls (modern aperture control)
 * 
 * Tests written from scratch using modern C++ and ApertureCore shapes.
 * No legacy XYShape or GetExtRealBound patterns.
 */
#include "stdafx.h"
#include "gtest/gtest.h"
#include "Controls\CApertureCtrls.h"
#include "ApertureCore\include\aperturecore\geometry\Ellipse.h"
#include "ApertureCore\include\aperturecore\geometry\Rectangle.h"
#include "ApertureCore\include\aperturecore\geometry\Polygon.h"
#include "ApertureCore\include\aperturecore\visibility\IDataProviders.h"

using namespace DigitMode;
using namespace aperture;

// ============================================================================
// Mock Image Data Provider for Testing
// ============================================================================

class MockImageData : public IImageData {
public:
    MockImageData(int width = 1024, int height = 768)
        : m_width(width)
        , m_height(height)
        , m_version(1)
    {}
    
    bool HasImage() const override { return true; }
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }
    const unsigned char* GetBitmapData() const override { return nullptr; }
    unsigned char GetPixel(int, int) const override { return 0; }
    uint64_t GetImageVersion() const override { return m_version; }
    
    void SetDimensions(int width, int height) {
        m_width = width;
        m_height = height;
        m_version++;
    }
    
private:
    int m_width;
    int m_height;
    uint64_t m_version;
};

// ============================================================================
// Test Fixture
// ============================================================================

class ApertureCtrlsTest : public ::testing::Test {
protected:
    ApertureCtrlsTest()
        : mockImageData(std::make_unique<MockImageData>())
        , apertureCtrls(nullptr)
    {}
    
    void SetUp() override {
        // Create CApertureCtrls with mock image data
        apertureCtrls = std::make_unique<CApertureCtrls>(*mockImageData);
        apertureCtrls->Init();
    }
    
    void TearDown() override {
        if (apertureCtrls) {
            apertureCtrls->Clear();
        }
        apertureCtrls.reset();
    }
    
    std::unique_ptr<MockImageData> mockImageData;
    std::unique_ptr<CApertureCtrls> apertureCtrls;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, InitialState_IsEmpty) {
    EXPECT_EQ(0u, apertureCtrls->GetShapeCount());
    EXPECT_EQ(0u, apertureCtrls->GetExternalCount());
    EXPECT_EQ(0u, apertureCtrls->GetInternalCount());
    EXPECT_EQ(0u, apertureCtrls->GetApertureCount());
}

TEST_F(ApertureCtrlsTest, Clear_RemovesAllShapes) {
    // Add some shapes
    auto ellipse = std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0);
    apertureCtrls->AddExternalShape(std::move(ellipse));
    
    auto rect = std::make_unique<aperture::Rectangle>(50.0, 50.0, 200.0, 200.0);
    apertureCtrls->AddInternalShape(std::move(rect));
    
    ASSERT_EQ(2u, apertureCtrls->GetShapeCount());
    
    // Clear
    apertureCtrls->Clear();
    
    EXPECT_EQ(0u, apertureCtrls->GetShapeCount());
}

// ============================================================================
// Shape Management Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, AddExternalShape_IncreasesCount) {
    auto ellipse = std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0);
    Shape* shape = apertureCtrls->AddExternalShape(std::move(ellipse));
    
    EXPECT_NE(nullptr, shape);
    EXPECT_EQ(1u, apertureCtrls->GetExternalCount());
    EXPECT_EQ(1u, apertureCtrls->GetShapeCount());
    EXPECT_EQ(TypeLimits::EXTERNAL, apertureCtrls->GetShapeType(shape));
}

TEST_F(ApertureCtrlsTest, AddInternalShape_IncreasesCount) {
    auto rect = std::make_unique<aperture::Rectangle>(20.0, 20.0, 100.0, 100.0);
    Shape* shape = apertureCtrls->AddInternalShape(std::move(rect));
    
    EXPECT_NE(nullptr, shape);
    EXPECT_EQ(1u, apertureCtrls->GetInternalCount());
    EXPECT_EQ(TypeLimits::INTERNAL, apertureCtrls->GetShapeType(shape));
}

TEST_F(ApertureCtrlsTest, AddApertureShape_IncreasesCount) {
    auto polygon = std::make_unique<aperture::Polygon>(std::vector<Point>{
        {0, 0}, {10, 0}, {10, 10}, {0, 10}
    });
    Shape* shape = apertureCtrls->AddApertureShape(std::move(polygon));
    
    EXPECT_NE(nullptr, shape);
    EXPECT_EQ(1u, apertureCtrls->GetApertureCount());
    EXPECT_EQ(TypeLimits::APERTURE, apertureCtrls->GetShapeType(shape));
}

TEST_F(ApertureCtrlsTest, AddMultipleShapes_TracksCountsCorrectly) {
    apertureCtrls->AddExternalShape(std::make_unique<aperture::Ellipse>(50, 50, 100, 100));
    apertureCtrls->AddExternalShape(std::make_unique<aperture::Rectangle>(50, 50, 200, 200));
    apertureCtrls->AddInternalShape(std::make_unique<aperture::Ellipse>(10, 10, 100, 100));
    
    EXPECT_EQ(2, apertureCtrls->GetExternalCount());
    EXPECT_EQ(1, apertureCtrls->GetInternalCount());
    EXPECT_EQ(0, apertureCtrls->GetApertureCount());
    EXPECT_EQ(3, apertureCtrls->GetShapeCount());
}

TEST_F(ApertureCtrlsTest, AddNullShape_ReturnsNull) {
    Shape* shape = apertureCtrls->AddExternalShape(nullptr);
    EXPECT_EQ(nullptr, shape);
}

// ============================================================================
// Shape Access Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, GetShapeType_ValidShape_ReturnsCorrectType) {
    Shape* extShape = apertureCtrls->AddExternalShape(std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0));
    Shape* intShape = apertureCtrls->AddInternalShape(std::make_unique<aperture::Ellipse>(30.0, 30.0, 50.0, 50.0));
    Shape* aptShape = apertureCtrls->AddApertureShape(std::make_unique<aperture::Rectangle>(10.0, 10.0, 100.0, 100.0));
    
    EXPECT_EQ(TypeLimits::EXTERNAL, apertureCtrls->GetShapeType(extShape));
    EXPECT_EQ(TypeLimits::INTERNAL, apertureCtrls->GetShapeType(intShape));
    EXPECT_EQ(TypeLimits::APERTURE, apertureCtrls->GetShapeType(aptShape));
}

TEST_F(ApertureCtrlsTest, GetShapeIndex_ValidShape_ReturnsCorrectIndex) {
    Shape* shape1 = apertureCtrls->AddExternalShape(std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0));
    Shape* shape2 = apertureCtrls->AddExternalShape(std::make_unique<aperture::Ellipse>(30.0, 30.0, 50.0, 50.0));
    
    EXPECT_EQ(0, apertureCtrls->GetShapeIndex(shape1));
    EXPECT_EQ(1, apertureCtrls->GetShapeIndex(shape2));
}

TEST_F(ApertureCtrlsTest, GetShapeIndex_NullShape_ReturnsMinusOne) {
    EXPECT_EQ(-1, apertureCtrls->GetShapeIndex(nullptr));
}

TEST_F(ApertureCtrlsTest, RemoveShape_ValidPointer_RemovesShape) {
    Shape* shape = apertureCtrls->AddExternalShape(std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0));
    ASSERT_EQ(1, apertureCtrls->GetExternalCount());
    
    bool removed = apertureCtrls->RemoveShape(shape);
    
    EXPECT_TRUE(removed);
    EXPECT_EQ(0, apertureCtrls->GetExternalCount());
}

TEST_F(ApertureCtrlsTest, RemoveExternalShape_ValidIndex_RemovesShape) {
    apertureCtrls->AddExternalShape(std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0));
    ASSERT_EQ(1, apertureCtrls->GetExternalCount());
    
    bool removed = apertureCtrls->RemoveExternalShape(0);
    
    EXPECT_TRUE(removed);
    EXPECT_EQ(0, apertureCtrls->GetExternalCount());
}

// ============================================================================
// Version Tracking Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, Version_InitiallyNonZero) {
    EXPECT_GT(apertureCtrls->GetVersion(), 0);
}

TEST_F(ApertureCtrlsTest, AddShape_IncrementsVersion) {
    uint64_t v1 = apertureCtrls->GetVersion();
    
    apertureCtrls->AddExternalShape(std::make_unique<aperture::Ellipse>(50, 50, 100, 100));
    
    uint64_t v2 = apertureCtrls->GetVersion();
    EXPECT_GT(v2, v1);
}

TEST_F(ApertureCtrlsTest, NotifyShapeModified_IncrementsVersion) {
    apertureCtrls->AddExternalShape(std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0));
    
    uint64_t v1 = apertureCtrls->GetVersion();
    
    apertureCtrls->NotifyShapeModified();
    
    uint64_t v2 = apertureCtrls->GetVersion();
    EXPECT_GT(v2, v1);
}

// ============================================================================
// Hit Testing Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, HitTest_PointInsideShape_ReturnsHit) {
    // Add a rectangle at (50, 50) with size 100x100
    auto rect = std::make_unique<aperture::Rectangle>(100.0, 100.0, 100.0, 100.0);
    apertureCtrls->AddExternalShape(std::move(rect));
    
    // Test point at center
    Point testPoint{100.0, 100.0};
    auto result = apertureCtrls->HitTest(testPoint, 5.0);
    
    EXPECT_TRUE(result.hitShape());
    EXPECT_TRUE(result.hitBody());
    EXPECT_FALSE(result.hitControlPoint());
}

TEST_F(ApertureCtrlsTest, HitTest_PointOutsideShape_ReturnsNoHit) {
    auto rect = std::make_unique<aperture::Rectangle>(100.0, 100.0, 100.0, 100.0);
    apertureCtrls->AddExternalShape(std::move(rect));
    
    // Test point far outside
    Point testPoint{500.0, 500.0};
    auto result = apertureCtrls->HitTest(testPoint, 5.0);
    
    EXPECT_FALSE(result.hitShape());
}
