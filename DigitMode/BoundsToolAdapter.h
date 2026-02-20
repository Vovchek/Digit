/**
 * @file BoundsToolAdapter.h
 * @brief Tool adapter for bounds editing handler
 * 
 * Wraps BoundsInputHandler (IInputHandler) as IInteractionTool
 * for use in the InteractionManager system.
 */

#pragma once

#include "InputHandlerAdapter.h"

namespace DigitMode {

// Forward declarations
class BoundsInputHandler;
class BoundsHandler;

/**
 * @brief Specialized adapter for BoundsInputHandler
 * 
 * Provides bounds-specific implementations of:
 * - HitTest: Test against rendered shapes
 * - GetViewState: Draft preview, handles, status
 */
class BoundsToolAdapter : public InputHandlerAdapter {
public:
    /// Construct adapter wrapping bounds handler
    /// @param boundsInputHandler The input handler to wrap
    /// @param boundsHandler The domain logic handler
    BoundsToolAdapter(BoundsInputHandler* boundsInputHandler, BoundsHandler* boundsHandler);
    
    /// Test for hits against bounds shapes
    HitResult HitTest(CPoint screenPt, int tolerance = 5) override;
    
    /// Get visual state: draft shapes, handles, status text
    ViewState GetViewState(bool isActive, bool isCapturing) const override;
    
private:
    BoundsHandler* m_boundsHandler;
};

}  // namespace DigitMode
