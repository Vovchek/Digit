/**
 * @file FringeToolAdapter.h
 * @brief Tool adapter for fringe editing handler
 * 
 * Wraps FringeInputHandler (IInputHandler) as IInteractionTool
 * for use in the InteractionManager system.
 */

#pragma once

#include "InputHandlerAdapter.h"
#include "SelectionManager.h"

namespace DigitMode {

// Forward declarations
class FringeInputHandler;

/**
 * @brief Specialized adapter for FringeInputHandler
 * 
 * Provides fringe-specific implementations of:
 * - GetViewState: Fringe visualization (dots, rubber-band, etc.) and tooltips
 * - OnMouseMove: Track hover state for tooltip generation
 */
class FringeToolAdapter : public InputHandlerAdapter {
public:
    /// Construct adapter wrapping fringe handler
    /// @param fringeInputHandler The input handler to wrap
    FringeToolAdapter(FringeInputHandler* fringeInputHandler);
    
	HitResult HitTest(CPoint screenPt, int tolerance = 5) override;

    /// Override to track hover state for tooltips
    void OnMouseMove(const ToolContext& ctx) override;
    
    /// Get visual state: dots, fringes, rubber-band, tooltips
    ViewState GetViewState(bool isActive, bool isCapturing) const override;
    
private:
    FringeInputHandler* m_fringeInputHandler;
    
    /// Hover state for tooltip generation (mutable for GetViewState const-correctness)
    mutable SelectionManager::SelectedObject m_hoveredObject;
};

}  // namespace DigitMode
