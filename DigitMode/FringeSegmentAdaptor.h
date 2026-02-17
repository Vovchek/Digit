#pragma once

#include "CFringeSegment.h"
#include "DigitizationParams.h"

namespace DigitMode::digitization {

    /**
     * @brief Adapter: Convert pure algorithm output to MFC-compatible CFringeSegment
     *
     * The core digitization algorithm works with pure STL structures (NumberedFringe).
     * This adapter bridges the gap between pure algorithm and MFC-contaminated CFringeSegment.
     *
     * RATIONALE:
     * - CFringeSegment is the first-class data model for CDigitInfo (segment-primary model)
     * - It contains MFC types (inherits from CObject, uses MFC patterns)
     * - Pure algorithm must not depend on MFC
     * - Adapter performs the final conversion: pure → MFC-compatible
     */
    class FringeSegmentAdapter {
    public:
        /**
         * @brief Convert pure NumberedFringe to MFC CFringeSegment
         *
         * @param numberedFringe Pure algorithm output (STL-only)
         * @return MFC-compatible CFringeSegment for CDigitInfo storage
         */
        static CFringeSegment AdaptFringe(const NumberedFringe& numbered);

        /**
         * @brief Convert entire vector of pure fringes to MFC fringes
         *
         * Batch conversion for StandardDigitizer output.
         *
         * @param fringes Vector of pure NumberedFringe objects
         * @return Vector of MFC CFringeSegment objects ready for CDigitInfo::Fringes
         */
        static std::vector<CFringeSegment> AdaptFringes(
            const std::vector<NumberedFringe>& fringes);
    };

} // namespace DigitMode::digitization
