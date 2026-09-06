/**
 * @file BoundsHandlerPhase2Test.cpp
 * @brief Google Test suite for BoundsHandler - Phase 2 (Modal Editing & Draft Shapes)
 * 
 * Tests Phase 2 features:
 * - Edit mode management
 * - Draft shape workflows
 * - Keyboard handling
 * - Point accumulation and preview
 */

#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/BoundsHandler.h"
#include "DigitMode/EditMode.h"
#include "DigitMode/CommandDispatcher.h"
#include "Controls/CApertureCtrls.h"
#include "ImageTempl/ViewTransform.h"

using namespace DigitMode;
using namespace aperture;

// ============================================================================
// Test Fixture for Phase 2
// ============================================================================
class ImageDataMock : public IImageData {
    public:
    bool hasImage() const override { return true; }
    int getWidth() const override { return 800; }
    int getHeight() const override { return 600; }
    const unsigned char* getBitmapData() const override { return nullptr; }
    unsigned char getPixel(int, int) const override { return 0; }
    uint64_t getImageVersion() const override { return 1; }  // Always valid
};

class BoundsHandlerPhase2Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create real aperture controls
		m_imageData = std::make_unique<ImageDataMock>();
        m_apertureCtrls = std::make_unique<CApertureCtrls>(*m_imageData);
        m_apertureCtrls->SetVisibilityDomain(800, 600);
        
        // Create real command dispatcher
        m_dispatcher = std::make_unique<CommandDispatcher>();
        
        // Create view transform (1:1 scale, no offset)
        m_viewTransform = std::make_unique<ViewTransform>();
        m_viewTransform->SetScale(1.0);
        m_viewTransform->SetOffset({0.0, 0.0});
        
        // Setup handler
        m_handler.SetApertureCtrls(m_apertureCtrls.get(), nullptr);
        m_handler.SetViewTransform(m_viewTransform.get());
        m_handler.SetCommandDispatcher(m_dispatcher.get());
    }
    
    void TearDown() override {
        m_apertureCtrls.reset();
        m_dispatcher.reset();
        m_viewTransform.reset();
    }

    BoundsHandler m_handler;
    std::unique_ptr<CApertureCtrls> m_apertureCtrls;
    std::unique_ptr<CommandDispatcher> m_dispatcher;
    std::unique_ptr<ViewTransform> m_viewTransform;
	std::unique_ptr<ImageDataMock> m_imageData = std::make_unique<ImageDataMock>();
    
    static constexpr double TOLERANCE = 1e-6;
};

// ============================================================================
// Edit Mode Management Tests
// ============================================================================

TEST_F(BoundsHandlerPhase2Test, DefaultEditMode_IsSelect) {
    EXPECT_EQ(m_handler.GetEditMode(), ShapeEditMode::Select);
}

TEST_F(BoundsHandlerPhase2Test, SetEditMode_ChangesMode) {
    m_handler.SetEditMode(ShapeEditMode::AddRectangle);
    EXPECT_EQ(m_handler.GetEditMode(), ShapeEditMode::AddRectangle);
    
    m_handler.SetEditMode(ShapeEditMode::AddCircle);
    EXPECT_EQ(m_handler.GetEditMode(), ShapeEditMode::AddCircle);
    
    m_handler.SetEditMode(ShapeEditMode::Select);
    EXPECT_EQ(m_handler.GetEditMode(), ShapeEditMode::Select);
}

TEST_F(BoundsHandlerPhase2Test, SetEditMode_ClearsDraftOnSwitch) {
    // Start in AddRectangle mode
    m_handler.SetEditMode(ShapeEditMode::AddRectangle);
    
    // Add some points to draft
    m_handler.AddDraftPoint({10, 20});
    m_handler.AddDraftPoint({110, 20});
    EXPECT_TRUE(m_handler.IsDrafting());
    
    // Switch to Select mode - should clear draft
    m_handler.SetEditMode(ShapeEditMode::Select);
    EXPECT_FALSE(m_handler.IsDrafting());
    
    // Switch to AddEllipse - should not have residual draft
    m_handler.SetEditMode(ShapeEditMode::AddEllipse);
    EXPECT_FALSE(m_handler.IsDrafting());  // Fresh draft, no points yet
}

// ============================================================================
// Draft Point Accumulation Tests
// ============================================================================

TEST_F(BoundsHandlerPhase2Test, AddDraftPoint_InSelectMode_ReturnsFalse) {
    m_handler.SetEditMode(ShapeEditMode::Select);
    
    bool result = m_handler.AddDraftPoint({10, 20});
    
    EXPECT_FALSE(result);
    EXPECT_FALSE(m_handler.IsDrafting());
}

TEST_F(BoundsHandlerPhase2Test, AddDraftPoint_InAddMode_AcceptsPoint) {
    m_handler.SetEditMode(ShapeEditMode::AddRectangle);
    
    bool result = m_handler.AddDraftPoint({10, 20});
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(m_handler.IsDrafting());
}

TEST_F(BoundsHandlerPhase2Test, AddRectangle_ThreePoints_Commits) {
    m_handler.SetEditMode(ShapeEditMode::AddRectangle);
    
    // Add 3 points
    m_handler.AddDraftPoint({0, 0});
    m_handler.AddDraftPoint({100, 0});
    m_handler.AddDraftPoint({100, 50});
    
    // Count shapes before
    size_t countBefore = m_apertureCtrls->GetShapes().getApertures().size();
    
    // Commit draft
    bool committed = m_handler.CommitDraft();
    
    EXPECT_TRUE(committed);
    EXPECT_FALSE(m_handler.IsDrafting());  // Draft cleared after commit
    
    // Verify shape was added
    size_t countAfter = m_apertureCtrls->GetShapes().getApertures().size();
    EXPECT_EQ(countAfter, countBefore + 1);
}

TEST_F(BoundsHandlerPhase2Test, AddEllipse_FourPoints_UpdatesPreview) {
    m_handler.SetEditMode(ShapeEditMode::AddEllipse);
    
    // First point - no preview yet
    m_handler.AddDraftPoint({10, 0});
    EXPECT_EQ(m_handler.GetDraftPreview(), nullptr);
    
    // Second point - still no preview (need 3 for ellipse)
    m_handler.AddDraftPoint({0, 10});
    EXPECT_EQ(m_handler.GetDraftPreview(), nullptr);
    
    // Third point - preview appears
    m_handler.AddDraftPoint({-10, 0});
    EXPECT_EQ(m_handler.GetDraftPreview(), nullptr);
    
    // Fourth point - preview updates
    m_handler.AddDraftPoint({0, -10});
    auto preview = m_handler.GetDraftPreview();
    ASSERT_NE(preview, nullptr);
    EXPECT_STREQ(preview->typeName(), "Ellipse");
}

// ============================================================================
// Draft Finalization Tests
// ============================================================================

TEST_F(BoundsHandlerPhase2Test, CancelDraft_ClearsDraft) {
    m_handler.SetEditMode(ShapeEditMode::AddEllipse);
    
    // Add some points
    m_handler.AddDraftPoint({10, 0});
    m_handler.AddDraftPoint({0, 10});
    m_handler.AddDraftPoint({-10, 0});
    m_handler.AddDraftPoint({0, -10});

    EXPECT_TRUE(m_handler.IsDrafting());
    EXPECT_NE(m_handler.GetDraftPreview(), nullptr);
    
    // Cancel draft
    m_handler.CancelDraft();
    
    EXPECT_FALSE(m_handler.IsDrafting());
    EXPECT_EQ(m_handler.GetDraftPreview(), nullptr);
}

TEST_F(BoundsHandlerPhase2Test, CommitDraft_DispatchesAddCommand) {
    m_handler.SetEditMode(ShapeEditMode::AddPolygon);
    
    size_t countBefore = m_apertureCtrls->GetShapes().getApertures().size();
    
    // Add polygon vertices
    m_handler.AddDraftPoint({0, 0});
    m_handler.AddDraftPoint({100, 0});
    m_handler.AddDraftPoint({50, 86.6});
    
    m_handler.CommitDraft();
    
    size_t countAfter = m_apertureCtrls->GetShapes().getApertures().size();
    EXPECT_EQ(countAfter, countBefore + 1);
}

// ============================================================================
// Keyboard Input Tests
// ============================================================================

TEST_F(BoundsHandlerPhase2Test, OnKeyDown_Escape_CancelsDraft) {
    m_handler.SetEditMode(ShapeEditMode::AddRectangle);
    
    m_handler.AddDraftPoint({0, 0});
    m_handler.AddDraftPoint({100, 0});
    
    EXPECT_TRUE(m_handler.IsDrafting());
    
    // Press Escape
    bool handled = m_handler.OnKeyDown(VK_ESCAPE, 1, 0);
    
    EXPECT_TRUE(handled);
    EXPECT_FALSE(m_handler.IsDrafting());
}

TEST_F(BoundsHandlerPhase2Test, OnKeyDown_Enter_CommitsPolygon) {
    m_handler.SetEditMode(ShapeEditMode::AddPolygon);
    
    size_t countBefore = m_apertureCtrls->GetShapes().getApertures().size();
    
    // Add polygon vertices
    m_handler.AddDraftPoint({0, 0});
    m_handler.AddDraftPoint({100, 0});
    m_handler.AddDraftPoint({50, 86.6});
    
    // Press Enter to commit
    bool handled = m_handler.OnKeyDown(VK_RETURN, 1, 0);
    
    EXPECT_TRUE(handled);
    EXPECT_FALSE(m_handler.IsDrafting());
    
    size_t countAfter = m_apertureCtrls->GetShapes().getApertures().size();
    EXPECT_EQ(countAfter, countBefore + 1);
}

TEST_F(BoundsHandlerPhase2Test, OnKeyDown_Enter_NoEffect_InSelectMode) {
    m_handler.SetEditMode(ShapeEditMode::Select);
    
    // No draft, so Enter should not be handled
    bool handled = m_handler.OnKeyDown(VK_RETURN, 1, 0);
    
    EXPECT_FALSE(handled);
}

// ============================================================================
// Integration Tests - Full Workflows
// ============================================================================

TEST_F(BoundsHandlerPhase2Test, Workflow_CreateRectangle_Complete) {
    m_handler.SetEditMode(ShapeEditMode::AddRectangle);
    EXPECT_EQ(m_handler.GetEditMode(), ShapeEditMode::AddRectangle);
    
    size_t countBefore = m_apertureCtrls->GetShapes().getApertures().size();
    
    m_handler.AddDraftPoint({10, 20});
    m_handler.AddDraftPoint({110, 20});
    auto preview1 = m_handler.GetDraftPreview();
    EXPECT_NE(preview1, nullptr);  // 2-point preview
    
    m_handler.AddDraftPoint({110, 70});
    auto preview2 = m_handler.GetDraftPreview();
    EXPECT_NE(preview2, nullptr);  // 3-point final preview
    
    bool committed = m_handler.CommitDraft();
    EXPECT_TRUE(committed);
    
    EXPECT_FALSE(m_handler.IsDrafting());
    
    size_t countAfter = m_apertureCtrls->GetShapes().getApertures().size();
    EXPECT_EQ(countAfter, countBefore + 1);
}

TEST_F(BoundsHandlerPhase2Test, Workflow_CreateCircle_ThenCancel) {
    m_handler.SetEditMode(ShapeEditMode::AddCircle);
    
    m_handler.AddDraftPoint({10, 0});
    m_handler.AddDraftPoint({0, 10});
    m_handler.AddDraftPoint({-10, 0});
    
    EXPECT_TRUE(m_handler.IsDrafting());
    
    // User changes mind and presses Escape
    m_handler.OnKeyDown(VK_ESCAPE, 1, 0);
    
    EXPECT_FALSE(m_handler.IsDrafting());
    
    // Switch back to Select mode
    m_handler.SetEditMode(ShapeEditMode::Select);
    EXPECT_EQ(m_handler.GetEditMode(), ShapeEditMode::Select);
}

TEST_F(BoundsHandlerPhase2Test, Workflow_SwitchModes_ClearsInProgressDraft) {
    // User starts creating rectangle
    m_handler.SetEditMode(ShapeEditMode::AddRectangle);
    m_handler.AddDraftPoint({0, 0});
    m_handler.AddDraftPoint({100, 0});
    
    EXPECT_TRUE(m_handler.IsDrafting());
    
    // User switches to AddCircle without committing
    m_handler.SetEditMode(ShapeEditMode::AddCircle);
    
    // Draft should be cleared
    EXPECT_FALSE(m_handler.IsDrafting());
    
    // New draft is fresh
    m_handler.AddDraftPoint({10, 0});
    EXPECT_TRUE(m_handler.IsDrafting());
}
