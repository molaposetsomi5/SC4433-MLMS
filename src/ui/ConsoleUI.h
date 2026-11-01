#pragma once
// Presentation layer: role-specific menus. It only talks to the services,
// never to repositories or SQL (design section 3.4).
#include <iostream>
#include <memory>

#include "app/AppContext.h"
#include "ui/Console.h"

namespace mlms {

class ConsoleUI {
public:
    ConsoleUI(AppContext& app, std::istream& in, std::ostream& out);
    void run();

private:
    template <typename Action>
    void attempt(Action action);

    void firstRunSetup();
    std::shared_ptr<User> loginScreen();
    void changeMyPassword(User& user);

    // Administrator
    void adminMenu(User& admin);
    void createUser(const User& admin);
    void listUsers(const User& admin);
    void setUserActive(const User& admin);
    void editUser(const User& admin);
    void resetUserPassword(const User& admin);
    void createDepartment(const User& admin);
    void listDepartments();
    void createCourse(const User& admin);
    void listCourses();
    void assignLecturer(const User& admin);
    void enrolStudent(const User& admin);
    void showAudit(const User& actor);

    // Lecturer
    void lecturerMenu(User& lecturer);
    void listMyCourses(const User& user);
    void createAssignment(const User& lecturer);
    void listStaffAssignments(const User& staff);
    void viewSubmissions(const User& lecturer);
    void markSubmission(const User& lecturer);
    void correctMark(const User& lecturer);
    void grantExtension(const User& lecturer);

    // Head of Department
    void hodMenu(User& hod);
    void showPending(const User& hod);
    void approveMark(const User& hod);
    void amendMark(const User& hod);
    void returnMark(const User& hod);
    void releaseMarks(const User& hod);

    // Student
    void studentMenu(User& student);
    void listStudentAssignments(const User& student);
    void submitWork(const User& student);
    void viewResults(const User& student);

    AppContext& app_;
    Console io_;
};

}  // namespace mlms
