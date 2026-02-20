/**
 * @file ToolCapabilities.h
 * @brief Tool capability declaration for interaction system
 * 
 * Allows tools to declare their interaction model without hardcoding
 * tool-specific logic in the manager.
 */

#pragma once

namespace DigitMode {

/**
 * @brief Capability declaration for tools
 * 
 * Tools declare what they can and cannot do in the interaction system.
 * InteractionManager uses these to make arbitration decisions.
 */
struct ToolCapabilities {
    /// Can this tool accept hover highlights when not active?
    /// 
    /// Example: Bounds tool can show hover highlights even when Fringe is active.
    bool supportsHover = true;
    
    /// Can foreign tools temporarily capture input via captureTool?
    /// 
    /// Example: 
    /// - Fringe tool: allowForeignDrags=true (bounds can capture while fringe active)
    /// - Bounds tool: allowForeignDrags=true (fringe can capture while bounds active)
    /// - Fiducials: allowForeignDrags=false (exclusive tool, no foreign capture)
    bool allowForeignDrags = true;
    
    /// Must this tool be explicitly activated to use?
    /// 
    /// Example:
    /// - Fringe: false (can respond to foreign drag)
    /// - Bounds: false (can respond to foreign drag)
    /// - Fiducials: true (must be active tool to use)
    bool requiresExplicitActivation = false;
    
    /// When this tool is active, does it exclude other tools from any interaction?
    /// 
    /// Example:
    /// - Fringe: false (not exclusive)
    /// - Bounds: false (not exclusive)
    /// - Fiducials: true (exclusive, cancels other tool interaction)
    bool isExclusive = false;
    
    /// Priority when multiple tools claim the same hit point
    /// Higher = takes precedence in arbitration.
    /// 
    /// Default = 100
    /// Can be adjusted per tool or per tool state (e.g., handle = 200, body = 100)
    int hitTestPriority = 100;
    
    /// Human-readable description of tool (for debugging)
    const char* name = "Unknown";
};

}  // namespace DigitMode
