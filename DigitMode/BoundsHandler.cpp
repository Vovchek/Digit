/**
 * @file BoundsHandler.cpp
 * @brief Implementation of BoundsHandler with strict Command pattern
 * 
 * IMPORTANT:
 * Shape geometry MUST NOT be modified outside Command::Execute/Undo.
 * BoundsHandler operates on preview copies only.
 */
#include "stdafx.h"
#include "BoundsHandler.h"
#include "Controls\CApertureCtrls.h"
#include "Commands\ReplaceShapeCommand.h"
#include "Commands\AddShapeCommand.h"
#include "CommandDispatcher.h"
#include "Rendering\ShapeDrawStyle.h"
#include "Rendering\ShapeDrawDispatcher.h"
#include "ApertureCore\include\aperturecore\geometry\Handle.h"

namespace DigitMode {

BoundsHandler::BoundsHandler()
    : m_pApertureCtrls(nullptr)
    , m_pImage(nullptr)
    , m_pView(nullptr)
    , m_pDispatcher(nullptr)
    , m_isDragging(false)
    , m_dragShapeType(aperture::TypeLimits::EXTERNAL)
    , m_dragShapeIndex(0)
    , m_dragControlPointIndex(-1)
{
}

BoundsHandler::~BoundsHandler()
{
    // No cleanup needed (pointers are not owned)
}

// ========================================================================
// Initialization
// ========================================================================

void BoundsHandler::SetApertureCtrls(CApertureCtrls* pApertureCtrls, IImageData* pImage)
{
    m_pApertureCtrls = pApertureCtrls;
    m_pImage = pImage;
}

void BoundsHandler::SetViewTransform(ViewTransform* pView)
{
    m_pView = pView;
}

void BoundsHandler::SetCommandDispatcher(CommandDispatcher* pDispatcher)
{
    m_pDispatcher = pDispatcher;
}

// ========================================================================
// Coordinate Transforms
// ========================================================================

CPoint2d BoundsHandler::ScreenToWorldDouble(const CPoint& screenPt) const
{
    ASSERT(m_pView && "ViewTransform must be set before coordinate conversion");
    
    // Get current transform parameters
    double scale = m_pView->GetScale();
    CPoint2d offset = m_pView->GetOffset();
    
    // Convert to world coordinates with double precision
    CPoint2d worldPt;
    worldPt.x = (screenPt.x - offset.x) / scale;
    worldPt.y = (screenPt.y - offset.y) / scale;
    
    return worldPt;
}

// ========================================================================
// Hit-Testing (Delegates to CApertureCtrls)
// ========================================================================

BoundsHandler::HitResult BoundsHandler::HitTest(const CPoint& screenPt, int tolerance) const
{
    HitResult result;
    
    if (!m_pApertureCtrls || !m_pView) {
        return result;  // Not initialized
    }
    
    // Convert screen point to world coordinates
    aperture::Point worldPt = ScreenToAperturePoint(screenPt);
    
    // Convert screen tolerance to world tolerance
    double scale = m_pView->GetScale();
    double worldTolerance = tolerance / scale;
    
    // Delegate to CApertureCtrls hit-testing
    auto apertureResult = m_pApertureCtrls->HitTest(worldPt, worldTolerance);
    
    if (apertureResult.hitShape()) {
        result.hit = true;
        result.type = apertureResult.type;
        result.shapeIndex = static_cast<size_t>(m_pApertureCtrls->GetShapeIndex(apertureResult.shape));
        result.controlPointIndex = apertureResult.controlPointIndex;
        result.distance = apertureResult.distance;
    }
    
    return result;
}

// ========================================================================
// Handle Hover Detection (Phase 4 - Visual Feedback)
// ========================================================================

bool BoundsHandler::UpdateHoveredHandle(const CPoint& screenPt)
{
    // Store previous hover state
    int previousHandleIndex = m_hoveredHandleIndex;
    size_t previousShapeIndex = m_hoveredShapeIndex;
    aperture::TypeLimits previousType = m_hoveredShapeType;
    
    // Perform hit test
    HitResult hit = HitTest(screenPt, HANDLE_TOLERANCE);
    
    if (hit.hit) {
        m_hoveredShapeType = hit.type;
        m_hoveredShapeIndex = hit.shapeIndex;
        m_hoveredHandleIndex = hit.controlPointIndex;  // -1 for body, >=0 for handle
    } else {
        // No hit - clear hover
        m_hoveredHandleIndex = -1;
    }
    
    // Return true if hover state changed
    bool changed = (m_hoveredHandleIndex != previousHandleIndex ||
                    m_hoveredShapeIndex != previousShapeIndex ||
                    m_hoveredShapeType != previousType);
    
    return changed;
}

const aperture::Shape* BoundsHandler::GetHoveredShape() const
{
    if (m_hoveredHandleIndex == -1 && !m_pApertureCtrls) {
        return nullptr;  // No hover or not initialized
    }
    
    // Get the shape at (m_hoveredShapeType, m_hoveredShapeIndex)
    const std::vector<std::unique_ptr<aperture::Shape>>* container = nullptr;
    
    switch (m_hoveredShapeType) {
        case aperture::TypeLimits::EXTERNAL:
            container = &m_pApertureCtrls->GetShapes().getExternal();
            break;
        case aperture::TypeLimits::INTERNAL:
            container = &m_pApertureCtrls->GetShapes().getInternal();
            break;
        case aperture::TypeLimits::APERTURE:
            container = &m_pApertureCtrls->GetShapes().getApertures();
            break;
    }
    
    if (!container || m_hoveredShapeIndex >= container->size()) {
        return nullptr;
    }
    
    return (*container)[m_hoveredShapeIndex].get();
}

void BoundsHandler::ClearHover()
{
    m_hoveredHandleIndex = -1;
    m_hoveredShapeIndex = 0;
    m_hoveredShapeType = aperture::TypeLimits::EXTERNAL;
}

// ========================================================================
// Drag Lifecycle (Preview + Command Pattern)
// ========================================================================

void BoundsHandler::BeginDrag(aperture::TypeLimits type, size_t index, int controlPointIndex, const CPoint& screenStart)
{
    ASSERT(m_pApertureCtrls && m_pView && "BoundsHandler must be initialized");
    
    if (m_isDragging) {
        return;  // Already dragging
    }

    // Get the shape at (type, index)
    const std::vector<std::unique_ptr<aperture::Shape>>* container = nullptr;
    
    switch (type) {
        case aperture::TypeLimits::EXTERNAL:
            container = &m_pApertureCtrls->GetShapes().getExternal();
            break;
        case aperture::TypeLimits::INTERNAL:
            container = &m_pApertureCtrls->GetShapes().getInternal();
            break;
        case aperture::TypeLimits::APERTURE:
            container = &m_pApertureCtrls->GetShapes().getApertures();
            break;
    }
    
    if (!container || index >= container->size()) {
        return;  // Invalid shape reference
    }
    
    const auto& shape = (*container)[index];
    
    // Clone shape for before-state and preview
    m_originalShape = shape->clone();
    m_previewShape = shape->clone();
    
    // Cache drag info
    m_dragShapeType = type;
    m_dragShapeIndex = index;
    m_dragControlPointIndex = controlPointIndex;
    m_dragStart = screenStart;
    m_dragCurrent = screenStart;
    m_isDragging = true;
}

void BoundsHandler::UpdateDrag(const CPoint& screenCurrent)
{
    if (!m_isDragging || !m_previewShape) {
        return;
    }

    // Compute world-space positions
    aperture::Point dragStartWorld = ScreenToAperturePoint(m_dragStart);
    aperture::Point dragCurrentWorld = ScreenToAperturePoint(screenCurrent);
    
    aperture::Point deltaWorld{
        dragCurrentWorld.x - dragStartWorld.x,
        dragCurrentWorld.y - dragStartWorld.y
    };
    
    // Reset preview to original state
    m_previewShape = m_originalShape->clone();
    
    // Create DragContext with modifier keys
    aperture::DragContext dragContext;
    dragContext.dragStartWorld = dragStartWorld;
    dragContext.dragCurrentWorld = dragCurrentWorld;
    dragContext.deltaWorld = deltaWorld;
    dragContext.shiftKey = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    dragContext.altKey = (GetKeyState(VK_MENU) & 0x8000) != 0;
    
    // Enumerate handles and apply drag based on controlPointIndex
    std::vector<aperture::HandleDesc> handles;
    m_previewShape->EnumerateHandles(handles);
    
    if (m_dragControlPointIndex >= 0 && static_cast<size_t>(m_dragControlPointIndex) < handles.size()) {
        aperture::HandleDesc& handle = handles[m_dragControlPointIndex];
        dragContext.handle = handle;
        
        // Apply drag transformation to preview shape
        m_previewShape->ApplyHandleDrag(handle, dragContext);
    }
    
    m_dragCurrent = screenCurrent;
    
    // Invalidate view to show preview (caller's responsibility to redraw)
}

void BoundsHandler::EndDrag(bool bCommit)
{
    if (!m_isDragging) {
        return;
    }

    if (bCommit && m_pDispatcher && m_originalShape && m_previewShape) {
        // Create ReplaceShapeCommand and dispatch
        auto cmd = std::make_unique<ReplaceShapeCommand>(
            *m_pApertureCtrls,
            m_dragShapeType,
            m_dragShapeIndex,
            std::move(m_originalShape),
            std::move(m_previewShape)
        );
        
        m_pDispatcher->Execute(std::move(cmd));
    }
    
    // Clear preview state
    m_previewShape.reset();
    m_originalShape.reset();
    m_isDragging = false;
    m_dragControlPointIndex = -1;
}

void BoundsHandler::CancelDrag()
{
    EndDrag(false);  // Discard preview
}

// ========================================================================
// Edit Mode Management (Phase 2)
// ========================================================================

void BoundsHandler::SetEditMode(DigitMode::ShapeEditMode mode)
{
    // Cancel any active operations when switching modes
    if (m_isDragging) {
        CancelDrag();
    }
    
    if (IsDrafting()) {
        CancelDraft();
    }
    
    m_editMode = mode;
    
    // Initialize draft if switching to Add mode
    if (mode == DigitMode::ShapeEditMode::AddRectangle ||
        mode == ShapeEditMode::AddEllipse ||
        mode == ShapeEditMode::AddCircle ||
        mode == ShapeEditMode::AddPolygon)
    {
        // Create new draft with appropriate kind
        DraftShape draft;
        draft.type = aperture::TypeLimits::EXTERNAL;  // Default, can be changed later
        
        switch (mode) {
            case ShapeEditMode::AddRectangle:
                draft.kind = DraftShape::Kind::Rectangle;
                break;
            case ShapeEditMode::AddEllipse:
                draft.kind = DraftShape::Kind::Ellipse;
                break;
            case ShapeEditMode::AddCircle:
                draft.kind = DraftShape::Kind::Circle;
                break;
            case ShapeEditMode::AddPolygon:
                draft.kind = DraftShape::Kind::Polygon;
                break;
            default:
                break;
        }
        
        m_draft = draft;
    }
    else {
        // Clear draft if not in Add mode
        m_draft.reset();
        m_draftPreview.reset();
    }
}

// ========================================================================
// Draft Shape Management (Phase 2)
// ========================================================================

bool BoundsHandler::AddDraftPoint(const aperture::Point& worldPt)
{
    // Only accept points in Add modes
    if (m_editMode == ShapeEditMode::Select || m_editMode == ShapeEditMode::Delete) {
        return false;
    }
    
    // Ensure draft is initialized
    if (!m_draft.has_value()) {
        return false;
    }
    
    // Add point to draft
    m_draft->AddPoint(worldPt);
    
    // Update preview
    m_draftPreview = m_draft->GetPreview();
    
    return true;
}

bool BoundsHandler::CommitDraft()
{
    if (!m_draft.has_value()) {
        return false;
    }
    
    if (!m_draft->CanCommit()) {
        return false;
    }
    
    // Convert draft to committed shape
    auto shape = m_draft->ToShape();
    if (!shape) {
        return false;
    }
    
    // Get the type from the draft
    aperture::TypeLimits shapeType = m_draft->type;
    
    // Create and dispatch AddShapeCommand
    if (m_pDispatcher && m_pApertureCtrls) {
        auto cmd = std::make_unique<AddShapeCommand>(
            *m_pApertureCtrls,
            shapeType,
            std::move(shape)
        );
        
        m_pDispatcher->Execute(std::move(cmd));
    }
    
    // Clear draft for next shape
    m_draft->Clear();
    m_draftPreview.reset();
    
    return true;
}

void BoundsHandler::CancelDraft()
{
    if (m_draft.has_value()) {
        m_draft->Clear();
    }
    m_draftPreview.reset();
}

const aperture::Shape* BoundsHandler::GetDraftPreview() const
{
    return m_draftPreview.get();
}

// ========================================================================
// Rendering Preview (Phase 4 - Visual Feedback)
// ========================================================================

void BoundsHandler::RenderPreview(
    CDC& dc,
    const ViewTransform& worldToScreen,
    const ShapeDrawDispatcher& dispatcher) const
{
    using namespace aperture;
    
    // Render drag preview (Selected state with handles)
    if (m_isDragging && m_previewShape) {
        ShapeDrawStyle style;
        style.type = m_dragShapeType;
        style.state = ShapeDrawStyle::State::Selected;  // Thicker outline during drag
        style.showHandles = true;
        style.activeHandleIndex = m_dragControlPointIndex;  // Highlight dragged handle
        
        // Render shape
        dispatcher.Draw(*m_previewShape, dc, style, worldToScreen);
        
        // Render handles
        dispatcher.DrawHandles(*m_previewShape, dc, style, worldToScreen);
    }
    
    // Render draft preview (Draft state - dashed outline)
    if (IsDrafting() && m_draftPreview) {
        ShapeDrawStyle style;
        style.type = m_draft->type;  // Use draft's type
        style.state = ShapeDrawStyle::State::Draft;  // Dashed outline
        style.showHandles = false;  // No handles during draft
        style.activeHandleIndex = -1;
        
        // Render shape only (no handles for drafts)
        dispatcher.Draw(*m_draftPreview, dc, style, worldToScreen);
    }
}

// ========================================================================
// Keyboard Input Handling (Phase 2)
// ========================================================================

bool BoundsHandler::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar) {
        case VK_ESCAPE:
            // Cancel draft or drag
            if (IsDrafting()) {
                CancelDraft();
                return true;
            }
            else if (IsDragging()) {
                CancelDrag();
                return true;
            }
            return false;
        
        case VK_RETURN:
            // Commit draft (for polygon/ellipse/circle finalization)
            if (IsDrafting() && m_draft->CanCommit()) {
                CommitDraft();
                return true;
            }
            return false;
        
        default:
            return false;  // Not handled
    }
}

} // namespace DigitMode
