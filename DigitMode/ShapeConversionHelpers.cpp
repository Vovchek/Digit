/**
 * @file ShapeConversionHelpers.cpp
 * @brief Conversion helper implementations
 */

#include "ShapeConversionHelpers.h"
#include "AppDef.h"

namespace DigitMode {

void CopyEllipsesToAperture(const CArray<XYEllipse>& source, aperture::ShapeCollection& target)
{
	for (int i = 0; i < source.GetSize(); ++i) {
		const auto& shape = source[i];
		auto ellipse = std::make_unique<aperture::Ellipse>(
			shape.Ax, shape.By, shape.Xc, shape.Yc, shape.Fi);
		
		if (shape.TypeLimits == S_EXTERNAL) {
			target.addAperture(std::move(ellipse));
		} else if (shape.TypeLimits == S_INTERNAL) {
			target.addInternal(std::move(ellipse));
		}
	}
}

void CopyRectsToAperture(const CArray<XYRect>& source, aperture::ShapeCollection& target)
{
	for (int i = 0; i < source.GetSize(); ++i) {
		const auto& shape = source[i];
		auto rect = std::make_unique<aperture::Rectangle>(
			2.0 * shape.Ax, 2.0 * shape.By, shape.Xc, shape.Yc, shape.Fi);
		
		if (shape.TypeLimits == S_EXTERNAL) {
			target.addAperture(std::move(rect));
		} else if (shape.TypeLimits == S_INTERNAL) {
			target.addInternal(std::move(rect));
		}
	}
}

void CopyPolygonsToAperture(const CArray<XYPolygon>& source, aperture::ShapeCollection& target)
{
	for (int i = 0; i < source.GetSize(); ++i) {
		const auto& shape = source[i];
		std::vector<aperture::Point> vertices;
		
		for (int iV = 0; iV < shape.GetSize(); ++iV) {
			vertices.push_back({ shape[iV].X, shape[iV].Y });
		}
		
		auto polygon = std::make_unique<aperture::Polygon>(vertices);
		
		if (shape.TypeLimits == S_EXTERNAL) {
			target.addAperture(std::move(polygon));
		} else if (shape.TypeLimits == S_INTERNAL) {
			target.addInternal(std::move(polygon));
		}
	}
}

void CopyApertureToEllipses(const aperture::ShapeCollection& source, CArray<XYEllipse>& target)
{
	target.RemoveAll();
	
	// Iterate through external shapes
	for (const auto& shapePtr : source.getApertures()) {
		const aperture::Ellipse* ellipse = dynamic_cast<const aperture::Ellipse*>(shapePtr.get());
		if (ellipse != nullptr) {
			XYEllipse xyEllipse(
				ellipse->semiMajor(),
				ellipse->semiMinor(),
				ellipse->center().x,
				ellipse->center().y,
				ellipse->rotationDegrees(),
				S_EXTERNAL
			);
			target.Add(xyEllipse);
		}
	}
	
	// Iterate through internal shapes
	for (const auto& shapePtr : source.getInternal()) {
		const aperture::Ellipse* ellipse = dynamic_cast<const aperture::Ellipse*>(shapePtr.get());
		if (ellipse != nullptr) {
			XYEllipse xyEllipse(
				ellipse->semiMajor(),
				ellipse->semiMinor(),
				ellipse->center().x,
				ellipse->center().y,
				ellipse->rotationDegrees(),
				S_INTERNAL
			);
			target.Add(xyEllipse);
		}
	}
}

void CopyApertureToRects(const aperture::ShapeCollection& source, CArray<XYRect>& target)
{
	target.RemoveAll();
	
	// Iterate through external shapes
	for (const auto& shapePtr : source.getApertures()) {
		const aperture::Rectangle* rect = dynamic_cast<const aperture::Rectangle*>(shapePtr.get());
		if (rect != nullptr) {
			XYRect xyRect(
				rect->width() / 2.0,   // aperture::Rectangle width -> XYRect Ax
				rect->height() / 2.0,  // aperture::Rectangle height -> XYRect By
				rect->center().x,
				rect->center().y,
				rect->rotationDegrees(),
				S_EXTERNAL
			);
			target.Add(xyRect);
		}
	}
	
	// Iterate through internal shapes
	for (const auto& shapePtr : source.getInternal()) {
		const aperture::Rectangle* rect = dynamic_cast<const aperture::Rectangle*>(shapePtr.get());
		if (rect != nullptr) {
			XYRect xyRect(
				rect->width() / 2.0,   // aperture::Rectangle width -> XYRect Ax
				rect->height() / 2.0,  // aperture::Rectangle height -> XYRect By
				rect->center().x,
				rect->center().y,
				rect->rotationDegrees(),
				S_INTERNAL
			);
			target.Add(xyRect);
		}
	}
}

void CopyApertureToPolygons(const aperture::ShapeCollection& source, CArray<XYPolygon>& target)
{
	target.RemoveAll();
	
	// Iterate through external shapes
	for (const auto& shapePtr : source.getApertures()) {
		const aperture::Polygon* polygon = dynamic_cast<const aperture::Polygon*>(shapePtr.get());
		if (polygon != nullptr) {
			XYPolygon xyPolygon;
			
			// Copy vertices
			const auto& vertices = polygon->vertices();
			for (const auto& vertex : vertices) {
				xyPolygon.Add(XYPoint(vertex.x, vertex.y));
			}
			
			// Set type limits
			xyPolygon.SetTypeLimits(S_EXTERNAL);
			
			target.Add(xyPolygon);
		}
	}
	
	// Iterate through internal shapes
	for (const auto& shapePtr : source.getInternal()) {
		const aperture::Polygon* polygon = dynamic_cast<const aperture::Polygon*>(shapePtr.get());
		if (polygon != nullptr) {
			XYPolygon xyPolygon;
			
			// Copy vertices
			const auto& vertices = polygon->vertices();
			for (const auto& vertex : vertices) {
				xyPolygon.Add(XYPoint(vertex.x, vertex.y));
			}
			
			// Set type limits
			xyPolygon.SetTypeLimits(S_INTERNAL);
			
			target.Add(xyPolygon);
		}
	}
}

} // namespace DigitMode
