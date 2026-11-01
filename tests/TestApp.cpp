#include "TestApp.h"

namespace mlms::test {

void LmsTest::SetUp() {
    t0 = toSeconds("2026-11-02 08:00:00");
    auto fixed = std::make_unique<FixedClock>(t0);
    clock = fixed.get();
    app = std::make_unique<AppContext>(
        ":memory:", std::move(fixed),
        std::make_unique<SodiumPasswordHasher>(SodiumPasswordHasher::fastForTesting()));

    app->users.createInitialAdministrator("admin", "AdminPass1", "Admin");
    admin = app->userRepo.findByUsername("admin");

    dept = app->courses.createDepartment(*admin, "Computer Science");
    otherDept = app->courses.createDepartment(*admin, "Languages");

    lecturer = makeUser(Role::Lecturer, "lecturer", dept);
    otherLecturer = makeUser(Role::Lecturer, "lecturer2", otherDept);
    hod = makeUser(Role::HoD, "hod", dept);
    otherHod = makeUser(Role::HoD, "hod2", otherDept);
    student = makeUser(Role::Student, "student", std::nullopt, "S001");
    student2 = makeUser(Role::Student, "student2", std::nullopt, "S002");
    outsider = makeUser(Role::Student, "outsider", std::nullopt, "S003");

    course = app->courses.createCourse(*admin, dept, "CS101", "Programming");
    otherCourse = app->courses.createCourse(*admin, otherDept, "EN101", "English");
    app->courses.assignLecturer(*admin, lecturer->id(), course);
    app->courses.assignLecturer(*admin, otherLecturer->id(), otherCourse);
    app->courses.enrolStudent(*admin, student->id(), course);
    app->courses.enrolStudent(*admin, student2->id(), course);
    app->courses.enrolStudent(*admin, student->id(), otherCourse);
}

std::shared_ptr<User> LmsTest::makeUser(Role role, const std::string& username, std::optional<int> department,
                                        std::optional<std::string> studentNumber) {
    NewUserRequest r;
    r.role = role;
    r.username = username;
    r.displayName = username + " name";
    r.password = "Password123";
    r.departmentId = department;
    r.studentNumber = std::move(studentNumber);
    const int id = app->users.createUser(*admin, r);
    return app->userRepo.findById(id);
}

std::shared_ptr<User> LmsTest::reload(const std::shared_ptr<User>& user) { return app->userRepo.findById(user->id()); }

int LmsTest::newAssignment(Seconds deadlineOffset, double maxMark, int courseId) {
    setTime(0);
    const int target = courseId == 0 ? course : courseId;
    const User& owner = target == course ? *lecturer : *otherLecturer;
    return app->assignments.createAssignment(owner, target, "Lab", "desc", formatDateTime(t0 + deadlineOffset),
                                             maxMark);
}

SubmitResult LmsTest::submitAt(const User& who, int assignmentId, Seconds offset, const std::string& content) {
    setTime(offset);
    return app->submissions.submit(who, assignmentId, content);
}

Mark LmsTest::markAt(int submissionId, double raw, Seconds offset, const std::string& feedback) {
    setTime(offset);
    return app->marking.markSubmission(*lecturer, submissionId, raw, feedback);
}

}  // namespace mlms::test
