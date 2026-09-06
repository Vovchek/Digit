#pragma once

#include <afxwin.h>

/**
 * @brief Base interface for bounds control implementations
 * 
 * Marker interface for polymorphic bounds handling.
 * CBoundCtrls (legacy CRect-based) and CApertureCtrls (modern Shape-based)
 * have no common API - they use fundamentally different approaches.
 * 
 * @note This interface is kept empty to allow future common methods
 *       if a unified abstraction emerges.
 */
class IBoundsData {
public:
    virtual ~IBoundsData() = default;
    
    // No common methods between legacy (CRect/BOUND_TYPE) and modern (Shape) approaches
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
    virtual bool hasImage() const = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    virtual const unsigned char* getBitmapData() const = 0;
    virtual unsigned char getPixel(int x, int y) const = 0;

    // Version increases when image changes
    virtual uint64_t getImageVersion() const = 0;
};
