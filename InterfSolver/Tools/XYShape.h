#ifndef _XYSHAPE_H_
#define _XYSHAPE_H_

#include "XYPoint.h"
#include "XYBounds.h"

// Forward declarations
class XYBrokenLine;
class XYPolygon;

/**
 * @brief Abstract base class for all geometric shapes (Ellipse, Rectangle, Polygon)
 * 
 * This class defines the common interface and shared implementation for all
 * geometric shapes used in the interferogram processing system. It provides:
 * - Common visibility and classification properties (TypeLimits, TypeSystCoor)
 * - Standard geometric operations (isInside, isVisible, Normalize)
 * - Contour generation interface
 * 
 * Design Pattern: Template Method
 * - Abstract methods define the contract (must be implemented by derived classes)
 * - Concrete methods provide default implementation using the template pattern
 */
class XYShape
{
public:  // CHANGED: Made public for backward compatibility with existing code
    // Shape classification and coordinate system
    int TypeLimits;     // EXTERNAL (aperture) or INTERNAL (obstruction)
    int TypeSystCoor;   // MEASURING or NORMALISED

public:
    // ========================================================================
    // Constructors and Destructor
    // ========================================================================
    
    /**
     * @brief Default constructor
     * @param _TypeLimits Shape type: EXTERNAL (aperture) or INTERNAL (obstruction)
     * @param _TypeSystCoor Coordinate system: MEASURING or NORMALISED
     */
    XYShape(int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~XYShape();

    // ========================================================================
    // Type Classification - Common Implementation
    // ========================================================================
    
    /**
     * @brief Set the shape classification type
     * @param TypLim EXTERNAL (aperture) or INTERNAL (obstruction)
     */
    virtual void SetTypeLimits(int TypLim);
    
    /**
     * @brief Get the shape classification type
     * @return EXTERNAL or INTERNAL
     */
    virtual int GetTypeLimits() const;
    
    /**
     * @brief Set the coordinate system type
     * @param _TypeSystCoor MEASURING or NORMALISED
     */
    virtual void SetTypeSystCoor(int _TypeSystCoor);
    
    /**
     * @brief Get the coordinate system type
     * @return MEASURING or NORMALISED
     */
    virtual int GetTypeSystCoor() const;

    // ========================================================================
    // Pure Virtual Methods - Must be implemented by derived classes
    // ========================================================================
    
    /**
     * @brief Calculate the perimeter/circumference of the shape
     * @return Perimeter length
     */
    virtual double Perimeter() const = 0;
    
    /**
     * @brief Test if a point is inside the shape
     * @param P Point to test
     * @return true if point is inside, false otherwise
     */
    virtual bool isInside(const XYPoint &P) const = 0;

    /**
     * @brief Test if a point at coordinates (X,Y) is inside the shape
     * @param X X-coordinate
     * @param Y Y-coordinate
     * @return true if point is inside, false otherwise
     */
    virtual bool isInside(double X, double Y) const = 0;
    
    /**
     * @brief Generate contour as broken line with specified number of points
     * @param BLine Output broken line
     * @param NFi Number of points to generate
     * @return true if successful, false otherwise
     */
    virtual bool GetContour(XYBrokenLine &BLine, int NFi) const = 0;
    
    /**
     * @brief Generate contour as broken line with specified step size
     * @param BLine Output broken line
     * @param Step Distance between points
     * @return true if successful, false otherwise
     */
    virtual bool GetContour(XYBrokenLine &BLine, double Step) const = 0;
    
    /**
     * @brief Generate contour as polygon with specified number of points
     * NOTE: Non-const to match existing XYEllipse/XYRect implementations
     * @param Plg Output polygon
     * @param NFi Number of points to generate
     * @return true if successful, false otherwise
     */
    virtual bool GetContour(XYPolygon &Plg, int NFi) = 0;  // NON-CONST for compatibility
    
    /**
     * @brief Generate contour as polygon with specified step size
     * NOTE: Non-const to match existing XYEllipse/XYRect implementations
     * @param Plg Output polygon
     * @param Step Distance between points
     * @return true if successful, false otherwise
     */
    virtual bool GetContour(XYPolygon &Plg, double Step) = 0;  // NON-CONST for compatibility
    
    /**
     * @brief Normalize coordinates to unit system
     * @param Xo Origin X coordinate
     * @param Yo Origin Y coordinate
     * @param Ro Normalization radius/scale factor
     */
    virtual void Normalize(double Xo, double Yo, double Ro) = 0;

    virtual void DeNormalize(double Xo, double Yo, double Ro) = 0;
    virtual void InverseY(double YcInv) = 0;
    virtual void ShiftX(double dX) = 0;
    virtual void ShiftY(double dY) = 0;

    // ========================================================================
    // Template Method Pattern - Common Implementation using pure virtuals
    // ========================================================================
    
    /**
     * @brief Test if a point is visible according to shape type
     * 
     * Visibility logic (CORRECT IMPLEMENTATION):
     * - For EXTERNAL shapes (apertures): visible if point is INSIDE
     * - For INTERNAL shapes (obstructions): visible if point is OUTSIDE
     * 
     * This is the INVERSE of the isVisible check - we want points that
     * are NOT blocked by this shape.
     * 
     * @param P Point to test
     * @return true if point is visible, false if blocked
     */
    virtual bool isVisible(const XYPoint &P) const;
    
    /**
     * @brief Test if a point at coordinates (X,Y) is visible
     * @param X X-coordinate
     * @param Y Y-coordinate
     * @return true if point is visible, false if blocked
     */
    virtual bool isVisible(double X, double Y) const;

    // ========================================================================
    // Optional Virtual Methods 
    // ========================================================================
    
    /**
     * @brief Get bounding box of the shape
     * @return Bounding box
     */
    virtual XYBounds GetBounds() const = 0;
    
    /**
     * @brief Get bounding box of the shape (output parameter version)
     * @param Bnd Output bounding box
     */
    virtual void GetBounds(XYBounds &Bnd) const;
};

#endif // _XYSHAPE_H_
