#pragma once

#include <afxwin.h>
#include <afxtempl.h>

/**
 * @brief Interface for accessing bounds control data
 * 
 * Allows BoundsHandler to work with both real CBoundCtrls and test mocks
 * without coupling to concrete implementations.
 */
class IBoundsData {
public:
    virtual ~IBoundsData() = default;
    
    /**
     * Get the type of external bound (BOUND_RECT, BOUND_ROUND, etc.)
     * @return Bound type constant, or -1 if no external bound
     */
    virtual int GetExtBoundType() const = 0;
    
    /**
     * Get the type of internal bound
     * @return Bound type constant, or -1 if no internal bound
     */
    virtual int GetInsBoundType() const = 0;
    
    /**
     * Get the actual external bound rectangle/polygon
     * @param Type Expected bound type (BOUND_RECT, BOUND_ROUND, etc.)
     * @param xDIB Image width
     * @param yDIB Image height
     * @param Bound Output: bounding rectangle
     * @param PlgPoints Output: polygon points (for BOUND_POLYGON)
     * @return TRUE if bound retrieved successfully
     */
    virtual BOOL GetExtRealBound(int Type, int xDIB, int yDIB, 
                                 CRect& Bound, 
                                 CArray<CPoint, CPoint>& PlgPoints) = 0;
    
    /**
     * Get the actual internal bound rectangle/polygon
     * @param Type Expected bound type
     * @param xDIB Image width
     * @param yDIB Image height
     * @param Bound Output: bounding rectangle
     * @param PlgPoints Output: polygon points (for BOUND_POLYGON)
     * @return TRUE if bound retrieved successfully
     */
    virtual BOOL GetInsRealBound(int Type, int xDIB, int yDIB,
                                 CRect& Bound,
                                 CArray<CPoint, CPoint>& PlgPoints) = 0;
};

/**
 * @brief Interface for accessing image control data
 * 
 * Provides image dimensions needed for bounds calculations.
 */
class IImageData {
public:
    virtual ~IImageData() = default;
    
    /**
     * Get the image dimensions
     * @return Size of the image in pixels
     */
    virtual CSize GetImageSize() const = 0;
};
