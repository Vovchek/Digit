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

## Topology Considerations
- Saddle cases are not based on shared endpoints; they occur when more than two curves are adjacent around a point, and constraints should reflect that topology rather than shared endpoints.