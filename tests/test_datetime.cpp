// Unit tests: date/time parsing used for deadlines.
#include <gtest/gtest.h>

#include <string>

#include "domain/DateTime.h"
#include "domain/Errors.h"

using namespace mlms;

TEST(DateTime, ParsesMinutesAndSecondsFormats) {
    Seconds a = 0;
    Seconds b = 0;
    ASSERT_TRUE(parseDateTime("2026-11-02 10:30", a));
    ASSERT_TRUE(parseDateTime("2026-11-02 10:30:00", b));
    EXPECT_EQ(a, b);
}

TEST(DateTime, RoundTripsThroughFormat) {
    EXPECT_EQ(formatDateTime(toSeconds("2024-02-29 23:59:59")), "2024-02-29 23:59:59");
    EXPECT_EQ(normaliseDateTime("2026-01-05 07:03"), "2026-01-05 07:03:00");
}

TEST(DateTime, DifferenceAcrossMonthAndYearBoundaries) {
    EXPECT_EQ(toSeconds("2026-03-01 00:00") - toSeconds("2026-02-28 00:00"), 86400);
    EXPECT_EQ(toSeconds("2027-01-01 00:00") - toSeconds("2026-12-31 00:00"), 86400);
}

class InvalidDateTime : public ::testing::TestWithParam<const char*> {};

TEST_P(InvalidDateTime, IsRejected) {
    Seconds out = 0;
    EXPECT_FALSE(parseDateTime(GetParam(), out));
    EXPECT_THROW(toSeconds(GetParam()), ValidationError);
}

INSTANTIATE_TEST_SUITE_P(BadInputs, InvalidDateTime,
                         ::testing::Values("", "2026-11-02", "02/11/2026 10:00", "2026-13-01 10:00",
                                           "2026-02-30 10:00", "2025-02-29 10:00", "2026-11-02 24:00",
                                           "2026-11-02 10:60", "2026-11-02 10:00junk", "abcd-ef-gh ij:kl"),
                         [](const auto& info) { return "input" + std::to_string(info.index); });

TEST(DateTime, FixedClockCanBeMoved) {
    FixedClock clock(toSeconds("2026-11-02 08:00"));
    clock.advance(90);
    EXPECT_EQ(clock.nowText(), "2026-11-02 08:01:30");
}
