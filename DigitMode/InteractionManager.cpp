/**
 * @file InteractionManager.cpp
 * @brief Central tool manager and event router implementation
 */

#include "stdafx.h"
#include "InteractionManager.h"
#include <algorithm>

namespace DigitMode {

InteractionManager::InteractionManager()
    : m_activeTool(nullptr)
    , m_captureTool(nullptr)
    , m_hoveredTool(nullptr)
    , m_requestInvalidate(false)
{
}

InteractionManager::~InteractionManager()
{
    // Tools are not owned; just clear references
    m_activeTool = nullptr;
    m_captureTool = nullptr;
    m_hoveredTool = nullptr;
    m_tools.clear();
}

// ========================================================================
// Initialization
// ========================================================================

void InteractionManager::RegisterTool(IInteractionTool* tool)
{
    if (!tool) return;
    m_tools.push_back(tool);
}

void InteractionManager::SetActiveTool(IInteractionTool* tool)
{
    if (m_activeTool == tool) return;  // No-op if same tool
    
    // Deactivate old tool
    if (m_activeTool) {
        m_activeTool->OnDeactivate();
    }
    
    // Set new active tool
    m_activeTool = tool;
    m_captureTool = nullptr;  // Clear capture on mode switch
    
    // Activate new tool
    if (m_activeTool) {
        m_activeTool->OnActivate();
    }
}

// ========================================================================
// Hit-Testing Arbitration (Core of Manager)
// ========================================================================

HitResult InteractionManager::BestHit(CPoint screenPt, int tolerance) const
{
    std::vector<HitResult> hits;
    
    // Ask all tools for hit results
    for (auto tool : m_tools) {
        auto hit = tool->HitTest(screenPt, tolerance);
        if (hit.hit) {
            // Set tool reference and priority
            hit.tool = tool;
            hit.priority = tool->GetCapabilities().hitTestPriority;
            hits.push_back(hit);
        }
    }
    
    // No hits
    if (hits.empty()) {
        return HitResult();
    }
    
    // Sort by priority (higher first) then distance (lower first)
    std::sort(hits.begin(), hits.end());
    return hits[0];
}

// ========================================================================
// Context Building
// ========================================================================

ToolContext InteractionManager::MakeContext(UINT flags, CPoint pt, const HitResult& hit)
{
    ToolContext ctx;
    ctx.mouseFlags = flags;
    ctx.screenPoint = pt;
    ctx.hit = hit;
    
    // Capture modifier state
    UpdateModifiers(ctx);
    
    // Previous state for delta computation
    if (m_lastMousePoint != CPoint(0, 0)) {
        ctx.previous.screenPoint = m_lastMousePoint;
        ctx.previous.valid = true;
    }
    m_lastMousePoint = pt;
    
    return ctx;
}

void InteractionManager::UpdateModifiers(ToolContext& ctx)
{
    ctx.shiftKey = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
    ctx.altKey = (::GetKeyState(VK_MENU) & 0x80) != 0;    // VK_MENU = Alt key
    ctx.ctrlKey = (::GetKeyState(VK_CONTROL) & 0x80) != 0;
}

// ========================================================================
// Event Routing (Main Interface)
// ========================================================================

bool InteractionManager::OnMouseDown(UINT flags, CPoint pt)
{
    if (!m_activeTool) return false;
    
    // Find what was hit
    auto hit = BestHit(pt, 5);
    m_lastHit = hit;
    auto ctx = MakeContext(flags, pt, hit);
    
    if (!hit.hit) {
        // Nothing hit - not consumed, let navigation handle it
        return false;
    }
    
    // Check if hit belongs to active tool
    if (hit.tool == m_activeTool) {
        // Normal case: active tool handles it
        bool consumed = m_activeTool->OnMouseDown(ctx);
        m_requestInvalidate = true;
        return consumed;
    }
    
    // Foreign tool hit - check if we can capture
    if (!m_activeTool->GetCapabilities().allowForeignDrags) {
        // Not allowed to capture - not consumed
        return false;
    }
    
    // Capture semantics: temporary tool capture without mode switch
    m_captureTool = hit.tool;
    bool consumed = m_captureTool->OnMouseDown(ctx);
    m_requestInvalidate = true;
    return consumed;
}

bool InteractionManager::OnMouseMove(UINT flags, CPoint pt)
{
    // Route to capture tool if active, else active tool
    auto tool = m_captureTool ? m_captureTool : m_activeTool;
    if (!tool) return false;
    
    auto hit = BestHit(pt);
    m_lastHit = hit;
    auto ctx = MakeContext(flags, pt, hit);
    
    // Update hover state (for hover highlighting)
    if (hit.tool != m_hoveredTool) {
        m_hoveredTool = hit.tool;
        m_requestInvalidate = true;
    }
    
    // Route event
    tool->OnMouseMove(ctx);
    
    // Request invalidation for preview updates
    m_requestInvalidate = true;
    return true;  // Always consumed during active drag/interaction
}

bool InteractionManager::OnMouseUp(UINT flags, CPoint pt)
{
    // Route to tool that has capture (or active)
    auto tool = m_captureTool ? m_captureTool : m_activeTool;
    if (!tool) return false;
    
    auto ctx = MakeContext(flags, pt, m_lastHit);
    tool->OnMouseUp(ctx);
    
    // Clear capture (temporary tool release)
    m_captureTool = nullptr;
    
    m_requestInvalidate = true;
    return true;  // Consumed
}

bool InteractionManager::OnMouseWheel(UINT flags, short zDelta, CPoint pt)
{
    if (!m_activeTool) return false;
    
    auto ctx = MakeContext(flags, pt);
    bool consumed = m_activeTool->OnMouseWheel(ctx, zDelta);
    if (consumed) {
        m_requestInvalidate = true;
    }
    return consumed;
}

bool InteractionManager::OnKeyDown(UINT nChar)
{
    if (!m_activeTool) return false;
    
    ToolContext ctx;
    UpdateModifiers(ctx);
    ctx.screenPoint = m_lastMousePoint;
    
    bool consumed = m_activeTool->OnKeyDown(ctx, nChar);
    if (consumed) {
        m_requestInvalidate = true;
    }
    return consumed;
}

void InteractionManager::Cancel()
{
    // Cancel active tool
    if (m_activeTool) {
        m_activeTool->Cancel();
    }
    
    // Cancel capture tool
    if (m_captureTool) {
        m_captureTool->Cancel();
    }
    
    // Clear capture
    m_captureTool = nullptr;
    m_requestInvalidate = true;
}

// ========================================================================
// View State Composition (For Rendering)
// ========================================================================

InteractionManager::CompositeViewState InteractionManager::GetViewState() const
{
    CompositeViewState state;
    
    // Layer 1: Active tool (always first)
    if (m_activeTool) {
        bool isCapturing = (m_captureTool == nullptr);
        auto toolState = m_activeTool->GetViewState(true, isCapturing);
        state.layers.push_back({toolState, m_activeTool, true, isCapturing});
    }
    
    // Layer 2: Capture tool (if different from active)
    if (m_captureTool && m_captureTool != m_activeTool) {
        auto toolState = m_captureTool->GetViewState(false, true);
        state.layers.push_back({toolState, m_captureTool, false, true});
    }
    
    // Layer 3: Hovered tool (if different from active/capture and foreign)
    if (m_hoveredTool && m_hoveredTool != m_activeTool && m_hoveredTool != m_captureTool) {
        auto toolState = m_hoveredTool->GetViewState(false, false);
        state.layers.push_back({toolState, m_hoveredTool, false, false});
    }
    
    // Resolve cursor (first non-null from layers)
    for (const auto& layer : state.layers) {
        if (layer.toolState.cursor) {
            state.cursor = layer.toolState.cursor;
            break;
        }
    }
    
    // Resolve status text (prefer active, then capture)
    if (m_activeTool) {
        bool isCapturing = (m_captureTool == nullptr);
        auto toolState = m_activeTool->GetViewState(true, isCapturing);
        if (!toolState.statusText.IsEmpty()) {
            state.statusText = toolState.statusText;
        }
    }
    
    if (state.statusText.IsEmpty() && m_captureTool) {
        auto toolState = m_captureTool->GetViewState(false, true);
        if (!toolState.statusText.IsEmpty()) {
            state.statusText = toolState.statusText;
        }
    }
    
    // Resolve tooltip (from hovered, if different from active)
    if (m_hoveredTool && m_hoveredTool != m_activeTool) {
        auto toolState = m_hoveredTool->GetViewState(false, false);
        if (!toolState.tooltip.IsEmpty()) {
            state.tooltip = toolState.tooltip;
        }
    }
    
    // Request invalidation if any tool needs it
    for (const auto& layer : state.layers) {
        if (layer.toolState.requestInvalidate) {
            state.requestInvalidate = true;
            break;
        }
    }
    
    return state;
}

// ========================================================================
// State Queries
// ========================================================================

bool InteractionManager::IsDragging() const
{
    // Simple heuristic: capturing tool means dragging
    // Can be enhanced to query tools
    return m_captureTool != nullptr;
}

bool InteractionManager::IsDrafting() const
{
    // Placeholder: can be enhanced to query tools
    // Would ask active/capture tool if they're drafting
    return false;
}

}  // namespace DigitMode
