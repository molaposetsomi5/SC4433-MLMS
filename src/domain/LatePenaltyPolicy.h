#pragma once
// Strategy pattern for late penalties (design section 4.4, pattern 1).
#include "domain/DateTime.h"

namespace mlms {

struct PenaltyResult {
    int lateDays = 0;
    double penalty = 0;    // marks actually deducted
    double finalMark = 0;  // never negative
};

class LatePenaltyPolicy {
public:
    virtual ~LatePenaltyPolicy() = default;
    // Number of started 24-hour periods after the effective deadline (0 = on time).
    virtual int lateDays(Seconds submittedAt, Seconds effectiveDeadline) const = 0;
    virtual PenaltyResult apply(double rawMark, double maxMark, int lateDays) const = 0;
};

// Agreed policy (SRS C3/C4, FR-09):
//  - exactly at the deadline is on time, exactly 24h late is day 1
//  - 10% of the maximum mark is deducted per late day, for days 1 to 3
//  - more than 3 days late gives a final mark of zero
//  - the final mark never goes below zero
class StandardLatePenaltyPolicy final : public LatePenaltyPolicy {
public:
    static constexpr int kMaxLateDays = 3;
    static constexpr double kRatePerDay = 0.10;

    int lateDays(Seconds submittedAt, Seconds effectiveDeadline) const override;
    PenaltyResult apply(double rawMark, double maxMark, int lateDays) const override;
};

}  // namespace mlms
