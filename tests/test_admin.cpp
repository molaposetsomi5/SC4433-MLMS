// Administrator functions (FR-03, FR-04, FR-05) and role checks (FR-02).
#include "TestApp.h"
#include "domain/Errors.h"

using namespace mlms;
using namespace mlms::test;

class AdminTest : public LmsTest {};

TEST_F(AdminTest, OnlyAdministratorCanCreateUsers) {
    NewUserRequest r;
    r.role = Role::Lecturer;
    r.username = "newlec";
    r.displayName = "New";
    r.password = "Password123";
    EXPECT_THROW(app->users.createUser(*lecturer, r), PermissionError);
    EXPECT_THROW(app->users.createUser(*student, r), PermissionError);
    EXPECT_NO_THROW(app->users.createUser(*admin, r));
}

TEST_F(AdminTest, RejectsInvalidUserDetails) {
    NewUserRequest r;
    r.role = Role::Student;
    r.username = "student";  // taken
    r.displayName = "X";
    r.password = "Password123";
    r.studentNumber = "S999";
    EXPECT_THROW(app->users.createUser(*admin, r), ValidationError);

    r.username = "fresh";
    r.studentNumber.reset();
    EXPECT_THROW(app->users.createUser(*admin, r), ValidationError);  // student needs number

    r.studentNumber = "S001";
    EXPECT_THROW(app->users.createUser(*admin, r), ValidationError);  // duplicate number

    r.role = Role::HoD;
    r.studentNumber.reset();
    EXPECT_THROW(app->users.createUser(*admin, r), ValidationError);  // HoD needs department

    r.departmentId = 999;
    EXPECT_THROW(app->users.createUser(*admin, r), NotFoundError);

    r.departmentId = dept;
    r.password = "short";
    EXPECT_THROW(app->users.createUser(*admin, r), ValidationError);

    r.password = "Password123";
    r.username = "has space";
    EXPECT_THROW(app->users.createUser(*admin, r), ValidationError);
}

TEST_F(AdminTest, InitialAdministratorOnlyOnEmptyDatabase) {
    EXPECT_THROW(app->users.createInitialAdministrator("x", "Password123", "X"), PermissionError);
}

TEST_F(AdminTest, CannotDeactivateOwnAccount) {
    EXPECT_THROW(app->users.setActive(*admin, admin->id(), false), RuleViolation);
}

TEST_F(AdminTest, UpdateDetailsChangesNameAndDepartment) {
    app->users.updateDetails(*admin, lecturer->id(), "Renamed", otherDept);
    auto updated = reload(lecturer);
    EXPECT_EQ(updated->displayName(), "Renamed");
    EXPECT_EQ(updated->departmentId(), otherDept);
    EXPECT_THROW(app->users.updateDetails(*admin, lecturer->id(), "  ", dept), ValidationError);
}

TEST_F(AdminTest, CourseNeedsValidDepartmentAndUniqueCode) {
    EXPECT_THROW(app->courses.createCourse(*admin, 999, "X1", "X"), NotFoundError);
    EXPECT_THROW(app->courses.createCourse(*admin, dept, "CS101", "Dup"), ValidationError);
    EXPECT_THROW(app->courses.createCourse(*lecturer, dept, "CS102", "X"), PermissionError);
    const int id = app->courses.createCourse(*admin, dept, "CS102", "Data Structures");
    EXPECT_EQ(app->courses.getCourse(id).departmentId, dept);
}

TEST_F(AdminTest, DuplicateDepartmentRejected) {
    EXPECT_THROW(app->courses.createDepartment(*admin, "Computer Science"), ValidationError);
    EXPECT_THROW(app->courses.createDepartment(*admin, ""), ValidationError);
}

TEST_F(AdminTest, AssignAndEnrolCheckRoles) {
    EXPECT_THROW(app->courses.assignLecturer(*admin, student->id(), course), ValidationError);
    EXPECT_THROW(app->courses.enrolStudent(*admin, lecturer->id(), course), ValidationError);
    app->courses.enrolStudent(*admin, outsider->id(), course);
    EXPECT_TRUE(app->courseRepo.isEnrolled(outsider->id(), course));
}

TEST_F(AdminTest, CoursesForEachRole) {
    EXPECT_EQ(app->courses.coursesFor(*lecturer).size(), 1u);
    EXPECT_EQ(app->courses.coursesFor(*student).size(), 2u);
    EXPECT_EQ(app->courses.coursesFor(*hod).size(), 1u);
    EXPECT_EQ(app->courses.coursesFor(*admin).size(), 2u);
    EXPECT_EQ(app->courses.coursesFor(*outsider).size(), 0u);
}

TEST_F(AdminTest, AssignmentCreationValidatesInput) {
    setTime(0);
    const std::string future = formatDateTime(t0 + kDay);
    EXPECT_THROW(app->assignments.createAssignment(*lecturer, course, "", "", future, 50), ValidationError);
    EXPECT_THROW(app->assignments.createAssignment(*lecturer, course, "T", "", future, 0), ValidationError);
    EXPECT_THROW(app->assignments.createAssignment(*lecturer, course, "T", "", "tomorrow", 50), ValidationError);
    EXPECT_THROW(app->assignments.createAssignment(*lecturer, course, "T", "", formatDateTime(t0 - 1), 50),
                 ValidationError);
    EXPECT_THROW(app->assignments.createAssignment(*lecturer, otherCourse, "T", "", future, 50), PermissionError);
    EXPECT_THROW(app->assignments.createAssignment(*student, course, "T", "", future, 50), PermissionError);
    EXPECT_GT(app->assignments.createAssignment(*lecturer, course, "T", "", future, 50), 0);
}
