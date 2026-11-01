// v1.1 - Change request CR1: class-wide extensions shown on the student's assignment list.
#include <sstream>

#include "TestApp.h"
#include "data/Schema.h"
#include "domain/Errors.h"
#include "ui/ConsoleUI.h"

using namespace mlms;
using namespace mlms::test;

class ClassExtensionTest : public LmsTest {
protected:
    int assignment = 0;
    void SetUp() override {
        LmsTest::SetUp();
        assignment = newAssignment(kDay);  // original deadline t0 + 1 day
    }
    std::string grant(Seconds deadlineOffset, const std::string& reason = "Exam week") {
        setTime(0);
        return app->extensions.grantClass(*lecturer, assignment, formatDateTime(t0 + deadlineOffset), reason);
    }
    StudentAssignmentView viewFor(const User& who) {
        for (const auto& v : app->assignments.listForStudent(who)) {
            if (v.assignment.id == assignment) return v;
        }
        throw NotFoundError("assignment not in list");
    }
};

TEST_F(ClassExtensionTest, MovesTheDeadlineForEveryStudent) {
    grant(3 * kDay);
    EXPECT_EQ(submitAt(*student, assignment, 2 * kDay).status, SubmissionStatus::Submitted);
    EXPECT_EQ(submitAt(*student2, assignment, 2 * kDay + kHour).status, SubmissionStatus::Submitted);
}

TEST_F(ClassExtensionTest, ShownOnStudentAssignmentList) {
    grant(3 * kDay, "School trip");
    const StudentAssignmentView v = viewFor(*student2);
    EXPECT_TRUE(v.deadline.classExtension);
    EXPECT_FALSE(v.deadline.individualExtension);
    EXPECT_EQ(v.deadline.effective, formatDateTime(t0 + 3 * kDay));
    EXPECT_EQ(v.deadline.original, formatDateTime(t0 + kDay));
    EXPECT_EQ(v.deadline.extensionReason, "School trip");
}

TEST_F(ClassExtensionTest, ShownOnLecturerAndHodAssignmentLists) {
    grant(3 * kDay);
    for (const auto& ca : app->assignments.listVisibleTo(*lecturer)) {
        if (ca.assignment.id == assignment) {
            ASSERT_TRUE(ca.classExtension.has_value());
            EXPECT_EQ(ca.classExtension->revisedDeadline, formatDateTime(t0 + 3 * kDay));
        }
    }
    EXPECT_TRUE(app->assignments.listVisibleTo(*hod).front().classExtension.has_value());
}

TEST_F(ClassExtensionTest, LaterIndividualExtensionStillWins) {
    grant(3 * kDay);
    app->extensions.grantIndividual(*lecturer, assignment, student->id(), formatDateTime(t0 + 5 * kDay), "Illness");
    const StudentAssignmentView v = viewFor(*student);
    EXPECT_TRUE(v.deadline.individualExtension);
    EXPECT_EQ(v.deadline.effective, formatDateTime(t0 + 5 * kDay));
    EXPECT_FALSE(viewFor(*student2).deadline.individualExtension);
}

TEST_F(ClassExtensionTest, ClassExtensionNeverShortensAnIndividualOne) {
    setTime(0);
    app->extensions.grantIndividual(*lecturer, assignment, student->id(), formatDateTime(t0 + 5 * kDay), "Illness");
    grant(2 * kDay);
    EXPECT_EQ(viewFor(*student).deadline.effective, formatDateTime(t0 + 5 * kDay));
    EXPECT_EQ(viewFor(*student2).deadline.effective, formatDateTime(t0 + 2 * kDay));
}

TEST_F(ClassExtensionTest, PenaltyClockStartsFromClassDeadline) {
    grant(2 * kDay);
    const int sub = submitAt(*student, assignment, 2 * kDay + 30 * kHour).submissionId;  // 2 days after new deadline
    const Mark m = markAt(sub, 40, 6 * kDay);
    EXPECT_DOUBLE_EQ(m.finalMark, 30);
}

TEST_F(ClassExtensionTest, MarkingWaitsForTheClassDeadline) {
    grant(3 * kDay);
    const int sub = submitAt(*student, assignment, kHour).submissionId;
    EXPECT_THROW(markAt(sub, 40, 2 * kDay), RuleViolation);  // original passed, class deadline not
    EXPECT_NO_THROW(markAt(sub, 40, 3 * kDay + 1));
}

TEST_F(ClassExtensionTest, InputAndPermissionRules) {
    setTime(0);
    const std::string later = formatDateTime(t0 + 3 * kDay);
    EXPECT_THROW(app->extensions.grantClass(*lecturer, assignment, later, ""), ValidationError);
    EXPECT_THROW(app->extensions.grantClass(*lecturer, assignment, formatDateTime(t0 + kDay), "x"), ValidationError);
    EXPECT_THROW(app->extensions.grantClass(*lecturer, assignment, "next week", "x"), ValidationError);
    EXPECT_THROW(app->extensions.grantClass(*otherLecturer, assignment, later, "x"), PermissionError);
    EXPECT_THROW(app->extensions.grantClass(*hod, assignment, later, "x"), PermissionError);
    EXPECT_THROW(app->extensions.grantClass(*student, assignment, later, "x"), PermissionError);
    EXPECT_THROW(app->extensions.grantClass(*lecturer, 9999, later, "x"), NotFoundError);
    EXPECT_FALSE(app->extensionRepo.findClassExtension(assignment).has_value());
}

TEST_F(ClassExtensionTest, RefusedOnceMarkingHasStarted) {
    const int sub = submitAt(*student, assignment, kHour).submissionId;
    markAt(sub, 40, 2 * kDay);
    EXPECT_THROW(grant(5 * kDay), RuleViolation);
}

TEST_F(ClassExtensionTest, CanBeChangedAndEveryGrantIsAudited) {
    grant(2 * kDay, "first");
    grant(3 * kDay, "longer");
    EXPECT_EQ(app->extensionRepo.findClassExtension(assignment)->revisedDeadline, formatDateTime(t0 + 3 * kDay));
    const auto records = app->auditRepo.listForEntity("ASSIGNMENT", assignment);
    ASSERT_EQ(records.size(), 2u);
    EXPECT_EQ(records[0].eventType, "CLASS_EXTENSION_GRANTED");
    EXPECT_EQ(records[1].eventType, "CLASS_EXTENSION_CHANGED");
    EXPECT_EQ(records[1].oldValue, formatDateTime(t0 + 2 * kDay));
    EXPECT_EQ(records[1].reason, "longer");
    // visible to the HoD of the department, not to the other HoD
    EXPECT_EQ(app->audit.query(*hod, {}).size(), 2u);
    EXPECT_TRUE(app->audit.query(*otherHod, {}).empty());
}

TEST_F(ClassExtensionTest, ConsoleLecturerGrantsAndStudentSeesIt) {
    setTime(0);
    std::istringstream in("1\nlecturer\nPassword123\n8\n" + std::to_string(assignment) + "\n" +
                          formatDateTime(t0 + 3 * kDay).substr(0, 16) + "\nPower cut\n0\n" +
                          "1\nstudent2\nPassword123\n1\n0\n0\n");
    std::ostringstream out;
    ConsoleUI ui(*app, in, out);
    ui.run();
    const std::string text = out.str();
    EXPECT_NE(text.find("Deadline extended for the whole class until"), std::string::npos);
    EXPECT_NE(text.find("extended for the whole class (was " + formatDateTime(t0 + kDay) + ")"), std::string::npos);
}

// Regression for the upgrade path: a v1.0 database (schema version 1) is
// migrated to version 2 without losing data.
TEST_F(ClassExtensionTest, VersionOneDatabaseIsUpgraded) {
    submitAt(*student, assignment, kHour);
    app->db.exec("DROP TABLE class_extensions; PRAGMA user_version = 1;");  // make it look like v1.0
    ASSERT_EQ(schemaVersion(app->db), 1);
    migrateSchema(app->db);
    EXPECT_EQ(schemaVersion(app->db), 2);
    EXPECT_TRUE(app->submissionRepo.find(assignment, student->id()).has_value());
    EXPECT_NO_THROW(grant(3 * kDay));
}
