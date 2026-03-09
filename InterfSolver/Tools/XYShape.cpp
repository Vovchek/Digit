#include "XYShape.h"
#include "XYBrokenLine.h"
#include "XYBounds.h"

//=========================================================================
// Constructor
//=========================================================================
XYShape::XYShape(int _TypeLimits, int _TypeSystCoor)
    : TypeLimits(_TypeLimits), TypeSystCoor(_TypeSystCoor)
{
}

//=========================================================================
// Destructor
//=========================================================================
XYShape::~XYShape()
{
}

//=========================================================================
// Type Classification - Common Implementation
//=========================================================================

void XYShape::SetTypeLimits(int TypLim)
{
    TypeLimits = TypLim;
}

int XYShape::GetTypeLimits() const
{
    return TypeLimits;
}

void XYShape::SetTypeSystCoor(int _TypeSystCoor)
{
    TypeSystCoor = _TypeSystCoor;
}

int XYShape::GetTypeSystCoor() const
{
    return TypeSystCoor;
}

//=========================================================================
// Visibility - Template Method Pattern (uses pure virtual isInside)
//=========================================================================

/**
 * @brief CORRECT isVisible implementation
 * 
 * This is the correct logic that was fixed in the isPupil refactoring.
 * 
 * Visibility means "not blocked by this shape":
 * - EXTERNAL shapes (apertures): block points OUTSIDE → visible if INSIDE
 * - INTERNAL shapes (obstructions): block points INSIDE → visible if OUTSIDE
 * 
 * Truth table:
 * ┌──────────┬─────────┬──────────┐
 * │TypeLimits│ isInside│ isVisible│
 * ├──────────┼─────────┼──────────┤
 * │ EXTERNAL │  false  │  false   │ Point outside aperture → blocked
 * │ EXTERNAL │  true   │  true    │ Point inside aperture → visible
 * │ INTERNAL │  false  │  true    │ Point outside obstruction → visible
 * │ INTERNAL │  true   │  false   │ Point inside obstruction → blocked
 * └──────────┴─────────┴──────────┘
 */
bool XYShape::isVisible(const XYPoint &P) const
{
    bool isIn = isInside(P);  // Call pure virtual method
    
    if (isIn && TypeLimits == INTERNAL)
        return false;   // Inside obstruction → blocked
    else if (!isIn && TypeLimits == EXTERNAL)
        return false;   // Outside aperture → blocked
        
    return true;  // All other cases → visible
}

bool XYShape::isVisible(double X, double Y) const
{
    XYPoint P(X, Y);
    return isVisible(P);
}

//=========================================================================
// Bounding Box - Default Implementation
//=========================================================================

void XYShape::GetBounds(XYBounds &Bnd) const
{
    Bnd = GetBounds();
}
