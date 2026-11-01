// UC-02 Mark submission and provide feedback: FR-09, FR-11, FR-14.
#include "TestApp.h"
#include "domain/Errors.h"

using namespace mlms;
using namespace mlms::test;

class MarkingTest : public LmsTest {
protected:
    int assignment = 0;
    int submission = 0;
    void SetUp() override {
        LmsTest::SetUp();
        assignment = newAssignment(kDay);
        submission = submitAt(*student, assignment, kHour).submissionId;
    }
};

TEST_F(MarkingTest, CannotMarkBeforeDeadline) {
    EXPECT_THROW(markAt(submission, 40, kHour * 2), RuleViolation);
    EXPECT_THROW(markAt(submission, 40, kDay), RuleViolation);  // exactly at the deadline
    EXPECT_NO_THROW(markAt(submission, 40, kDay + 1));
}

TEST_F(MarkingTest, MarkMustBeWithinRange) {
    EXPECT_THROW(markAt(submission, -0.5, 2 * kDay), ValidationError);
    EXPECT_THROW(markAt(submission, 50.01, 2 * kDay), ValidationError);
    EXPECT_FALSE(app->markRepo.findBySubmission(submission).has_value());
}

TEST_F(MarkingTest, BoundaryMarksAccepted) {
    EXPECT_EQ(markAt(submission, 50, 2 * kDay).finalMark, 50);
    const int s2 = submitAt(*student2, assignment, kHour).submissionId;
    EXPECT_EQ(markAt(s2, 0, 2 * kDay).finalMark, 0);
}

TEST_F(MarkingTest, OnlyAssignedLecturerCanMark) {
    setTime(2 * kDay);
    EXPECT_THROW(app->marking.markSubmission(*otherLecturer, submission, 40, "x"), PermissionError);
    EXPECT_THROW(app->marking.markSubmission(*hod, submission, 40, "x"), PermissionError);
    EXPECT_THROW(app->marking.markSubmission(*student, submission, 40, "x"), PermissionError);
    EXPECT_THROW(app->marking.markSubmission(*lecturer, 9999, 40, "x"), NotFoundError);
}

TEST_F(MarkingTest, NewMarkAwaitsModerationAndIsAudited) {
    const Mark m = markAt(submission, 42, 2 * kDay, "Well done");
    EXPECT_EQ(m.status, MarkStatus::AwaitingModeration);
    EXPECT_EQ(m.feedback, "Well done");
    auto records = app->auditRepo.listForEntity("MARK", m.id);
    ASSERT_EQ(records.size(), 1u);
    EXPECT_EQ(records[0].eventType, "MARK_ENTERED");
    EXPECT_EQ(records[0].actorUserId, lecturer->id());
    EXPECT_FALSE(records[0].oldValue.has_value());
    EXPECT_EQ(records[0].newValue, "raw=42.00; penalty=0.00; final=42.00");
    EXPECT_FALSE(records[0].reason.empty());
}

TEST_F(MarkingTest, LatePenaltyWorkedExample) {
    // SRS worked example: max 50, raw 40, 30 hours late -> 2 days -> final 30
    const int s2 = submitAt(*student2, assignment, kDay + 30 * kHour).submissionId;
    const Mark m = markAt(s2, 40, 5 * kDay);
    EXPECT_DOUBLE_EQ(m.penalty, 10);
    EXPECT_DOUBLE_EQ(m.finalMark, 30);
}

TEST_F(MarkingTest, MoreThanThreeDaysLateGetsZero) {
    const int s2 = submitAt(*student2, assignment, kDay + 72 * kHour + 1).submissionId;
    const Mark m = markAt(s2, 45, 6 * kDay);
    EXPECT_DOUBLE_EQ(m.finalMark, 0);
    EXPECT_DOUBLE_EQ(m.rawMark, 45);
}

TEST_F(MarkingTest, ExtensionCountsTowardsPenaltyClock) {
    setTime(0);
    app->extensions.grantIndividual(*lecturer, assignment, student2->id(), formatDateTime(t0 + 2 * kDay), "ok");
    const int s2 = submitAt(*student2, assignment, 2 * kDay + kHour).submissionId;  // 1 day late vs extension
    const Mark m = markAt(s2, 40, 5 * kDay);
    EXPECT_DOUBLE_EQ(m.finalMark, 35);
}

TEST_F(MarkingTest, SecondMarkMustUseCorrection) {
    markAt(submission, 40, 2 * kDay);
    EXPECT_THROW(markAt(submission, 41, 2 * kDay), RuleViolation);
}

TEST_F(MarkingTest, CorrectionNeedsReasonAndIsAudited) {
    const Mark m = markAt(submission, 40, 2 * kDay);
    EXPECT_THROW(app->marking.correctMark(*lecturer, submission, 44, "", ""), ValidationError);
    const Mark c = app->marking.correctMark(*lecturer, submission, 44, "", "Missed a question");
    EXPECT_DOUBLE_EQ(c.finalMark, 44);
    EXPECT_EQ(c.feedback, "feedback");  // kept when left blank
    auto records = app->auditRepo.listForEntity("MARK", m.id);
    ASSERT_EQ(records.size(), 2u);
    EXPECT_EQ(records[1].eventType, "MARK_CORRECTED");
    EXPECT_EQ(records[1].oldValue, "raw=40.00; penalty=0.00; final=40.00");
    EXPECT_EQ(records[1].newValue, "raw=44.00; penalty=0.00; final=44.00");
    EXPECT_EQ(records[1].reason, "Missed a question");
}

TEST_F(MarkingTest, CannotCorrectUnmarkedSubmission) {
    setTime(2 * kDay);
    EXPECT_THROW(app->marking.correctMark(*lecturer, submission, 40, "", "why"), NotFoundError);
}

TEST_F(MarkingTest, ExtensionNotAllowedAfterMarking) {
    markAt(submission, 40, 2 * kDay);
    EXPECT_THROW(app->extensions.grantIndividual(*lecturer, assignment, student->id(),
                                                 formatDateTime(t0 + 5 * kDay), "late request"),
                 RuleViolation);
}

TEST_F(MarkingTest, CannotMarkBeforeStudentsExtendedDeadline) {
    // student2 has an extension, so the deadline is still open for them when marking happens
    setTime(0);
    app->extensions.grantIndividual(*lecturer, assignment, student2->id(), formatDateTime(t0 + 5 * kDay), "ok");
    const int s2 = submitAt(*student2, assignment, kHour).submissionId;
    // lecturer cannot mark s2 before student2's effective deadline
    EXPECT_THROW(markAt(s2, 30, 2 * kDay), RuleViolation);
}

TEST_F(MarkingTest, LecturerSeesSubmissionsWithLatenessAndOwnMarks) {
    submitAt(*student2, assignment, kDay + 30 * kHour);
    markAt(submission, 40, 3 * kDay);
    const auto views = app->marking.submissionsFor(*lecturer, assignment);
    ASSERT_EQ(views.size(), 2u);
    EXPECT_TRUE(views[0].mark.has_value());
    EXPECT_EQ(views[1].lateDays, 2);
    EXPECT_FALSE(views[1].mark.has_value());
    EXPECT_THROW(app->marking.submissionsFor(*otherLecturer, assignment), PermissionError);
}
