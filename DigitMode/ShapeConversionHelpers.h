/**
 * @file ShapeConversionHelpers.h
 * @brief Conversion helpers between MFC shape arrays and aperture::ShapeCollection
 * 
 * Simplifies conversion between legacy MFC bound control arrays
 * (CArrayXYEllipse, CArrayXYRect, CArrayXYPolygon) and modern aperture shapes.
 */
#pragma once

#include "ApertureCore\include\aperturecore\visibility\ShapeCollection.h"
#include "ApertureCore\include\aperturecore\geometry\Ellipse.h"
#include "ApertureCore\include\aperturecore\geometry\Rectangle.h"
#include "ApertureCore\include\aperturecore\geometry\Polygon.h"
#include "InterfSolver\Tools\XYEllipse.h"
#include "InterfSolver\Tools\XYRect.h"
#include "InterfSolver\Tools\XYPolygon.h"

namespace DigitMode {

/**
 * @brief Copy ellipses from CArrayXYEllipse to aperture::ShapeCollection
 * 
 * Creates aperture::Ellipse shapes from XYEllipse array.
 * TypeLimits from source shapes are preserved.
 * 
 * @param source MFC array of XYEllipse shapes
 * @param target aperture::ShapeCollection to add shapes to
 */
void CopyEllipsesToAperture(const CArray<XYEllipse>& source, aperture::ShapeCollection& target);

/**
 * @brief Copy rectangles from CArrayXYRect to aperture::ShapeCollection
 * 
 * Creates aperture::Rectangle shapes from XYRect array.
 * Note: XYRect stores Ax/By (half-widths), while aperture::Rectangle expects full widths.
 * Multiplies by 2.0 during conversion.
 * TypeLimits from source shapes are preserved.
 * 
 * @param source MFC array of XYRect shapes
 * @param target aperture::ShapeCollection to add shapes to
 */
void CopyRectsToAperture(const CArray<XYRect>& source, aperture::ShapeCollection& target);

/**
 * @brief Copy polygons from CArrayXYPolygon to aperture::ShapeCollection
 * 
 * Creates aperture::Polygon shapes from XYPolygon array.
 * TypeLimits from source shapes are preserved.
 * 
 * @param source MFC array of XYPolygon shapes
 * @param target aperture::ShapeCollection to add shapes to
 */
void CopyPolygonsToAperture(const CArray<XYPolygon>& source, aperture::ShapeCollection& target);

/**
 * @brief Copy ellipses from aperture::ShapeCollection to CArrayXYEllipse
 * 
 * Extracts ellipse shapes from ShapeCollection and creates XYEllipse array.
 * Only EXTERNAL and INTERNAL type shapes are extracted (not APERTURE).
 * Target array is cleared before appending.
 * 
 * @param source aperture::ShapeCollection to extract from
 * @param target MFC array to populate with XYEllipse shapes
 */
void CopyApertureToEllipses(const aperture::ShapeCollection& source, CArray<XYEllipse>& target);

/**
 * @brief Copy rectangles from aperture::ShapeCollection to CArrayXYRect
 * 
 * Extracts rectangle shapes from ShapeCollection and creates XYRect array.
 * Note: aperture::Rectangle stores full widths, while XYRect expects Ax/By (half-widths).
 * Divides by 2.0 during conversion.
 * Only EXTERNAL and INTERNAL type shapes are extracted (not APERTURE).
 * Target array is cleared before appending.
 * 
 * @param source aperture::ShapeCollection to extract from
 * @param target MFC array to populate with XYRect shapes
 */
void CopyApertureToRects(const aperture::ShapeCollection& source, CArray<XYRect>& target);

/**
 * @brief Copy polygons from aperture::ShapeCollection to CArrayXYPolygon
 * 
 * Extracts polygon shapes from ShapeCollection and creates XYPolygon array.
 * Only EXTERNAL and INTERNAL type shapes are extracted (not APERTURE).
 * Target array is cleared before appending.
 * 
 * @param source aperture::ShapeCollection to extract from
 * @param target MFC array to populate with XYPolygon shapes
 */
void CopyApertureToPolygons(const aperture::ShapeCollection& source, CArray<XYPolygon>& target);

} // namespace DigitMode
