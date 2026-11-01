#include "app/DemoData.h"

#include <string>

#include "domain/Errors.h"

namespace mlms {

namespace {

int addUser(AppContext& app, const std::string& username, Role role, const std::string& name,
            std::optional<int> department, std::optional<std::string> studentNumber, const std::string& hash) {
    UserRecord r;
    r.username = username;
    r.passwordHash = hash;
    r.role = role;
    r.displayName = name;
    r.departmentId = department;
    r.studentNumber = std::move(studentNumber);
    r.createdAt = app.clock.nowText();
    return app.userRepo.insert(r);
}

int addAssignment(AppContext& app, int courseId, const std::string& title, Seconds deadline, double maxMark,
                  int lecturerId) {
    Assignment a;
    a.courseId = courseId;
    a.title = title;
    a.description = "Demo assignment";
    a.deadline = formatDateTime(deadline);
    a.maxMark = maxMark;
    a.createdBy = lecturerId;
    a.createdAt = app.clock.nowText();
    return app.assignmentRepo.insert(a);
}

}  // namespace

void seedDemoData(AppContext& app) {
    if (app.userRepo.count() > 0) throw RuleViolation("Demo data can only be loaded into an empty database");

    const std::string hash = app.hasher.hash(kDemoPassword);
    const Seconds now = app.clock.now();

    app.tx.run([&] {
        const int cs = app.courseRepo.addDepartment("Mathematics and Computer Science");
        const int lang = app.courseRepo.addDepartment("Languages");

        addUser(app, "admin", Role::Administrator, "System Administrator", std::nullopt, std::nullopt, hash);
        addUser(app, "hod.cs", Role::HoD, "Dr T. Mokoena (HoD CS)", cs, std::nullopt, hash);
        addUser(app, "hod.lang", Role::HoD, "Mrs P. Ntsane (HoD Languages)", lang, std::nullopt, hash);
        const int lec1 = addUser(app, "lec.molefi", Role::Lecturer, "Mr K. Molefi", cs, std::nullopt, hash);
        const int lec2 = addUser(app, "lec.sello", Role::Lecturer, "Ms M. Sello", lang, std::nullopt, hash);

        const int cs101 = app.courseRepo.addCourse({0, cs, "CS101", "Introduction to Programming"});
        const int ma101 = app.courseRepo.addCourse({0, cs, "MA101", "Mathematics I"});
        const int en101 = app.courseRepo.addCourse({0, lang, "EN101", "English Literature"});
        app.courseRepo.assignLecturer(lec1, cs101);
        app.courseRepo.assignLecturer(lec1, ma101);
        app.courseRepo.assignLecturer(lec2, en101);

        const char* names[] = {"Lerato Phiri", "Thabo Mohapi", "Palesa Lethole",
                               "Rethabile Khumalo", "Neo Sekhesa", "Kamohelo Rakuoane"};
        for (int i = 0; i < 6; ++i) {
            const std::string number = "2026" + std::to_string(1001 + i);
            const int id = addUser(app, "stu" + std::to_string(i + 1), Role::Student, names[i], std::nullopt,
                                   number, hash);
            app.courseRepo.enrolStudent(id, cs101, app.clock.nowText());
            app.courseRepo.enrolStudent(id, en101, app.clock.nowText());
            if (i < 3) app.courseRepo.enrolStudent(id, ma101, app.clock.nowText());
        }

        // one deadline already passed (ready to mark), others still open
        addAssignment(app, cs101, "Lab 1 - Variables and Loops", now - 2 * kSecondsPerDay, 50, lec1);
        addAssignment(app, cs101, "Lab 2 - Functions", now + 7 * kSecondsPerDay, 50, lec1);
        addAssignment(app, ma101, "Tutorial 1 - Algebra", now + 3 * kSecondsPerDay, 20, lec1);
        addAssignment(app, en101, "Essay - Poetry Analysis", now + 10 * kSecondsPerDay, 100, lec2);
    });
}

void seedBulkStudents(AppContext& app, int count) {
    const std::string hash = app.hasher.hash(kDemoPassword);
    const auto courses = app.courseRepo.listCourses();
    app.tx.run([&] {
        for (int i = 0; i < count; ++i) {
            const std::string number = "BULK" + std::to_string(100000 + i);
            const int id = addUser(app, "bulk" + std::to_string(i + 1), Role::Student,
                                   "Bulk Student " + std::to_string(i + 1), std::nullopt, number, hash);
            for (const Course& c : courses) app.courseRepo.enrolStudent(id, c.id, app.clock.nowText());
        }
    });
}

}  // namespace mlms
