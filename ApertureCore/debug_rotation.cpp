/**
 * Debug test to investigate rotation precision
 */
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Point {
    double x, y;
};

int main() {
    // Test parameters
    double semiMajor = 5.0;
    double semiMinor = 3.0;
    double rotationDeg = 45.0;
    double rotationRad = rotationDeg * M_PI / 180.0;
    
    double cosRot = std::cos(rotationRad);
    double sinRot = std::sin(rotationRad);
    
    // Create point on major axis
    double angle = 45.0 * M_PI / 180.0;
    Point worldPoint{5.0 * std::cos(angle), 5.0 * std::sin(angle)};
    
    std::cout << "World point: (" << worldPoint.x << ", " << worldPoint.y << ")\n";
    
    // Transform to local coordinates (same as Ellipse::toLocalCoordinates)
    double dx = worldPoint.x - 0.0;  // center at origin
    double dy = worldPoint.y - 0.0;
    
    Point local{
        dx * cosRot + dy * sinRot,
        -dx * sinRot + dy * cosRot
    };
    
    std::cout << "Local point: (" << local.x << ", " << local.y << ")\n";
    
    // Check ellipse equation
    double term1 = (local.x * local.x) / (semiMajor * semiMajor);
    double term2 = (local.y * local.y) / (semiMinor * semiMinor);
    double sum = term1 + term2;
    
    std::cout << "term1: " << term1 << "\n";
    std::cout << "term2: " << term2 << "\n";
    std::cout << "sum: " << sum << "\n";
    std::cout << "sum <= 1.0? " << (sum <= 1.0 ? "YES" : "NO") << "\n";
    std::cout << "Difference from 1.0: " << (sum - 1.0) << "\n";
    
    return 0;
}
