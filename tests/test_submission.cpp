// UC-01 Submit assignment: FR-07, FR-08, FR-09, FR-10, FR-15.
#include "TestApp.h"
#include "domain/Errors.h"

using namespace mlms;
using namespace mlms::test;

class SubmissionTest : public LmsTest {};

TEST_F(SubmissionTest, OnTimeSubmissionIsRecorded) {
    const int a = newAssignment(kDay);
    const SubmitResult r = submitAt(*student, a, kHour);
    EXPECT_EQ(r.status, SubmissionStatus::Submitted);
    EXPECT_EQ(r.versionNumber, 1);
    EXPECT_EQ(r.lateDays, 0);
    EXPECT_FALSE(r.resubmission);
    EXPECT_EQ(app->submissionRepo.versions(r.submissionId).size(), 1u);
}

TEST_F(SubmissionTest, ExactlyAtDeadlineIsOnTime) {
    const int a = newAssignment(kDay);
    EXPECT_EQ(submitAt(*student, a, kDay).status, SubmissionStatus::Submitted);
}

TEST_F(SubmissionTest, OneSecondAfterDeadlineIsLate) {
    const int a = newAssignment(kDay);
    const SubmitResult r = submitAt(*student, a, kDay + 1);
    EXPECT_EQ(r.status, SubmissionStatus::Late);
    EXPECT_EQ(r.lateDays, 1);
    EXPECT_FALSE(r.beyondLateWindow);
}

TEST_F(SubmissionTest, MoreThanThreeDaysLateIsStillAcceptedButFlagged) {
    const int a = newAssignment(kDay);
    const SubmitResult r = submitAt(*student, a, kDay + 3 * kDay + 1);
    EXPECT_EQ(r.status, SubmissionStatus::Late);
    EXPECT_EQ(r.lateDays, 4);
    EXPECT_TRUE(r.beyondLateWindow);
}

TEST_F(SubmissionTest, ResubmissionBeforeDeadlineKeepsHistory) {
    const int a = newAssignment(kDay);
    submitAt(*student, a, kHour, "first");
    const SubmitResult r = submitAt(*student, a, 2 * kHour, "second");
    EXPECT_TRUE(r.resubmission);
    EXPECT_EQ(r.versionNumber, 2);
    const auto versions = app->submissionRepo.versions(r.submissionId);
    ASSERT_EQ(versions.size(), 2u);
    EXPECT_FALSE(versions[0].isLatestValid);
    EXPECT_TRUE(versions[1].isLatestValid);
    EXPECT_EQ(app->submissionRepo.latestVersion(r.submissionId)->contentReference, "second");
}

TEST_F(SubmissionTest, ResubmissionAfterDeadlineIsRejected) {
    const int a = newAssignment(kDay);
    submitAt(*student, a, kHour, "first");
    EXPECT_THROW(submitAt(*student, a, kDay + 1, "too late"), RuleViolation);
    auto sub = app->submissionRepo.find(a, student->id());
    EXPECT_EQ(app->submissionRepo.versions(sub->id).size(), 1u);
}

TEST_F(SubmissionTest, NotEnrolledStudentCannotSubmit) {
    const int a = newAssignment(kDay);
    EXPECT_THROW(submitAt(*outsider, a, kHour), PermissionError);
}

TEST_F(SubmissionTest, StaffCannotSubmit) {
    const int a = newAssignment(kDay);
    EXPECT_THROW(submitAt(*lecturer, a, kHour), PermissionError);
}

TEST_F(SubmissionTest, BlankContentAndUnknownAssignmentRejected) {
    const int a = newAssignment(kDay);
    EXPECT_THROW(submitAt(*student, a, kHour, "   "), ValidationError);
    EXPECT_THROW(submitAt(*student, 9999, kHour), NotFoundError);
    EXPECT_FALSE(app->submissionRepo.find(a, student->id()).has_value());
}

TEST_F(SubmissionTest, IndividualExtensionMovesThatStudentsDeadlineOnly) {
    const int a = newAssignment(kDay);
    setTime(kHour);
    app->extensions.grantIndividual(*lecturer, a, student->id(), formatDateTime(t0 + 3 * kDay), "Illness");
    EXPECT_EQ(submitAt(*student, a, 2 * kDay).status, SubmissionStatus::Submitted);
    EXPECT_EQ(submitAt(*student2, a, 2 * kDay).status, SubmissionStatus::Late);
}

TEST_F(SubmissionTest, ExtensionShownInStudentAssignmentList) {
    const int a = newAssignment(kDay);
    setTime(0);
    app->extensions.grantIndividual(*lecturer, a, student->id(), formatDateTime(t0 + 2 * kDay), "Sports trip");
    for (const auto& view : app->assignments.listForStudent(*student)) {
        if (view.assignment.id != a) continue;
        EXPECT_TRUE(view.deadline.individualExtension);
        EXPECT_EQ(view.deadline.effective, formatDateTime(t0 + 2 * kDay));
        EXPECT_EQ(view.deadline.original, formatDateTime(t0 + kDay));
    }
}

TEST_F(SubmissionTest, ExtensionRulesAreEnforced) {
    const int a = newAssignment(kDay);
    setTime(0);
    const std::string later = formatDateTime(t0 + 2 * kDay);
    EXPECT_THROW(app->extensions.grantIndividual(*lecturer, a, student->id(), later, " "), ValidationError);
    EXPECT_THROW(app->extensions.grantIndividual(*lecturer, a, student->id(), formatDateTime(t0 + kHour), "x"),
                 ValidationError);
    EXPECT_THROW(app->extensions.grantIndividual(*otherLecturer, a, student->id(), later, "x"), PermissionError);
    EXPECT_THROW(app->extensions.grantIndividual(*student, a, student->id(), later, "x"), PermissionError);
    EXPECT_THROW(app->extensions.grantIndividual(*lecturer, a, outsider->id(), later, "x"), ValidationError);
}

TEST_F(SubmissionTest, ExtensionCanBeChangedAndIsAudited) {
    const int a = newAssignment(kDay);
    setTime(0);
    app->extensions.grantIndividual(*lecturer, a, student->id(), formatDateTime(t0 + 2 * kDay), "first");
    app->extensions.grantIndividual(*lecturer, a, student->id(), formatDateTime(t0 + 3 * kDay), "longer");
    EXPECT_EQ(app->extensionRepo.find(a, student->id())->revisedDeadline, formatDateTime(t0 + 3 * kDay));
    const auto entries = app->audit.query(*admin, {});
    ASSERT_GE(entries.size(), 2u);
    EXPECT_EQ(entries.back().record.eventType, "EXTENSION_CHANGED");
    EXPECT_EQ(entries.back().record.oldValue, formatDateTime(t0 + 2 * kDay));
}

TEST_F(SubmissionTest, SubmissionIsAudited) {
    const int a = newAssignment(kDay);
    const SubmitResult r = submitAt(*student, a, kHour);
    auto records = app->auditRepo.listForEntity("SUBMISSION", r.submissionId);
    ASSERT_EQ(records.size(), 1u);
    EXPECT_EQ(records[0].actorUserId, student->id());
}
