#include "domain/LatePenaltyPolicy.h"

#include <algorithm>
#include <cmath>

namespace mlms {

int StandardLatePenaltyPolicy::lateDays(Seconds submittedAt, Seconds effectiveDeadline) const {
    if (submittedAt <= effectiveDeadline) return 0;
    const Seconds late = submittedAt - effectiveDeadline;
    // round up: any started 24-hour period counts as a full day
    return static_cast<int>((late + kSecondsPerDay - 1) / kSecondsPerDay);
}

PenaltyResult StandardLatePenaltyPolicy::apply(double rawMark, double maxMark, int lateDays) const {
    PenaltyResult result;
    result.lateDays = std::max(0, lateDays);

    double finalMark = rawMark;
    if (result.lateDays > kMaxLateDays) {
        finalMark = 0;
    } else if (result.lateDays > 0) {
        finalMark = rawMark - kRatePerDay * maxMark * result.lateDays;
    }
    finalMark = std::max(0.0, finalMark);
    // keep two decimals so stored values are tidy
    finalMark = std::round(finalMark * 100.0) / 100.0;

    result.finalMark = finalMark;
    result.penalty = std::round((rawMark - finalMark) * 100.0) / 100.0;
    return result;
}

}  // namespace mlms
