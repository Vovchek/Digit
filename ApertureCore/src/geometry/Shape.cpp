/**
 * @file Shape.cpp
 * @brief Implementation notes for the Shape base class
 *
 * The Shape base class is an abstract interface: most behavior is implemented
 * by concrete subclasses. Lightweight helper methods that are safe to
 * implement in a header are provided as inline NVI functions. For example,
 * `HandleHit()` is implemented inline in `Shape.h` and uses the pure virtual
 * `EnumerateHandles()` to perform handle hit-testing.
 *
 * This translation unit intentionally contains no out-of-line definitions.
 * Keeping the file present helps documentation tools and provides a place
 * to add implementation details later if necessary.
 */

#include "aperturecore/geometry/Shape.h"

namespace aperture {

// No non-inline symbols to define for Shape at the moment.

} // namespace aperture
