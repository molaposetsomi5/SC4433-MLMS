// UC-03 Moderate and release marks: FR-12, FR-13, NFR-04.
#include "TestApp.h"
#include "domain/Errors.h"

using namespace mlms;
using namespace mlms::test;

class ModerationTest : public LmsTest {
protected:
    int assignment = 0;
    int sub1 = 0;
    int sub2 = 0;
    Mark mark1;
    Mark mark2;
    void SetUp() override {
        LmsTest::SetUp();
        assignment = newAssignment(kDay);
        sub1 = submitAt(*student, assignment, kHour).submissionId;
        sub2 = submitAt(*student2, assignment, kHour).submissionId;
        mark1 = markAt(sub1, 40, 2 * kDay, "Good");
        mark2 = markAt(sub2, 30, 2 * kDay, "Fair");
    }
    MarkStatus statusOf(int markId) { return app->markRepo.findById(markId)->status; }
};

TEST_F(ModerationTest, StudentCannotSeeUnreleasedMark) {
    EXPECT_TRUE(app->release.resultsFor(*student).empty());
    app->moderation.approve(*hod, mark1.id, "");
    EXPECT_TRUE(app->release.resultsFor(*student).empty());  // approved but not released
}

TEST_F(ModerationTest, PendingListShowsDepartmentMarksOnly) {
    EXPECT_EQ(app->moderation.pending(*hod).size(), 2u);
    EXPECT_TRUE(app->moderation.pending(*otherHod).empty());
    EXPECT_THROW(app->moderation.pending(*lecturer), PermissionError);
}

TEST_F(ModerationTest, ApproveRecordsDecision) {
    app->moderation.approve(*hod, mark1.id, "");
    EXPECT_EQ(statusOf(mark1.id), MarkStatus::Approved);
    const auto decisions = app->markRepo.decisionsForMark(mark1.id);
    ASSERT_EQ(decisions.size(), 1u);
    EXPECT_EQ(decisions[0].decision, ModerationAction::Approve);
    EXPECT_EQ(decisions[0].hodUserId, hod->id());
    EXPECT_FALSE(decisions[0].decidedAt.empty());
}

TEST_F(ModerationTest, AmendNeedsReasonAndRecalculates) {
    EXPECT_THROW(app->moderation.amend(*hod, mark1.id, 45, ""), ValidationError);
    EXPECT_THROW(app->moderation.amend(*hod, mark1.id, 51, "too high"), ValidationError);
    app->moderation.amend(*hod, mark1.id, 45, "Under-marked Q3");
    auto m = app->markRepo.findById(mark1.id);
    EXPECT_DOUBLE_EQ(m->finalMark, 45);
    EXPECT_EQ(m->status, MarkStatus::Approved);
    const auto decisions = app->markRepo.decisionsForMark(mark1.id);
    EXPECT_EQ(decisions[0].previousMark, 40);
    EXPECT_EQ(decisions[0].newMark, 45);
}

TEST_F(ModerationTest, AmendKeepsLatePenalty) {
    auto late = makeUser(Role::Student, "latecomer", std::nullopt, "S010");
    app->courses.enrolStudent(*admin, late->id(), course);
    const int sub = submitAt(*late, assignment, kDay + 30 * kHour).submissionId;  // 2 days late
    const Mark m = markAt(sub, 40, 4 * kDay);
    EXPECT_DOUBLE_EQ(m.finalMark, 30);
    app->moderation.amend(*hod, m.id, 45, "Answer 4 deserved more");
    auto amended = app->markRepo.findById(m.id);
    EXPECT_DOUBLE_EQ(amended->rawMark, 45);
    EXPECT_DOUBLE_EQ(amended->penalty, 10);
    EXPECT_DOUBLE_EQ(amended->finalMark, 35);
}

TEST_F(ModerationTest, ReturnSendsMarkBackToLecturer) {
    EXPECT_THROW(app->moderation.returnForCorrection(*hod, mark1.id, " "), ValidationError);
    app->moderation.returnForCorrection(*hod, mark1.id, "Check Q2");
    EXPECT_EQ(statusOf(mark1.id), MarkStatus::Returned);
    EXPECT_THROW(app->moderation.approve(*hod, mark1.id, ""), RuleViolation);  // must be corrected first
    app->marking.correctMark(*lecturer, sub1, 42, "", "Re-marked Q2");
    EXPECT_EQ(statusOf(mark1.id), MarkStatus::AwaitingModeration);
    app->moderation.approve(*hod, mark1.id, "");
    EXPECT_EQ(statusOf(mark1.id), MarkStatus::Approved);
}

TEST_F(ModerationTest, WrongDepartmentIsDeniedAndNothingRecorded) {
    EXPECT_THROW(app->moderation.approve(*otherHod, mark1.id, ""), PermissionError);
    EXPECT_THROW(app->release.releaseAssignment(*otherHod, assignment), PermissionError);
    EXPECT_TRUE(app->markRepo.decisionsForMark(mark1.id).empty());
    EXPECT_EQ(statusOf(mark1.id), MarkStatus::AwaitingModeration);
}

TEST_F(ModerationTest, LecturerCannotChangeApprovedMark) {
    app->moderation.approve(*hod, mark1.id, "");
    EXPECT_THROW(app->marking.correctMark(*lecturer, sub1, 50, "", "sneaky"), RuleViolation);
}

TEST_F(ModerationTest, MixedBatchIsRejectedAsAWhole) {
    app->moderation.approve(*hod, mark1.id, "");
    EXPECT_THROW(app->release.releaseAssignment(*hod, assignment), RuleViolation);
    EXPECT_EQ(statusOf(mark1.id), MarkStatus::Approved);  // not released
    EXPECT_TRUE(app->release.resultsFor(*student).empty());
}

TEST_F(ModerationTest, UnmarkedSubmissionBlocksRelease) {
    // a third student submits but is never marked
    auto s3 = makeUser(Role::Student, "student3", std::nullopt, "S004");
    app->courses.enrolStudent(*admin, s3->id(), course);
    submitAt(*s3, assignment, kHour);
    app->moderation.approve(*hod, mark1.id, "");
    app->moderation.approve(*hod, mark2.id, "");
    const ReleaseSummary s = app->release.summarise(*hod, assignment);
    EXPECT_EQ(s.unmarked, 1);
    EXPECT_THROW(app->release.releaseAssignment(*hod, assignment), RuleViolation);
}

TEST_F(ModerationTest, ReleaseShowsEachStudentOnlyTheirOwnMark) {
    app->moderation.approve(*hod, mark1.id, "");
    app->moderation.amend(*hod, mark2.id, 33, "Consistency");
    EXPECT_EQ(app->release.releaseAssignment(*hod, assignment), 2);

    const auto r1 = app->release.resultsFor(*student);
    ASSERT_EQ(r1.size(), 1u);
    EXPECT_EQ(r1[0].mark.submissionId, sub1);
    EXPECT_DOUBLE_EQ(r1[0].mark.finalMark, 40);
    EXPECT_EQ(r1[0].mark.feedback, "Good");

    const auto r2 = app->release.resultsFor(*student2);
    ASSERT_EQ(r2.size(), 1u);
    EXPECT_DOUBLE_EQ(r2[0].mark.finalMark, 33);
    EXPECT_TRUE(app->release.resultsFor(*outsider).empty());
}

TEST_F(ModerationTest, OnlyHodCanRelease) {
    app->moderation.approve(*hod, mark1.id, "");
    app->moderation.approve(*hod, mark2.id, "");
    EXPECT_THROW(app->release.releaseAssignment(*lecturer, assignment), PermissionError);
    EXPECT_THROW(app->release.releaseAssignment(*admin, assignment), PermissionError);
    EXPECT_THROW(app->release.resultsFor(*hod), PermissionError);
}

TEST_F(ModerationTest, ReleasedMarkIsFinal) {
    app->moderation.approve(*hod, mark1.id, "");
    app->moderation.approve(*hod, mark2.id, "");
    app->release.releaseAssignment(*hod, assignment);
    EXPECT_THROW(app->release.releaseAssignment(*hod, assignment), RuleViolation);
    EXPECT_THROW(app->moderation.amend(*hod, mark1.id, 10, "late change"), RuleViolation);
    EXPECT_THROW(app->marking.correctMark(*lecturer, sub1, 10, "", "late change"), RuleViolation);
    // assignment is now closed for new submissions (SRS C15)
    auto s3 = makeUser(Role::Student, "student3", std::nullopt, "S004");
    app->courses.enrolStudent(*admin, s3->id(), course);
    EXPECT_THROW(submitAt(*s3, assignment, 3 * kDay), RuleViolation);
}

TEST_F(ModerationTest, NoSubmissionsMeansNothingToRelease) {
    const int empty = newAssignment(kDay);
    EXPECT_THROW(app->release.releaseAssignment(*hod, empty), RuleViolation);
}
