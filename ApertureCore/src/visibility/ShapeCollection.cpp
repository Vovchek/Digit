/**
 * @file ShapeCollection.cpp
 * @brief Implementation of ShapeCollection class
 */

#include "aperturecore/visibility/ShapeCollection.h"
#include <algorithm>

namespace aperture {

void ShapeCollection::addShape(std::unique_ptr<Shape> shape) {
    if (!shape) return;
    
    TypeLimits type = shape->getTypeLimits();
    
    switch (type) {
        case TypeLimits::EXTERNAL:
            external_.push_back(std::move(shape));
            break;
        case TypeLimits::INTERNAL:
            internal_.push_back(std::move(shape));
            break;
        case TypeLimits::APERTURE:
            apertures_.push_back(std::move(shape));
            break;
    }
}

void ShapeCollection::addExternal(std::unique_ptr<Shape> shape) {
    if (!shape) return;
    shape->setTypeLimits(TypeLimits::EXTERNAL);
    external_.push_back(std::move(shape));
}

void ShapeCollection::addInternal(std::unique_ptr<Shape> shape) {
    if (!shape) return;
    shape->setTypeLimits(TypeLimits::INTERNAL);
    internal_.push_back(std::move(shape));
}

void ShapeCollection::addAperture(std::unique_ptr<Shape> shape) {
    if (!shape) return;
    shape->setTypeLimits(TypeLimits::APERTURE);
    apertures_.push_back(std::move(shape));
}

Bounds ShapeCollection::getCombinedBounds() const {
    if (isEmpty()) {
        return Bounds{};
    }
    
    Bounds combined = Bounds::infinite();
    bool hasAny = false;
    
    // Process all EXTERNAL shapes
    for (const auto& shape : external_) {
        if (hasAny) {
            combined.merge(shape->getBounds());
        } else {
            combined = shape->getBounds();
            hasAny = true;
        }
    }
    
    // Process all INTERNAL shapes
    for (const auto& shape : internal_) {
        if (hasAny) {
            combined.merge(shape->getBounds());
        } else {
            combined = shape->getBounds();
            hasAny = true;
        }
    }
    
    // Process all APERTURE shapes
    for (const auto& shape : apertures_) {
        if (hasAny) {
            combined.merge(shape->getBounds());
        } else {
            combined = shape->getBounds();
            hasAny = true;
        }
    }
    
    return combined;
}

void ShapeCollection::clear() {
    external_.clear();
    internal_.clear();
    apertures_.clear();
}

size_t ShapeCollection::countByType(TypeLimits type) const {
    switch (type) {
        case TypeLimits::EXTERNAL:
            return external_.size();
        case TypeLimits::INTERNAL:
            return internal_.size();
        case TypeLimits::APERTURE:
            return apertures_.size();
        default:
            return 0;
    }
}

} // namespace aperture
