/**
 * @file Polygon.h
 * @brief Arbitrary polygon shape
 */
#pragma once

#include "Shape.h"
#include <vector>

namespace aperture {

/**
 * @brief Polygon defined by arbitrary vertices
 * 
 * Represents a closed polygon with any number of vertices.
 * Supports convex and non-convex polygons. Uses ray-casting
 * algorithm for point-in-polygon testing.
 * 
 * Replaces XYPolygon with modern C++ design.
 */
class Polygon : public Shape {
public:
    /**
     * @brief Construct empty polygon
     */
    Polygon() = default;
    
    /**
     * @brief Construct from vertex list
     * @param vertices Vector of vertices (will be closed automatically)
     */
    explicit Polygon(const std::vector<Point>& vertices);
    
    /**
     * @brief Construct from initializer list
     */
    Polygon(std::initializer_list<Point> vertices);
    
    // Shape interface implementation
    
    bool isInside(const Point& point) const override;
    Bounds getBounds() const override;
    std::vector<Point> getContour(double stepSize) const override;
    double perimeter() const override;
    double area() const override;
    std::unique_ptr<Shape> clone() const override;
    const char* typeName() const override { return "Polygon"; }
    
    // Polygon-specific operations
    
    /**
     * @brief Add a vertex to the polygon
     * @param point Vertex to add
     */
    void addVertex(const Point& point);
    
    /**
     * @brief Get number of vertices
     */
    size_t vertexCount() const { return vertices_.size(); }
    
    /**
     * @brief Get vertex at index
     */
    const Point& vertex(size_t index) const { return vertices_[index]; }
    
    /**
     * @brief Get all vertices
     */
    const std::vector<Point>& vertices() const { return vertices_; }
    
    /**
     * @brief Clear all vertices
     */
    void clear() { vertices_.clear(); }
    
    /**
     * @brief Check if polygon is closed
     * @param tolerance Maximum distance between first and last vertex
     */
    bool isClosed(double tolerance = 1e-6) const;
    
    /**
     * @brief Check if polygon is degenerate (< 3 vertices or zero area)
     */
    bool isDegenerate(double tolerance = 1e-6) const;
    
    /**
     * @brief Check if polygon is convex
     */
    bool isConvex() const;
    
    /**
     * @brief Get centroid (geometric center)
     */
    Point centroid() const;
    
    /**
     * @brief Close the polygon if not already closed
     */
    void ensureClosed(double tolerance = 1e-6);
    
    // Coordinate transformation interface implementation
    
    void normalize(double originX, double originY, double radius) override;
    void denormalize(double originX, double originY, double radius) override;
    void inverseY(double centerY) override;
    void shiftX(double deltaX) override;
    void shiftY(double deltaY) override;

private:
    std::vector<Point> vertices_;  ///< Polygon vertices
    
    /**
     * @brief Calculate signed area (positive for counter-clockwise)
     */
    double signedArea() const;
};

} // namespace aperture
