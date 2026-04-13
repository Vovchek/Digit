# Copilot Instructions

## General Guidelines
- First general instruction
- Second general instruction

## Code Style
- Use specific formatting rules
- Follow naming conventions
- When MFC macro conflicts (e.g., std::min/std::max or aperture::EXTERNAL) cause errors, use `#undef` to remove MFC macro definitions; use unsigned constants (e.g., `0u`) in Google Test comparisons with `size_t`.
- Prefer existing interfaces (use `aperturecore visibility IDataProviders.h`) to avoid duplication, and avoid `MGTools CDPoint` dependency; use a lighter point type like `CPoint2d` or a new simple point struct instead.
- Solution is set to C++17 for all projects; do not downgrade to pre-C++17 syntax. Avoid touching legacy BoundsCtrls.* (deprecated; aim to remove).
- Keep `CMFCStatusBar` for the main frame status bar; avoid using `DockControlBar` on `CReBar` (`m_wndInfoBar`/`m_wndMeasureBar`) because it crashes and prior related visibility changes were ineffective.

## Project-Specific Rules
- EngLangD and RusLangD projects are RESOURCE-ONLY projects. Do NOT add .cpp, .h, .hxx implementation files to these projects. All implementation files (e.g., DigitMode\ShapeConversionHelpers.cpp) must be added to the Digit project (or its subprojects like DigitMode). These two projects exist solely to hold language resource files.

## Topology Considerations
- Saddle cases are not based on shared endpoints; they occur when more than two curves are adjacent around a point, and constraints should reflect that topology rather than shared endpoints.

## Bug Tracking
- Rectangle stretch handles are not responding to drag; only move/rotate work. Verify that the controlPointIndex is being set properly for Rectangle shapes, likely in `CApertureCtrls::HitTest`. The issue may be in the BoundsHandler hit-test or CApertureCtrls hit-test not correctly identifying Rectangle stretch handle indices. Ellipse and polygon handles are functioning correctly.