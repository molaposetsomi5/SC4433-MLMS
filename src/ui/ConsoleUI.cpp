#include "ui/ConsoleUI.h"

#include <string>

#include "domain/Errors.h"
#include "domain/Validation.h"

namespace mlms {

ConsoleUI::ConsoleUI(AppContext& app, std::istream& in, std::ostream& out) : app_(app), io_(in, out) {}

// Runs one menu action; any expected error is shown and control returns to the menu (FR-16).
template <typename Action>
void ConsoleUI::attempt(Action action) {
    try {
        action();
    } catch (const MlmsError& e) {
        io_.error(e.what());
    }
}

void ConsoleUI::run() {
    io_.out() << "Moderated Learning Management System (MLMS) v1.1\n";
    try {
        if (!app_.users.hasAnyUsers()) firstRunSetup();
        while (true) {
            auto user = loginScreen();
            if (!user) break;
            switch (user->role()) {
                case Role::Administrator: adminMenu(*user); break;
                case Role::Lecturer: lecturerMenu(*user); break;
                case Role::HoD: hodMenu(*user); break;
                case Role::Student: studentMenu(*user); break;
            }
        }
    } catch (const InputClosed&) {
        io_.out() << "\n";
    }
    io_.out() << "Goodbye.\n";
}

void ConsoleUI::firstRunSetup() {
    io_.heading("First-time setup");
    io_.out() << "No accounts exist yet. Create the administrator account.\n";
    while (true) {
        try {
            const std::string username = io_.readLine("Administrator username: ");
            const std::string name = io_.readLine("Full name: ");
            const std::string password = io_.readLine("Password (min 8 characters): ");
            app_.users.createInitialAdministrator(username, password, name);
            io_.success("Administrator account created. Please log in.");
            return;
        } catch (const MlmsError& e) {
            io_.error(e.what());
        }
    }
}

std::shared_ptr<User> ConsoleUI::loginScreen() {
    while (true) {
        io_.heading("Login");
        io_.out() << "1. Log in\n0. Exit\n";
        const int choice = io_.readInt("Choice: ");
        if (choice == 0) return nullptr;
        if (choice != 1) {
            io_.error("Unknown option.");
            continue;
        }
        const std::string username = io_.readLine("Username: ");
        const std::string password = io_.readLine("Password: ");
        try {
            auto user = app_.auth.login(username, password);
            io_.success("Welcome, " + user->displayName() + " (" + user->title() + ")");
            return user;
        } catch (const MlmsError& e) {
            io_.error(e.what());
        }
    }
}

void ConsoleUI::changeMyPassword(User& user) {
    const std::string current = io_.readLine("Current password: ");
    const std::string next = io_.readLine("New password (min 8 characters): ");
    const std::string again = io_.readLine("Repeat new password: ");
    if (next != again) throw ValidationError("The new passwords do not match");
    app_.auth.changePassword(user, current, next);
    io_.success("Password changed.");
}

// =================================================================== Administrator

void ConsoleUI::adminMenu(User& admin) {
    while (true) {
        io_.heading("Administrator menu");
        io_.out() << " 1. Create user account\n 2. List users\n 3. Activate / deactivate user\n"
                     " 4. Edit user details\n 5. Reset a user's password\n 6. Create department\n"
                     " 7. List departments\n 8. Create course\n 9. List courses\n"
                     "10. Assign lecturer to course\n11. Enrol student in course\n12. View audit log\n"
                     "13. Change my password\n 0. Log out\n";
        const int choice = io_.readInt("Choice: ");
        switch (choice) {
            case 1: attempt([&] { createUser(admin); }); break;
            case 2: attempt([&] { listUsers(admin); }); break;
            case 3: attempt([&] { setUserActive(admin); }); break;
            case 4: attempt([&] { editUser(admin); }); break;
            case 5: attempt([&] { resetUserPassword(admin); }); break;
            case 6: attempt([&] { createDepartment(admin); }); break;
            case 7: attempt([&] { listDepartments(); }); break;
            case 8: attempt([&] { createCourse(admin); }); break;
            case 9: attempt([&] { listCourses(); }); break;
            case 10: attempt([&] { assignLecturer(admin); }); break;
            case 11: attempt([&] { enrolStudent(admin); }); break;
            case 12: attempt([&] { showAudit(admin); }); break;
            case 13: attempt([&] { changeMyPassword(admin); }); break;
            case 0: return;
            default: io_.error("Unknown option.");
        }
    }
}

void ConsoleUI::createUser(const User& admin) {
    NewUserRequest request;
    io_.out() << "Role: 1=Administrator 2=Lecturer 3=Head of Department 4=Student\n";
    switch (io_.readInt("Role: ")) {
        case 1: request.role = Role::Administrator; break;
        case 2: request.role = Role::Lecturer; break;
        case 3: request.role = Role::HoD; break;
        case 4: request.role = Role::Student; break;
        default: throw ValidationError("Unknown role");
    }
    request.username = io_.readLine("Username: ");
    request.displayName = io_.readLine("Full name: ");
    request.password = io_.readLine("Initial password (min 8 characters): ");
    if (request.role != Role::Administrator) {
        listDepartments();
        request.departmentId = io_.readOptionalInt(request.role == Role::HoD ? "Department ID: "
                                                                              : "Department ID (blank for none): ");
    }
    if (request.role == Role::Student) request.studentNumber = io_.readLine("Student number: ");
    const int id = app_.users.createUser(admin, request);
    io_.success("User created with ID " + std::to_string(id));
}

void ConsoleUI::listUsers(const User& admin) {
    io_.out() << pad("ID", 6) << pad("Username", 16) << pad("Role", 15) << pad("Name", 28) << pad("Dept", 6)
              << pad("Student no.", 13) << "Active\n";
    for (const auto& u : app_.users.listUsers(admin)) {
        const auto& r = u->record();
        io_.out() << pad(std::to_string(r.id), 6) << pad(r.username, 16) << pad(toString(r.role), 15)
                  << pad(r.displayName, 28) << pad(r.departmentId ? std::to_string(*r.departmentId) : "-", 6)
                  << pad(r.studentNumber.value_or("-"), 13) << (r.active ? "yes" : "no") << "\n";
    }
}

void ConsoleUI::setUserActive(const User& admin) {
    const int id = io_.readInt("User ID: ");
    const bool activate = io_.confirm("Activate this account? (n = deactivate)");
    app_.users.setActive(admin, id, activate);
    io_.success(activate ? "Account activated." : "Account deactivated.");
}

void ConsoleUI::editUser(const User& admin) {
    const int id = io_.readInt("User ID: ");
    const std::string name = io_.readLine("New full name: ");
    listDepartments();
    const auto dept = io_.readOptionalInt("Department ID (blank for none): ");
    app_.users.updateDetails(admin, id, name, dept);
    io_.success("User updated.");
}

void ConsoleUI::resetUserPassword(const User& admin) {
    const int id = io_.readInt("User ID: ");
    const std::string password = io_.readLine("New password (min 8 characters): ");
    app_.users.resetPassword(admin, id, password);
    io_.success("Password reset and account unlocked.");
}

void ConsoleUI::createDepartment(const User& admin) {
    const int id = app_.courses.createDepartment(admin, io_.readLine("Department name: "));
    io_.success("Department created with ID " + std::to_string(id));
}

void ConsoleUI::listDepartments() {
    io_.out() << "Departments:\n";
    for (const Department& d : app_.courses.listDepartments()) {
        io_.out() << "  " << pad(std::to_string(d.id), 5) << d.name << "\n";
    }
}

void ConsoleUI::createCourse(const User& admin) {
    listDepartments();
    const int dept = io_.readInt("Department ID: ");
    const std::string code = io_.readLine("Course code: ");
    const std::string name = io_.readLine("Course name: ");
    const int id = app_.courses.createCourse(admin, dept, code, name);
    io_.success("Course created with ID " + std::to_string(id));
}

void ConsoleUI::listCourses() {
    io_.out() << pad("ID", 6) << pad("Code", 10) << pad("Dept", 6) << "Name\n";
    for (const Course& c : app_.courses.listCourses()) {
        io_.out() << pad(std::to_string(c.id), 6) << pad(c.code, 10) << pad(std::to_string(c.departmentId), 6)
                  << c.name << "\n";
    }
}

void ConsoleUI::assignLecturer(const User& admin) {
    const int lecturer = io_.readInt("Lecturer user ID: ");
    listCourses();
    const int course = io_.readInt("Course ID: ");
    app_.courses.assignLecturer(admin, lecturer, course);
    io_.success("Lecturer assigned to course.");
}

void ConsoleUI::enrolStudent(const User& admin) {
    const int student = io_.readInt("Student user ID: ");
    listCourses();
    const int course = io_.readInt("Course ID: ");
    app_.courses.enrolStudent(admin, student, course);
    io_.success("Student enrolled.");
}

void ConsoleUI::showAudit(const User& actor) {
    AuditFilter filter;
    filter.assignmentId = io_.readOptionalInt("Filter by assignment ID (blank = all): ");
    filter.studentId = io_.readOptionalInt("Filter by student user ID (blank = all): ");
    const auto entries = app_.audit.query(actor, filter);
    if (entries.empty()) {
        io_.out() << "No audit records found.\n";
        return;
    }
    for (const auto& e : entries) {
        const AuditRecord& r = e.record;
        io_.out() << "#" << r.id << "  " << r.timestamp << "  " << pad(r.eventType, 20) << " by " << e.actorName
                  << "  [" << r.entityType << " " << r.entityId << "]\n"
                  << "      old: " << r.oldValue.value_or("-") << "\n"
                  << "      new: " << r.newValue.value_or("-") << "\n"
                  << "      reason: " << r.reason << "\n";
    }
}

// =================================================================== Lecturer

void ConsoleUI::lecturerMenu(User& lecturer) {
    while (true) {
        io_.heading("Lecturer menu");
        io_.out() << "1. My courses\n2. Create assignment\n3. List my assignments\n"
                     "4. View submissions for an assignment\n5. Mark a submission\n"
                     "6. Correct a mark (pending or returned)\n7. Grant an individual extension\n"
                     "8. Extend the deadline for the whole class\n9. Change my password\n0. Log out\n";
        const int choice = io_.readInt("Choice: ");
        switch (choice) {
            case 1: attempt([&] { listMyCourses(lecturer); }); break;
            case 2: attempt([&] { createAssignment(lecturer); }); break;
            case 3: attempt([&] { listStaffAssignments(lecturer); }); break;
            case 4: attempt([&] { viewSubmissions(lecturer); }); break;
            case 5: attempt([&] { markSubmission(lecturer); }); break;
            case 6: attempt([&] { correctMark(lecturer); }); break;
            case 7: attempt([&] { grantExtension(lecturer); }); break;
            case 8: attempt([&] { grantClassExtension(lecturer); }); break;
            case 9: attempt([&] { changeMyPassword(lecturer); }); break;
            case 0: return;
            default: io_.error("Unknown option.");
        }
    }
}

void ConsoleUI::listMyCourses(const User& user) {
    const auto courses = app_.courses.coursesFor(user);
    if (courses.empty()) io_.out() << "No courses.\n";
    for (const Course& c : courses) io_.out() << "  " << pad(std::to_string(c.id), 5) << pad(c.code, 9) << c.name << "\n";
}

void ConsoleUI::createAssignment(const User& lecturer) {
    listMyCourses(lecturer);
    const int course = io_.readInt("Course ID: ");
    const std::string title = io_.readLine("Title: ");
    const std::string description = io_.readLine("Description: ");
    const std::string deadline = io_.readLine("Deadline (YYYY-MM-DD HH:MM): ");
    const double maxMark = io_.readDouble("Maximum mark: ");
    const int id = app_.assignments.createAssignment(lecturer, course, title, description, deadline, maxMark);
    io_.success("Assignment created with ID " + std::to_string(id));
}

void ConsoleUI::listStaffAssignments(const User& staff) {
    const auto list = app_.assignments.listVisibleTo(staff);
    if (list.empty()) {
        io_.out() << "No assignments.\n";
        return;
    }
    io_.out() << pad("ID", 5) << pad("Course", 8) << pad("Title", 32) << pad("Deadline", 21) << "Max\n";
    for (const auto& ca : list) {
        const std::string deadline = ca.classExtension ? ca.classExtension->revisedDeadline : ca.assignment.deadline;
        io_.out() << pad(std::to_string(ca.assignment.id), 5) << pad(ca.course.code, 8)
                  << pad(ca.assignment.title, 32) << pad(deadline, 21) << formatMark(ca.assignment.maxMark) << "\n";
        if (ca.classExtension) {
            io_.out() << "      class extension (was " << ca.assignment.deadline << "): " << ca.classExtension->reason
                      << "\n";
        }
    }
}

void ConsoleUI::viewSubmissions(const User& lecturer) {
    listStaffAssignments(lecturer);
    const int assignmentId = io_.readInt("Assignment ID: ");
    const auto views = app_.marking.submissionsFor(lecturer, assignmentId);
    if (views.empty()) {
        io_.out() << "No submissions yet.\n";
        return;
    }
    io_.out() << pad("Sub", 5) << pad("Student", 22) << pad("Status", 10) << pad("Submitted", 21) << pad("Ver", 4)
              << pad("Late", 5) << pad("Mark (raw/final)", 18) << "Mark status\n";
    for (const auto& v : views) {
        std::string markText = "-";
        std::string markStatus = "NOT MARKED";
        if (v.mark) {
            markText = formatMark(v.mark->rawMark) + "/" + formatMark(v.mark->finalMark);
            markStatus = toString(v.mark->status);
        }
        io_.out() << pad(std::to_string(v.submission.id), 5) << pad(v.studentName, 22)
                  << pad(toString(v.submission.status), 10) << pad(v.latest ? v.latest->submittedAt : "-", 21)
                  << pad(std::to_string(v.versionCount), 4) << pad(std::to_string(v.lateDays), 5) << pad(markText, 18)
                  << markStatus << "\n";
        if (v.latest) io_.out() << "       content: " << v.latest->contentReference << "\n";
    }
}

void ConsoleUI::markSubmission(const User& lecturer) {
    const int submissionId = io_.readInt("Submission ID: ");
    const double raw = io_.readDouble("Raw mark: ");
    const std::string feedback = io_.readLine("Written feedback: ");
    const Mark mark = app_.marking.markSubmission(lecturer, submissionId, raw, feedback);
    io_.success("Saved. Raw " + formatMark(mark.rawMark) + ", penalty " + formatMark(mark.penalty) + ", final " +
                formatMark(mark.finalMark) + ". Awaiting HoD moderation - not visible to the student yet.");
}

void ConsoleUI::correctMark(const User& lecturer) {
    const int submissionId = io_.readInt("Submission ID: ");
    const double raw = io_.readDouble("Corrected raw mark: ");
    const std::string feedback = io_.readLine("New feedback (blank = keep current): ");
    const std::string reason = io_.readLine("Reason for the change: ");
    const Mark mark = app_.marking.correctMark(lecturer, submissionId, raw, feedback, reason);
    io_.success("Mark corrected to final " + formatMark(mark.finalMark) + " and sent back for moderation.");
}

void ConsoleUI::grantExtension(const User& lecturer) {
    listStaffAssignments(lecturer);
    const int assignmentId = io_.readInt("Assignment ID: ");
    const int studentId = io_.readInt("Student user ID: ");
    const std::string deadline = io_.readLine("Revised deadline (YYYY-MM-DD HH:MM): ");
    const std::string reason = io_.readLine("Reason: ");
    const std::string saved = app_.extensions.grantIndividual(lecturer, assignmentId, studentId, deadline, reason);
    io_.success("Extension granted until " + saved);
}

void ConsoleUI::grantClassExtension(const User& lecturer) {
    listStaffAssignments(lecturer);
    const int assignmentId = io_.readInt("Assignment ID: ");
    const std::string deadline = io_.readLine("New deadline for the whole class (YYYY-MM-DD HH:MM): ");
    const std::string reason = io_.readLine("Reason: ");
    const std::string saved = app_.extensions.grantClass(lecturer, assignmentId, deadline, reason);
    io_.success("Deadline extended for the whole class until " + saved);
}

// =================================================================== Head of Department

void ConsoleUI::hodMenu(User& hod) {
    while (true) {
        io_.heading("Head of Department menu");
        io_.out() << "1. Marks awaiting moderation\n2. Approve a mark\n3. Amend a mark\n"
                     "4. Return a mark to the lecturer\n5. Release marks for an assignment\n"
                     "6. Department audit log\n7. Change my password\n0. Log out\n";
        const int choice = io_.readInt("Choice: ");
        switch (choice) {
            case 1: attempt([&] { showPending(hod); }); break;
            case 2: attempt([&] { approveMark(hod); }); break;
            case 3: attempt([&] { amendMark(hod); }); break;
            case 4: attempt([&] { returnMark(hod); }); break;
            case 5: attempt([&] { releaseMarks(hod); }); break;
            case 6: attempt([&] { showAudit(hod); }); break;
            case 7: attempt([&] { changeMyPassword(hod); }); break;
            case 0: return;
            default: io_.error("Unknown option.");
        }
    }
}

void ConsoleUI::showPending(const User& hod) {
    const auto items = app_.moderation.pending(hod);
    if (items.empty()) {
        io_.out() << "No marks are awaiting moderation.\n";
        return;
    }
    for (const auto& item : items) {
        io_.out() << "Mark #" << item.mark.id << "  " << item.course.code << " / " << item.assignment.title << "\n"
                  << "   student: " << item.studentName << "\n"
                  << "   raw " << formatMark(item.mark.rawMark) << " - penalty " << formatMark(item.mark.penalty)
                  << " = final " << formatMark(item.mark.finalMark) << " out of "
                  << formatMark(item.assignment.maxMark) << "  (late days: " << item.lateDays << ")\n";
        if (item.deadline.isExtended()) {
            io_.out() << "   " << (item.deadline.classExtension ? "class" : "individual") << " extension to "
                      << item.deadline.effective << " - " << item.deadline.extensionReason << "\n";
        }
        io_.out() << "   feedback: " << item.mark.feedback << "\n";
    }
}

void ConsoleUI::approveMark(const User& hod) {
    const int markId = io_.readInt("Mark ID: ");
    const std::string comment = io_.readLine("Comment (optional): ");
    app_.moderation.approve(hod, markId, comment);
    io_.success("Mark approved.");
}

void ConsoleUI::amendMark(const User& hod) {
    const int markId = io_.readInt("Mark ID: ");
    const double raw = io_.readDouble("New raw mark: ");
    const std::string reason = io_.readLine("Reason for the amendment: ");
    app_.moderation.amend(hod, markId, raw, reason);
    io_.success("Mark amended and approved.");
}

void ConsoleUI::returnMark(const User& hod) {
    const int markId = io_.readInt("Mark ID: ");
    const std::string reason = io_.readLine("Reason for returning: ");
    app_.moderation.returnForCorrection(hod, markId, reason);
    io_.success("Mark returned to the lecturer.");
}

void ConsoleUI::releaseMarks(const User& hod) {
    listStaffAssignments(hod);
    const int assignmentId = io_.readInt("Assignment ID: ");
    const ReleaseSummary s = app_.release.summarise(hod, assignmentId);
    io_.out() << "Submissions: " << s.submissions << ", approved: " << s.approved << ", not marked: " << s.unmarked
              << ", not approved: " << s.awaitingOrReturned << ", already released: " << s.released << "\n";
    if (!io_.confirm("Release the approved marks to students?")) return;
    const int count = app_.release.releaseAssignment(hod, assignmentId);
    io_.success(std::to_string(count) + " mark(s) released.");
}

// =================================================================== Student

void ConsoleUI::studentMenu(User& student) {
    while (true) {
        io_.heading("Student menu");
        io_.out() << "1. My assignments\n2. Submit or resubmit work\n3. My released marks and feedback\n"
                     "4. Change my password\n0. Log out\n";
        const int choice = io_.readInt("Choice: ");
        switch (choice) {
            case 1: attempt([&] { listStudentAssignments(student); }); break;
            case 2: attempt([&] { submitWork(student); }); break;
            case 3: attempt([&] { viewResults(student); }); break;
            case 4: attempt([&] { changeMyPassword(student); }); break;
            case 0: return;
            default: io_.error("Unknown option.");
        }
    }
}

void ConsoleUI::listStudentAssignments(const User& student) {
    const auto list = app_.assignments.listForStudent(student);
    if (list.empty()) {
        io_.out() << "You have no assignments.\n";
        return;
    }
    io_.out() << pad("ID", 5) << pad("Course", 8) << pad("Title", 30) << pad("Deadline", 21) << pad("Max", 8)
              << "My status\n";
    for (const auto& v : list) {
        std::string status = "not submitted";
        if (v.submission) {
            status = toString(v.submission->status) + " (" + std::to_string(v.versionCount) + " version(s))";
            if (v.resultReleased) status += ", result released";
        }
        io_.out() << pad(std::to_string(v.assignment.id), 5) << pad(v.course.code, 8) << pad(v.assignment.title, 30)
                  << pad(v.deadline.effective, 21) << pad(formatMark(v.assignment.maxMark), 8) << status << "\n";
        if (v.deadline.individualExtension) {
            io_.out() << "      extended for you (was " << v.deadline.original << ")\n";
        } else if (v.deadline.classExtension) {
            io_.out() << "      extended for the whole class (was " << v.deadline.original << ")\n";
        }
    }
}

void ConsoleUI::submitWork(const User& student) {
    listStudentAssignments(student);
    const int assignmentId = io_.readInt("Assignment ID: ");
    const std::string content = io_.readLine("Your answer, or the path of your file: ");
    const SubmitResult r = app_.submissions.submit(student, assignmentId, content);
    io_.success(std::string(r.resubmission ? "Resubmitted" : "Submitted") + " at " + r.submittedAt + " (version " +
                std::to_string(r.versionNumber) + "), status " + toString(r.status));
    if (r.lateDays > 0) {
        io_.out() << "  This work is " << r.lateDays << " day(s) late (deadline " << r.effectiveDeadline
                  << "). A late penalty of 10% per day will apply.\n";
    }
    if (r.beyondLateWindow) {
        io_.out() << "  More than 3 days late: the work is recorded but the final mark will be 0.\n";
    }
}

void ConsoleUI::viewResults(const User& student) {
    const auto results = app_.release.resultsFor(student);
    if (results.empty()) {
        io_.out() << "No marks have been released to you yet.\n";
        return;
    }
    for (const auto& r : results) {
        io_.out() << r.course.code << " - " << r.assignment.title << "\n"
                  << "   mark: " << formatMark(r.mark.finalMark) << " / " << formatMark(r.assignment.maxMark);
        if (r.mark.penalty > 0) io_.out() << "  (late penalty " << formatMark(r.mark.penalty) << ")";
        io_.out() << "\n   feedback: " << (r.mark.feedback.empty() ? "-" : r.mark.feedback) << "\n";
    }
}

}  // namespace mlms
