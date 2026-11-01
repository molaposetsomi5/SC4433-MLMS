// Unit tests for the Strategy class StandardLatePenaltyPolicy.
// Equivalence partitions: on time | 1-3 days late | more than 3 days late.
// Boundary values: exactly at deadline, +1 s, exactly 24 h, 24 h + 1 s, 72 h, 72 h + 1 s.
#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "domain/LatePenaltyPolicy.h"

using namespace mlms;

namespace {
constexpr Seconds kDeadline = 1'000'000;
constexpr Seconds kH = 3600;
}  // namespace

struct LateDaysCase {
    Seconds offset;
    int expectedDays;
};

void PrintTo(const LateDaysCase& c, std::ostream* os) { *os << "offset " << c.offset << "s -> " << c.expectedDays; }

class LateDaysBoundaries : public ::testing::TestWithParam<LateDaysCase> {};

TEST_P(LateDaysBoundaries, CountsStartedDays) {
    StandardLatePenaltyPolicy policy;
    EXPECT_EQ(policy.lateDays(kDeadline + GetParam().offset, kDeadline), GetParam().expectedDays);
}

INSTANTIATE_TEST_SUITE_P(BVA, LateDaysBoundaries,
                         ::testing::Values(LateDaysCase{-24 * kH, 0},      // well before
                                           LateDaysCase{-1, 0},            // 1 s before
                                           LateDaysCase{0, 0},             // exactly at deadline = on time
                                           LateDaysCase{1, 1},             // 1 s late = day 1
                                           LateDaysCase{24 * kH, 1},       // exactly 24 h = day 1
                                           LateDaysCase{24 * kH + 1, 2},   // just into day 2
                                           LateDaysCase{30 * kH, 2},       // worked example
                                           LateDaysCase{72 * kH, 3},       // last second of day 3
                                           LateDaysCase{72 * kH + 1, 4},   // beyond window
                                           LateDaysCase{240 * kH, 10}),
                         [](const auto& info) {
                             const Seconds o = info.param.offset;
                             return (o < 0 ? "minus" + std::to_string(-o) : std::to_string(o)) + "s";
                         });

struct PenaltyCase {
    double raw;
    double max;
    int days;
    double expectedFinal;
    double expectedPenalty;
};

void PrintTo(const PenaltyCase& c, std::ostream* os) {
    *os << "raw " << c.raw << "/" << c.max << ", " << c.days << " day(s) -> " << c.expectedFinal;
}

class PenaltyAmounts : public ::testing::TestWithParam<PenaltyCase> {};

TEST_P(PenaltyAmounts, AppliesTenPercentOfMaxPerDay) {
    StandardLatePenaltyPolicy policy;
    const PenaltyCase c = GetParam();
    const PenaltyResult r = policy.apply(c.raw, c.max, c.days);
    EXPECT_DOUBLE_EQ(r.finalMark, c.expectedFinal);
    EXPECT_DOUBLE_EQ(r.penalty, c.expectedPenalty);
    EXPECT_GE(r.finalMark, 0.0);
}

INSTANTIATE_TEST_SUITE_P(EP_BVA, PenaltyAmounts,
                         ::testing::Values(PenaltyCase{40, 50, 0, 40, 0},    // on time
                                           PenaltyCase{40, 50, 1, 35, 5},    // day 1
                                           PenaltyCase{40, 50, 2, 30, 10},   // SRS worked example
                                           PenaltyCase{40, 50, 3, 25, 15},   // day 3 (last penalised day)
                                           PenaltyCase{40, 50, 4, 0, 40},    // more than 3 days -> zero
                                           PenaltyCase{10, 50, 3, 0, 10},    // penalty bigger than mark -> floor 0
                                           PenaltyCase{0, 50, 1, 0, 0},      // zero raw mark
                                           PenaltyCase{50, 50, 0, 50, 0},    // full marks on time
                                           PenaltyCase{17.5, 20, 1, 15.5, 2}),
                         [](const auto& info) {
                             return "case" + std::to_string(info.index) + "_" +
                                    std::to_string(info.param.days) + "days";
                         });

TEST(LatePenaltyPolicy, NegativeLateDaysTreatedAsOnTime) {
    StandardLatePenaltyPolicy policy;
    const PenaltyResult r = policy.apply(30, 50, -2);
    EXPECT_EQ(r.lateDays, 0);
    EXPECT_DOUBLE_EQ(r.finalMark, 30);
}

TEST(LatePenaltyPolicy, UsableThroughInterface) {
    StandardLatePenaltyPolicy concrete;
    const LatePenaltyPolicy& policy = concrete;  // Strategy is used via the abstraction
    EXPECT_EQ(policy.lateDays(kDeadline + 49 * kH, kDeadline), 3);
}
