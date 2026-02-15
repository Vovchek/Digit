/**
 * @file Ellipse.cpp
 * @brief Implementation of Ellipse class
 */

#include "aperturecore/geometry/Ellipse.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace aperture {

Ellipse::Ellipse(double semiMajorAxis, double semiMinorAxis,
                 double centerX, double centerY,
                 double rotationDegrees,
                 TypeLimits typeLimits,
                 CoordinateSystem spatialSystem,
                 NormalizationState normState)
    : semiMajor_(semiMajorAxis)
    , semiMinor_(semiMinorAxis)
    , center_(centerX, centerY)
    , rotationDeg_(rotationDegrees)
    , rotationRad_(rotationDegrees * M_PI / 180.0)
    , cosRot_(0.0)
    , sinRot_(0.0)
{
    typeLimits_ = typeLimits;
    spatialSystem_ = spatialSystem;
    normState_ = normState;
    updateRotationCache();
}

void Ellipse::updateRotationCache() {
    cosRot_ = std::cos(rotationRad_);
    sinRot_ = std::sin(rotationRad_);
}

Point Ellipse::toLocalCoordinates(const Point& point) const {
    // Translate to origin
    double dx = point.x - center_.x;
    double dy = point.y - center_.y;
    
    // Rotate by -rotation to align with axes
    return Point{
        dx * cosRot_ + dy * sinRot_,
        -dx * sinRot_ + dy * cosRot_
    };
}

Point Ellipse::toWorldCoordinates(const Point& point) const {
    // Rotate by +rotation
    double x = point.x * cosRot_ - point.y * sinRot_;
    double y = point.x * sinRot_ + point.y * cosRot_;
    
    // Translate to center
    return Point{x + center_.x, y + center_.y};
}

bool Ellipse::isInside(const Point& point) const {
    // Transform to ellipse-local coordinates
    Point local = toLocalCoordinates(point);
    
    // Check ellipse equation: (x/a)^2 + (y/b)^2 <= 1
    double term1 = (local.x * local.x) / (semiMajor_ * semiMajor_);
    double term2 = (local.y * local.y) / (semiMinor_ * semiMinor_);
    
    return (term1 + term2) <= 1.0;
}

Bounds Ellipse::getBounds() const {
    if (std::abs(rotationDeg_) < 1e-6) {
        // No rotation - simple case
        return Bounds{
            center_.x - semiMajor_,
            center_.y - semiMinor_,
            center_.x + semiMajor_,
            center_.y + semiMinor_
        };
    }
    
    // For rotated ellipse, find the extreme points
    // The bounding box vertices are where dx/dt = 0 and dy/dt = 0
    // in the parametric equations:
    // x(t) = cx + a*cos(t)*cos(r) - b*sin(t)*sin(r)
    // y(t) = cy + a*cos(t)*sin(r) + b*sin(t)*cos(r)
    
    double a2_cos2 = semiMajor_ * semiMajor_ * cosRot_ * cosRot_;
    double b2_sin2 = semiMinor_ * semiMinor_ * sinRot_ * sinRot_;
    double a2_sin2 = semiMajor_ * semiMajor_ * sinRot_ * sinRot_;
    double b2_cos2 = semiMinor_ * semiMinor_ * cosRot_ * cosRot_;
    
    double halfWidth = std::sqrt(a2_cos2 + b2_sin2);
    double halfHeight = std::sqrt(a2_sin2 + b2_cos2);
    
    return Bounds{
        center_.x - halfWidth,
        center_.y - halfHeight,
        center_.x + halfWidth,
        center_.y + halfHeight
    };
}

std::vector<Point> Ellipse::getContour(double stepSize) const {
    // Calculate number of points based on perimeter and step size
    double perim = perimeter();
    int numPoints = std::max(8, static_cast<int>(perim / stepSize));
    
    std::vector<Point> contour;
    contour.reserve(numPoints + 1);  // +1 for closing point
    
    // Generate points parametrically
    double angleStep = 2.0 * M_PI / numPoints;
    
    for (int i = 0; i <= numPoints; ++i) {
        double t = i * angleStep;
        
        // Parametric ellipse in local coordinates
        Point local{
            semiMajor_ * std::cos(t),
            semiMinor_ * std::sin(t)
        };
        
        // Transform to world coordinates
        contour.push_back(toWorldCoordinates(local));
    }
    
    return contour;
}

double Ellipse::perimeter() const {
    // Use Ramanujan's approximation for ellipse perimeter
    // P ? ? * (3(a + b) - sqrt((3a + b)(a + 3b)))
    // This is accurate to within 0.01% for most ellipses
    
    double a = semiMajor_;
    double b = semiMinor_;
    
    if (isCircle()) {
        // Exact for circles
        return 2.0 * M_PI * a;
    }
    
    // Ramanujan's second approximation
    double h = ((a - b) * (a - b)) / ((a + b) * (a + b));
    return M_PI * (a + b) * (1.0 + (3.0 * h) / (10.0 + std::sqrt(4.0 - 3.0 * h)));
}

double Ellipse::area() const {
    return M_PI * semiMajor_ * semiMinor_;
}

double Ellipse::eccentricity() const {
    if (semiMajor_ < semiMinor_) {
        // b > a, swap for calculation
        double e2 = 1.0 - (semiMajor_ * semiMajor_) / (semiMinor_ * semiMinor_);
        return std::sqrt(std::max(0.0, e2));
    }
    
    double e2 = 1.0 - (semiMinor_ * semiMinor_) / (semiMajor_ * semiMajor_);
    return std::sqrt(std::max(0.0, e2));
}

double Ellipse::focalDistance() const {
    if (isCircle()) {
        return 0.0;
    }
    
    double a = std::max(semiMajor_, semiMinor_);
    double b = std::min(semiMajor_, semiMinor_);
    
    return std::sqrt(a * a - b * b);
}

std::unique_ptr<Shape> Ellipse::clone() const {
    return std::make_unique<Ellipse>(*this);
}

void Ellipse::normalize(double originX, double originY, double radius) {
    semiMajor_ /= radius;
    semiMinor_ /= radius;
    center_.x = (center_.x - originX) / radius;
    center_.y = (center_.y - originY) / radius;
    normState_ = NormalizationState::NORMALIZED;
}

void Ellipse::denormalize(double originX, double originY, double radius) {
    semiMajor_ *= radius;
    semiMinor_ *= radius;
    center_.x = center_.x * radius + originX;
    center_.y = center_.y * radius + originY;
    normState_ = NormalizationState::MEASURING;
}

void Ellipse::inverseY(double centerY) {
    center_.y = centerY - center_.y;
    rotationDeg_ = -rotationDeg_;
    rotationRad_ = -rotationRad_;
    updateRotationCache();
}

void Ellipse::shiftX(double deltaX) {
    center_.x += deltaX;
}

void Ellipse::shiftY(double deltaY) {
    center_.y += deltaY;
}

//===========================================================================
// Ellipse fitting constructor implementation
//===========================================================================

namespace {
    // Helper: Solve 5x5 linear system using Gaussian elimination
    // Returns true if solution found, false if singular
    bool solveLinearSystem5x5(double A[5][5], double b[5], double x[5]) {
        constexpr double EPSILON = 1e-10;
        
        // Create augmented matrix
        double aug[5][6];
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                aug[i][j] = A[i][j];
            }
            aug[i][5] = b[i];
        }
        
        // Forward elimination with partial pivoting
        for (int k = 0; k < 5; k++) {
            // Find pivot
            int maxRow = k;
            double maxVal = std::abs(aug[k][k]);
            for (int i = k + 1; i < 5; i++) {
                double val = std::abs(aug[i][k]);
                if (val > maxVal) {
                    maxVal = val;
                    maxRow = i;
                }
            }
            
            if (maxVal < EPSILON) {
                return false;  // Singular matrix
            }
            
            // Swap rows
            if (maxRow != k) {
                for (int j = 0; j < 6; j++) {
                    std::swap(aug[k][j], aug[maxRow][j]);
                }
            }
            
            // Eliminate
            for (int i = k + 1; i < 5; i++) {
                double factor = aug[i][k] / aug[k][k];
                for (int j = k; j < 6; j++) {
                    aug[i][j] -= factor * aug[k][j];
                }
            }
        }
        
        // Back substitution
        for (int i = 4; i >= 0; i--) {
            double sum = aug[i][5];
            for (int j = i + 1; j < 5; j++) {
                sum -= aug[i][j] * x[j];
            }
            x[i] = sum / aug[i][i];
        }
        
        return true;
    }
    
    // Helper: Convert conic coefficients to ellipse parameters
    // Conic: ax² + bxy + cy² + dx + ey + f = 0
    // Returns false if not an ellipse
    bool conicToEllipse(double a, double b, double c, double d, double e, double f,
                       double& centerX, double& centerY,
                       double& semiMajor, double& semiMinor, double& rotationDeg) {
        constexpr double EPSILON = 1e-10;
        
        // Check if it's an ellipse: b²-4ac < 0
        double discriminant = b * b - 4.0 * a * c;
        if (discriminant >= -EPSILON) {
            return false;  // Not an ellipse (parabola or hyperbola)
        }
        
        // Center calculation
        // From conic Ax² + Bxy + Cy² + Dx + Ey + F = 0
        // Completing the square gives:
        // (x - xc)²/a² + (y - yc)²/b² = 1 (for rotated ellipse)
        // Center: xc = (2CD - BE) / (B² - 4AC), yc = (2AE - BD) / (B² - 4AC)
        double denominator = b * b - 4.0 * a * c;
        if (std::abs(denominator) < EPSILON) {
            return false;
        }
        
        centerX = (2.0 * c * d - b * e) / denominator;
        centerY = (2.0 * a * e - b * d) / denominator;
        
        // Semi-axes calculation
        // Compute the numerator for the axis lengths
        // num = 2(Ae² + Cd² + Fb² - Bde - ACf) / (B² - 4AC)
        double numerator = 2.0 * (a * e * e + c * d * d + f * b * b - b * d * e - 4.0 * a * c * f);
        
        // The denominators involve eigenvalues
        double sqrtTerm = std::sqrt((a - c) * (a - c) + b * b);
        double denom1 = (b * b - 4.0 * a * c) * (sqrtTerm - (a + c));
        double denom2 = (b * b - 4.0 * a * c) * (-sqrtTerm - (a + c));
        
        if (denom1 >= -EPSILON || denom2 >= -EPSILON) {
            return false;  // Degenerate
        }
        
        double axis1 = std::sqrt(std::abs(numerator / denom1));
        double axis2 = std::sqrt(std::abs(numerator / denom2));
        
        semiMajor = std::max(axis1, axis2);
        semiMinor = std::min(axis1, axis2);
        
        // Rotation angle
        // tan(2θ) = B / (A - C)
        if (std::abs(b) < EPSILON) {
            rotationDeg = 0.0;
        } else {
            double angleRad = 0.5 * std::atan2(b, a - c);
            rotationDeg = angleRad * 180.0 / M_PI;
        }
        
        return true;
    }
}

Ellipse::Ellipse(const std::vector<Point>& points,
                TypeLimits typeLimits,
                CoordinateSystem spatialSystem,
                NormalizationState normState)
    : semiMajor_(0.0)
    , semiMinor_(0.0)
    , center_(0.0, 0.0)
    , rotationDeg_(0.0)
    , rotationRad_(0.0)
    , cosRot_(1.0)
    , sinRot_(0.0)
{
    typeLimits_ = typeLimits;
    spatialSystem_ = spatialSystem;
    normState_ = normState;
    
    constexpr double EPSILON = 1e-10;
    const size_t n = points.size();
    
    if (n == 0) {
        // Empty - leave at origin with zero radii
        return;
    }
    
    if (n == 1) {
        // Single point - degenerate ellipse
        center_ = points[0];
        return;
    }
    
    if (n == 2) {
        // Two points define circle diameter
        center_.x = (points[0].x + points[1].x) / 2.0;
        center_.y = (points[0].y + points[1].y) / 2.0;
        double radius = points[0].distanceTo(points[1]) / 2.0;
        semiMajor_ = radius;
        semiMinor_ = radius;
        return;
    }
    
    if (n == 3) {
        // Three points define a circle (geometric fit)
        double x1 = points[0].x, y1 = points[0].y;
        double x2 = points[1].x, y2 = points[1].y;
        double x3 = points[2].x, y3 = points[2].y;
        
        double A = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
        
        if (std::abs(A) < EPSILON) {
            // Collinear points - use centroid
            center_.x = (x1 + x2 + x3) / 3.0;
            center_.y = (y1 + y2 + y3) / 3.0;
            return;
        }
        
        double B = (x1 * x1 + y1 * y1) * (y3 - y2) +
                   (x2 * x2 + y2 * y2) * (y1 - y3) +
                   (x3 * x3 + y3 * y3) * (y2 - y1);
        double C = (x1 * x1 + y1 * y1) * (x2 - x3) +
                   (x2 * x2 + y2 * y2) * (x3 - x1) +
                   (x3 * x3 + y3 * y3) * (x1 - x2);
        
        center_.x = -B / (2.0 * A);
        center_.y = -C / (2.0 * A);
        
        double radius = std::sqrt((x1 - center_.x) * (x1 - center_.x) +
                                 (y1 - center_.y) * (y1 - center_.y));
        semiMajor_ = radius;
        semiMinor_ = radius;
        return;
    }
    
    if (n == 4) {
        // Four points - axis-aligned ellipse (bounding box approach)
        center_.x = (points[0].x + points[1].x + points[2].x + points[3].x) / 4.0;
        center_.y = (points[0].y + points[1].y + points[2].y + points[3].y) / 4.0;
        
        double maxX = 0.0, maxY = 0.0;
        for (size_t i = 0; i < 4; i++) {
            double dx = std::abs(points[i].x - center_.x);
            double dy = std::abs(points[i].y - center_.y);
            maxX = std::max(maxX, dx);
            maxY = std::max(maxY, dy);
        }
        
        semiMajor_ = maxX;
        semiMinor_ = maxY;
        return;
    }
    
    if (n == 5) {
        // Five points - exact ellipse fit (general conic)
        // Solve: Ax² + Bxy + Cy² + Dx + Ey + F = 0 with F = 1
        
        double A[5][5];
        double b[5];
        
        for (size_t i = 0; i < 5; i++) {
            double x = points[i].x;
            double y = points[i].y;
            A[i][0] = x * x;
            A[i][1] = x * y;
            A[i][2] = y * y;
            A[i][3] = x;
            A[i][4] = y;
            b[i] = 1.0;
        }
        
        double solution[5];
        if (solveLinearSystem5x5(A, b, solution)) {
            double a = solution[0];
            double bxy = solution[1];
            double c = solution[2];
            double d = solution[3];
            double e = solution[4];
            double f = -1.0;  // We normalized with F = 1
            
            if (conicToEllipse(a, bxy, c, d, e, f,
                             center_.x, center_.y,
                             semiMajor_, semiMinor_, rotationDeg_)) {
                rotationRad_ = rotationDeg_ * M_PI / 180.0;
                updateRotationCache();
                return;
            }
        }
        
        // Fallback to 4-point method
        std::vector<Point> fourPoints(points.begin(), points.begin() + 4);
        *this = Ellipse(fourPoints, typeLimits, spatialSystem, normState);
        return;
    }
    
    // n > 5: Least squares ellipse fit
    // Build design matrix D and scatter matrix S = D'*D
    
    double S[5][5] = {{0}};  // Scatter matrix (symmetric)
    
    // S = D'*D where D = [x² xy y² x y]
    for (size_t k = 0; k < n; k++) {
        double x = points[k].x;
        double y = points[k].y;
        double x2 = x * x;
        double xy = x * y;
        double y2 = y * y;
        
        double row[5] = {x2, xy, y2, x, y};
        
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                S[i][j] += row[i] * row[j];
            }
        }
    }
    
    // Right-hand side: D' * (-ones(n,1))
    // We're solving D'*D*coeff = -D'*ones
    // because conic equation is: Ax² + Bxy + Cy² + Dx + Ey + F = 0
    // with F = -1 (normalization), so: Ax² + Bxy + Cy² + Dx + Ey = 1
    double rhs[5] = {0};
    for (size_t k = 0; k < n; k++) {
        double x = points[k].x;
        double y = points[k].y;
        double x2 = x * x;
        double xy = x * y;
        double y2 = y * y;
        
        // Each row of D contributes to rhs: D'*ones = sum of each column
        rhs[0] -= x2;  // Negative because we solve for Ax²+...=-F with F=-1
        rhs[1] -= xy;
        rhs[2] -= y2;
        rhs[3] -= x;
        rhs[4] -= y;
    }
    
    // Solve S * solution = rhs
    double solution[5];
    if (solveLinearSystem5x5(S, rhs, solution)) {
        double a = solution[0];
        double bxy = solution[1];
        double c = solution[2];
        double d = solution[3];
        double e = solution[4];
        double f = -1.0;
        
        if (conicToEllipse(a, bxy, c, d, e, f,
                         center_.x, center_.y,
                         semiMajor_, semiMinor_, rotationDeg_)) {
            rotationRad_ = rotationDeg_ * M_PI / 180.0;
            updateRotationCache();
            return;
        }
    }
    
    // Final fallback to 4-point method
    std::vector<Point> fourPoints(points.begin(), points.begin() + 4);
    *this = Ellipse(fourPoints, typeLimits, spatialSystem, normState);
}

// ========================================================================
// Handle Enumeration (Interactive Editing - UX spec §4)
// ========================================================================

void Ellipse::EnumerateHandles(std::vector<HandleDesc>& out) const {
    // §2.1, §4.2 - Move handle at center
    out.push_back(HandleDesc{HandleType::Move, -1, center_});
    
    // §2.2, §4.3 - Rotation handle offset along major axis normal
    // Offset along perpendicular to major axis (minor axis direction)
    double boundsRadius = std::sqrt(semiMajor_ * semiMajor_ + semiMinor_ * semiMinor_);
    double rotHandleOffset = std::max(boundsRadius * 0.2, 20.0);
    
    // Minor axis direction (perpendicular to major axis)
    Point rotHandlePos{
        center_.x - rotHandleOffset * sinRot_,  // -sin for +Y rotation
        center_.y + rotHandleOffset * cosRot_   // +cos for +Y rotation
    };
    out.push_back(HandleDesc{HandleType::Rotate, -1, rotHandlePos});
    
    // §4.1 - 4 Axis resize handles (2 major + 2 minor)
    
    // Major axis endpoints (index 0, 1)
    Point majorEnd1{
        center_.x + semiMajor_ * cosRot_,
        center_.y + semiMajor_ * sinRot_
    };
    Point majorEnd2{
        center_.x - semiMajor_ * cosRot_,
        center_.y - semiMajor_ * sinRot_
    };
    
    // Normal for major axis = direction of major axis
    Point majorNormal{cosRot_, sinRot_};
    
    out.push_back(HandleDesc{HandleType::AxisResize, 0, majorEnd1, majorNormal});
    out.push_back(HandleDesc{HandleType::AxisResize, 1, majorEnd2, Point{-majorNormal.x, -majorNormal.y}});
    
    // Minor axis endpoints (index 2, 3)
    Point minorEnd1{
        center_.x - semiMinor_ * sinRot_,
        center_.y + semiMinor_ * cosRot_
    };
    Point minorEnd2{
        center_.x + semiMinor_ * sinRot_,
        center_.y - semiMinor_ * cosRot_
    };
    
    // Normal for minor axis = direction perpendicular to major
    Point minorNormal{-sinRot_, cosRot_};
    
    out.push_back(HandleDesc{HandleType::AxisResize, 2, minorEnd1, minorNormal});
    out.push_back(HandleDesc{HandleType::AxisResize, 3, minorEnd2, Point{-minorNormal.x, -minorNormal.y}});
}

void Ellipse::ApplyHandleDrag(const HandleDesc& handle, const DragContext& drag) {
    switch (handle.type) {
        case HandleType::Move:
            // Simple translation
            center_.x += drag.deltaWorld.x;
            center_.y += drag.deltaWorld.y;
            break;
            
        case HandleType::Rotate: {
            // Calculate angle from center to current drag position
            double dx = drag.dragCurrentWorld.x - center_.x;
            double dy = drag.dragCurrentWorld.y - center_.y;
            double newAngle = std::atan2(dy, dx);
            
            // Initial angle from center to drag start
            double dx0 = drag.dragStartWorld.x - center_.x;
            double dy0 = drag.dragStartWorld.y - center_.y;
            double initialAngle = std::atan2(dy0, dx0);
            
            // Apply rotation delta
            double deltaAngle = newAngle - initialAngle;
            rotationRad_ += deltaAngle;
            rotationDeg_ = rotationRad_ * 180.0 / M_PI;
            
            // Normalize to [0, 360)
            while (rotationDeg_ < 0.0) rotationDeg_ += 360.0;
            while (rotationDeg_ >= 360.0) rotationDeg_ -= 360.0;
            
            updateRotationCache();
            break;
        }
            
        case HandleType::AxisResize: {
            // Determine which axis is being dragged
            bool isMajorAxis = (handle.index == 0 || handle.index == 1);
            
            // Project drag delta onto handle normal to get resize amount
            double resize = drag.deltaWorld.x * handle.normal.x +
                           drag.deltaWorld.y * handle.normal.y;
            
            if (isMajorAxis) {
                semiMajor_ += resize;
                semiMajor_ = std::max(semiMajor_, 1.0);  // Minimum size
            } else {
                semiMinor_ += resize;
                semiMinor_ = std::max(semiMinor_, 1.0);  // Minimum size
            }
            break;
        }
            
        default:
            break;
    }
}

// ========================================================================
// Static Factory Methods (Phase 1 - Explicit Circle/Ellipse Fitting)
// ========================================================================

// Solve for circle parameters using algebraic method
// (x² + y²) + Ax + By + C = 0
// Center = (-A/2, -B/2), Radius = sqrt((A² + B²)/4 - C)

std::unique_ptr<Ellipse> Ellipse::FitCircle(
    const std::vector<Point>& points,
    TypeLimits typeLimits,
    CoordinateSystem spatialSystem,
    NormalizationState normState)
{
    if (points.size() < 3) {
        // Degenerate case: return zero-radius circle at origin
        return std::make_unique<Ellipse>(0.0, 0.0, 0.0, 0.0, 0.0,
            typeLimits, spatialSystem, normState);
    }

    size_t n = points.size();

    // Build linear system: [x²+y², x, y, 1] * [1, A, B, C]ᵀ = 0
    // Using D = -(x²+y²) to move to right-hand side

    double sum_x = 0, sum_y = 0, sum_x2 = 0, sum_y2 = 0;
    double sum_xy = 0, sum_x3 = 0, sum_y3 = 0, sum_xy2 = 0, sum_x2y = 0;
    double sum_x2_y2 = 0.0;  // sum of (x² + y²)

    for (const auto& p : points) {
        double x = p.x;
        double y = p.y;
        double x2 = x * x;
        double y2 = y * y;
        double xy = x * y;
        double x2_y2 = x2 + y2;

        sum_x += x;
        sum_y += y;
        sum_x2 += x2;
        sum_y2 += y2;
        sum_xy += xy;
        sum_x3 += x2 * x;
        sum_y3 += y2 * y;
        sum_xy2 += x * y2;
        sum_x2y += x2 * y;
        sum_x2_y2 += x2_y2;
    }

    // Build the normal matrix (3x3 symmetric)
    double M11 = sum_x2;
    double M12 = sum_xy;
    double M13 = sum_x;
    double M22 = sum_y2;
    double M23 = sum_y;
    double M33 = static_cast<double>(n);

    // Build the right-hand side
    double b1 = -(sum_x3 + sum_xy2);
    double b2 = -(sum_x2y + sum_y3);
    double b3 = -sum_x2_y2;

    // Solve the 3x3 system M * [A, B, C]ᵀ = b
    // Using Cramer's rule or a small solver. Here we use a simple direct method.
    double det = M11 * (M22 * M33 - M23 * M23)
        - M12 * (M12 * M33 - M23 * M13)
        + M13 * (M12 * M23 - M22 * M13);

    if (std::abs(det) < 1e-10) {
        // Points are collinear or degenerate
        return std::make_unique<Ellipse>(0.0, 0.0, 0.0, 0.0, 0.0,
            typeLimits, spatialSystem, normState);
    }

    double invDet = 1.0 / det;

    double A = (b1 * (M22 * M33 - M23 * M23)
        - M12 * (b2 * M33 - M23 * b3)
        + M13 * (b2 * M23 - M22 * b3)) * invDet;

    double B = (M11 * (b2 * M33 - M23 * b3)
        - b1 * (M12 * M33 - M23 * M13)
        + M13 * (M12 * b3 - b2 * M13)) * invDet;

    double C = (M11 * (M22 * b3 - M23 * b2)
        - M12 * (M12 * b3 - M13 * b2)
        + b1 * (M12 * M23 - M22 * M13)) * invDet;

    double cx = -A / 2.0;
    double cy = -B / 2.0;
    double radius = std::sqrt(cx * cx + cy * cy - C);

    // Ensure radius is non‑negative (should be, but guard against numerical issues)
    if (radius < 0.0) radius = 0.0;

    return std::make_unique<Ellipse>(
        radius, radius, cx, cy, 0.0,
        typeLimits, spatialSystem, normState
    );
}

std::unique_ptr<Ellipse> Ellipse::FitEllipse(
    const std::vector<Point>& points,
    TypeLimits typeLimits,
    CoordinateSystem spatialSystem,
    NormalizationState normState)
{
    // Delegate to existing Ellipse(vector<Point>) constructor
    // This static method exists for API clarity and consistency with FitCircle
    return std::make_unique<Ellipse>(points, typeLimits, spatialSystem, normState);
}

} // namespace aperture
