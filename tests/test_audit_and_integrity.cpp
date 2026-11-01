// FR-14, FR-18, NFR-05, NFR-06: audit trail, scoped audit access and database integrity.
#include "TestApp.h"
#include "data/Schema.h"
#include "domain/Errors.h"

using namespace mlms;
using namespace mlms::test;

class AuditTest : public LmsTest {
protected:
    int assignment = 0;
    int otherAssignment = 0;
    Mark mark;
    void SetUp() override {
        LmsTest::SetUp();
        assignment = newAssignment(kDay);
        otherAssignment = newAssignment(kDay, 100, otherCourse);
        const int sub = submitAt(*student, assignment, kHour).submissionId;
        submitAt(*student, otherAssignment, kHour);
        mark = markAt(sub, 40, 2 * kDay);
        app->moderation.amend(*hod, mark.id, 41, "Moderated");
    }
};

TEST_F(AuditTest, EveryMarkChangeHasActorTimeOldNewAndReason) {
    const auto records = app->auditRepo.listForEntity("MARK", mark.id);
    ASSERT_EQ(records.size(), 2u);
    for (const auto& r : records) {
        EXPECT_GT(r.actorUserId, 0);
        EXPECT_FALSE(r.timestamp.empty());
        EXPECT_TRUE(r.newValue.has_value());
        EXPECT_FALSE(r.reason.empty());
    }
    EXPECT_TRUE(records[1].oldValue.has_value());
    EXPECT_EQ(records[1].actorUserId, hod->id());
}

TEST_F(AuditTest, AuditLogCannotBeUpdatedOrDeleted) {
    EXPECT_THROW(app->db.exec("UPDATE audit_logs SET reason = 'x'"), DatabaseError);
    EXPECT_THROW(app->db.exec("DELETE FROM audit_logs"), DatabaseError);
    EXPECT_EQ(app->auditRepo.listForEntity("MARK", mark.id).size(), 2u);
}

TEST_F(AuditTest, AdministratorSeesEverythingHodSeesOwnDepartment) {
    const auto all = app->audit.query(*admin, {});
    const auto mine = app->audit.query(*hod, {});
    const auto theirs = app->audit.query(*otherHod, {});
    EXPECT_EQ(all.size(), 4u);  // 2 submissions + mark entry + amendment
    EXPECT_EQ(mine.size(), 3u);
    EXPECT_EQ(theirs.size(), 1u);
    for (const auto& e : mine) EXPECT_EQ(e.courseId, course);
}

TEST_F(AuditTest, FiltersByAssignmentAndStudent) {
    AuditFilter byAssignment;
    byAssignment.assignmentId = otherAssignment;
    EXPECT_EQ(app->audit.query(*admin, byAssignment).size(), 1u);
    AuditFilter byStudent;
    byStudent.studentId = student2->id();
    EXPECT_TRUE(app->audit.query(*admin, byStudent).empty());
}

TEST_F(AuditTest, LecturersAndStudentsCannotReadAudit) {
    EXPECT_THROW(app->audit.query(*lecturer, {}), PermissionError);
    EXPECT_THROW(app->audit.query(*student, {}), PermissionError);
}

TEST_F(AuditTest, FailedTransactionRollsBackEverything) {
    const auto before = app->auditRepo.listAll().size();
    EXPECT_THROW(app->tx.run([&] {
        app->audit.record(admin->id(), "TEST", "MARK", mark.id, std::nullopt, "x", "inside tx");
        throw DatabaseError("simulated failure");
    }),
                 DatabaseError);
    EXPECT_EQ(app->auditRepo.listAll().size(), before);
}

TEST_F(AuditTest, ForeignKeysAndChecksAreEnforced) {
    EXPECT_THROW(app->db.exec("INSERT INTO courses (department_id, code, name) VALUES (999, 'Z1', 'Z')"),
                 DatabaseError);
    EXPECT_THROW(app->db.exec("UPDATE marks SET final_mark = -1"), DatabaseError);
    EXPECT_THROW(app->db.exec("UPDATE marks SET status = 'PUBLISHED'"), DatabaseError);
    EXPECT_THROW(app->db.exec("INSERT INTO enrolments VALUES (" + std::to_string(student->id()) + ", " +
                              std::to_string(course) + ", 'x')"),
                 DatabaseError);  // duplicate composite key
}

TEST_F(AuditTest, SchemaIsVersioned) { EXPECT_GE(schemaVersion(app->db), 1); }

TEST_F(AuditTest, QueriesAreParameterisedAgainstInjection) {
    // a username full of SQL just fails to log in; the users table is untouched
    EXPECT_THROW(app->auth.login("x' OR '1'='1", "x' OR '1'='1"), AuthError);
    EXPECT_THROW(app->auth.login("admin'; DROP TABLE users; --", "x"), AuthError);
    EXPECT_NE(app->userRepo.findByUsername("admin"), nullptr);
}
