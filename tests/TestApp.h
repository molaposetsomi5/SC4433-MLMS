#pragma once
// Shared fixture: a full application on an in-memory SQLite database with a
// controllable clock. Used by the integration and system tests.
#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>

#include "app/AppContext.h"

namespace mlms::test {

constexpr Seconds kHour = 3600;
constexpr Seconds kDay = 86400;

class LmsTest : public ::testing::Test {
protected:
    void SetUp() override;

    std::shared_ptr<User> makeUser(Role role, const std::string& username, std::optional<int> department,
                                   std::optional<std::string> studentNumber = std::nullopt);
    std::shared_ptr<User> reload(const std::shared_ptr<User>& user);

    // Creates an assignment in `course` whose deadline is t0 + offset. Clock is left at t0.
    int newAssignment(Seconds deadlineOffset, double maxMark = 50, int courseId = 0);

    // Submits at time t0 + offset.
    SubmitResult submitAt(const User& who, int assignmentId, Seconds offset, const std::string& content = "answer");

    // Marks at t0 + offset (must be after the deadline).
    Mark markAt(int submissionId, double raw, Seconds offset, const std::string& feedback = "feedback");

    void setTime(Seconds offset) { clock->set(t0 + offset); }

    FixedClock* clock = nullptr;
    std::unique_ptr<AppContext> app;
    Seconds t0 = 0;

    std::shared_ptr<User> admin, lecturer, otherLecturer, hod, otherHod, student, student2, outsider;
    int dept = 0, otherDept = 0, course = 0, otherCourse = 0;
};

}  // namespace mlms::test
