/**
 * @file BoundingCircle.cpp
 * @brief Implementation of bounding circle computation
 * @author Vladimir N. Chekal
 * @see https://github.com/Vovchek
 */

#include "aperturecore/geometry/BoundingCircle.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <random>

namespace aperture {

namespace {
    // Tolerance for floating point comparisons
    constexpr double EPSILON = 1e-10;

    // Check if point is inside circle
    bool isInsideCircle(const Point& p, const Point& center, double radius) {
        double dx = p.x - center.x;
        double dy = p.y - center.y;
        double distSq = dx * dx + dy * dy;
        return distSq <= radius * radius + EPSILON;
    }

    // Circle from two points (diameter)
    BoundingCircle circleFromTwoPoints(const Point& a, const Point& b) {
        Point center = (a + b) * 0.5;
        double radius = a.distanceTo(b) * 0.5;
        return {center, radius, true};
    }

    // Circle from three points
    BoundingCircle circleFromThreePoints(const Point& a, const Point& b, const Point& c) {
        // Handle collinear points
        double ax = a.x, ay = a.y;
        double bx = b.x, by = b.y;
        double cx = c.x, cy = c.y;

        double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
        
        if (std::abs(d) < EPSILON) {
            // Collinear - use largest distance
            double dAB = a.distanceTo(b);
            double dBC = b.distanceTo(c);
            double dAC = a.distanceTo(c);

            if (dAB >= dBC && dAB >= dAC)
                return circleFromTwoPoints(a, b);
            else if (dBC >= dAC)
                return circleFromTwoPoints(b, c);
            else
                return circleFromTwoPoints(a, c);
        }

        double aSq = ax * ax + ay * ay;
        double bSq = bx * bx + by * by;
        double cSq = cx * cx + cy * cy;

        double ux = (aSq * (by - cy) + bSq * (cy - ay) + cSq * (ay - by)) / d;
        double uy = (aSq * (cx - bx) + bSq * (ax - cx) + cSq * (bx - ax)) / d;

        Point center{ux, uy};
        double radius = a.distanceTo(center);
        return {center, radius, true};
    }

    // Welzl's algorithm for minimum bounding circle
    // Points must be pre-shuffled before calling
    // P: point set, R: points on circle boundary, n: current index
    BoundingCircle welzlHelper(const std::vector<Point>& points, std::vector<Point>& boundary, size_t n) {
        if (n == 0u || boundary.size() == 3u) {
            if (boundary.empty()) {
                return {Point{0.0, 0.0}, 0.0, false};
            } else if (boundary.size() == 1u) {
                return {boundary[0], 0.0, true};
            } else if (boundary.size() == 2u) {
                return circleFromTwoPoints(boundary[0], boundary[1]);
            } else {
                return circleFromThreePoints(boundary[0], boundary[1], boundary[2]);
            }
        }

        // Get point at index n-1
        Point p = points[n - 1u];

        // Recurse with n-1 points
        BoundingCircle circle = welzlHelper(points, boundary, n - 1u);

        // If p is outside the circle, add it to boundary and recurse
        if (!circle.valid || !isInsideCircle(p, circle.center, circle.radius)) {
            boundary.push_back(p);
            circle = welzlHelper(points, boundary, n - 1u);
            boundary.pop_back();
        }

        return circle;
    }

} // namespace

BoundingCircle::BoundingCircle(std::vector<Point> points)
{
    std::mt19937 rng(0xD16D1234u);
    std::shuffle(points.begin(), points.end(), rng);

    // Compute minimum bounding circle using Welzl's algorithm
    std::vector<Point> boundary;
    boundary.reserve(3u);
    *this = welzlHelper(points, boundary, points.size());
}

} // namespace aperture
