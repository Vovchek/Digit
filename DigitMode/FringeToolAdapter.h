/**
 * @file FringeToolAdapter.h
 * @brief Tool adapter for fringe editing handler
 * 
 * Wraps FringeInputHandler (IInputHandler) as IInteractionTool
 * for use in the InteractionManager system.
 */

#pragma once

#include "InputHandlerAdapter.h"

namespace DigitMode {

// Forward declarations
class FringeInputHandler;

/**
 * @brief Specialized adapter for FringeInputHandler
 * 
 * Provides fringe-specific implementations of:
 * - GetViewState: Fringe visualization (dots, rubber-band, etc.)
 */
class FringeToolAdapter : public InputHandlerAdapter {
public:
    /// Construct adapter wrapping fringe handler
    /// @param fringeInputHandler The input handler to wrap
    FringeToolAdapter(FringeInputHandler* fringeInputHandler);
    
    /// Get visual state: dots, fringes, rubber-band
    ViewState GetViewState(bool isActive, bool isCapturing) const override;
    
private:
    FringeInputHandler* m_fringeInputHandler;
};

}  // namespace DigitMode
