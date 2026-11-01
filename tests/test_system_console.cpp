// System tests derived from the use-case specifications. Each test drives the
// real console (ConsoleUI) with scripted keyboard input and checks the output,
// so the presentation, service and database layers are exercised together.
#include <sstream>

#include "TestApp.h"
#include "ui/ConsoleUI.h"

using namespace mlms;
using namespace mlms::test;

class ConsoleSystemTest : public LmsTest {
protected:
    // Runs one console session; "0" at the end exits from the login screen.
    std::string session(const std::string& script) {
        std::istringstream in(script + "0\n");
        std::ostringstream out;
        ConsoleUI ui(*app, in, out);
        ui.run();
        return out.str();
    }
    static std::string login(const std::string& user) { return "1\n" + user + "\nPassword123\n"; }
    static bool has(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }
};

// ST-01: UC-01 main flow + A1 resubmission
TEST_F(ConsoleSystemTest, StudentSubmitsAndResubmitsBeforeDeadline) {
    const int a = newAssignment(kDay);
    setTime(kHour);
    const std::string id = std::to_string(a);
    const std::string out =
        session(login("student") + "2\n" + id + "\nfirst draft\n2\n" + id + "\nfinal answer\n1\n0\n");
    EXPECT_TRUE(has(out, "[OK] Submitted at"));
    EXPECT_TRUE(has(out, "[OK] Resubmitted at"));
    EXPECT_TRUE(has(out, "(version 2), status SUBMITTED"));
    EXPECT_TRUE(has(out, "SUBMITTED (2 version(s))"));
}

// ST-02: UC-01 A3 late submission and E1 more than three days late
TEST_F(ConsoleSystemTest, LateSubmissionsWarnTheStudent) {
    const int a = newAssignment(kDay);
    const std::string id = std::to_string(a);
    setTime(kDay + 2 * kHour);
    std::string out = session(login("student") + "2\n" + id + "\nlate work\n0\n");
    EXPECT_TRUE(has(out, "status LATE"));
    EXPECT_TRUE(has(out, "1 day(s) late"));

    setTime(kDay + 5 * kDay);
    out = session(login("student2") + "2\n" + id + "\nvery late\n0\n");
    EXPECT_TRUE(has(out, "final mark will be 0"));
}

// ST-03: UC-01 E2 invalid assignment, and FR-16 bad input does not crash
TEST_F(ConsoleSystemTest, InvalidInputIsReportedAndMenuContinues) {
    newAssignment(kDay);
    setTime(kHour);
    const std::string out = session(login("student") + "2\n999\nanswer\n2\nabc\n1\n   \n9\n0\n");
    EXPECT_TRUE(has(out, "[!] Assignment not found"));
    EXPECT_TRUE(has(out, "Please enter a whole number"));
    EXPECT_TRUE(has(out, "[!] Submission content is required"));
    EXPECT_TRUE(has(out, "[!] Unknown option"));
    EXPECT_TRUE(has(out, "Goodbye."));
}

// ST-04: UC-02 main flow, E1 invalid mark, then UC-03 approve + release, student views result
TEST_F(ConsoleSystemTest, EndToEndSubmissionToReleasedMark) {
    const int a = newAssignment(kDay);
    const std::string id = std::to_string(a);
    const int sub = submitAt(*student, a, kDay + 30 * kHour).submissionId;  // 2 days late
    const std::string subId = std::to_string(sub);

    setTime(5 * kDay);
    std::string out = session(login("lecturer") + "4\n" + id + "\n5\n" + subId + "\n60\nx\n5\n" + subId +
                              "\n40\nGood loops\n0\n");
    EXPECT_TRUE(has(out, "[!] Mark must be between 0 and 50.00"));
    EXPECT_TRUE(has(out, "Raw 40.00, penalty 10.00, final 30.00"));
    EXPECT_TRUE(has(out, "not visible to the student yet"));

    out = session(login("student") + "3\n0\n");
    EXPECT_TRUE(has(out, "No marks have been released to you yet."));

    const std::string markId = std::to_string(app->markRepo.findBySubmission(sub)->id);
    out = session(login("hod") + "1\n2\n" + markId + "\n\n5\n" + id + "\ny\n0\n");
    EXPECT_TRUE(has(out, "late days: 2"));
    EXPECT_TRUE(has(out, "[OK] Mark approved."));
    EXPECT_TRUE(has(out, "[OK] 1 mark(s) released."));

    out = session(login("student") + "3\n0\n");
    EXPECT_TRUE(has(out, "mark: 30.00 / 50.00  (late penalty 10.00)"));
    EXPECT_TRUE(has(out, "feedback: Good loops"));
}

// ST-05: UC-03 A1 return for correction, then lecturer correction
TEST_F(ConsoleSystemTest, HodReturnsMarkAndLecturerCorrectsIt) {
    const int a = newAssignment(kDay);
    const int sub = submitAt(*student, a, kHour).submissionId;
    const Mark m = markAt(sub, 20, 2 * kDay);
    std::string out = session(login("hod") + "4\n" + std::to_string(m.id) + "\nRecheck question 3\n0\n");
    EXPECT_TRUE(has(out, "[OK] Mark returned"));
    out = session(login("lecturer") + "6\n" + std::to_string(sub) + "\n25\n\nQ3 was correct\n0\n");
    EXPECT_TRUE(has(out, "sent back for moderation"));
    EXPECT_EQ(app->markRepo.findById(m.id)->status, MarkStatus::AwaitingModeration);
}

// ST-06: UC-03 A2 mixed batch and E1 wrong department
TEST_F(ConsoleSystemTest, ReleaseRefusedForMixedBatchAndOtherDepartment) {
    const int a = newAssignment(kDay);
    const int s1 = submitAt(*student, a, kHour).submissionId;
    submitAt(*student2, a, kHour);
    const Mark m = markAt(s1, 30, 2 * kDay);
    app->moderation.approve(*hod, m.id, "");
    std::string out = session(login("hod") + "5\n" + std::to_string(a) + "\ny\n0\n");
    EXPECT_TRUE(has(out, "Release refused: 1 submission(s) not marked"));

    out = session(login("hod2") + "2\n" + std::to_string(m.id) + "\n\n0\n");
    EXPECT_TRUE(has(out, "not in your department"));
}

// ST-07: UC-02 E2 unauthorised lecturer
TEST_F(ConsoleSystemTest, LecturerOfAnotherCourseIsDenied) {
    const int a = newAssignment(kDay);
    const int sub = submitAt(*student, a, kHour).submissionId;
    setTime(2 * kDay);
    const std::string out = session(login("lecturer2") + "5\n" + std::to_string(sub) + "\n40\nx\n0\n");
    EXPECT_TRUE(has(out, "not assigned to this course"));
    EXPECT_FALSE(app->markRepo.findBySubmission(sub).has_value());
}

// ST-08: authentication and account lockout through the console
TEST_F(ConsoleSystemTest, LoginFailuresAndLockout) {
    std::string script;
    for (int i = 0; i < 5; ++i) script += "1\nstudent\nwrong\n";
    script += login("student");
    const std::string out = session(script);
    EXPECT_TRUE(has(out, "Invalid username or password"));
    EXPECT_TRUE(has(out, "Account locked"));
    EXPECT_FALSE(has(out, "Welcome, student name"));
}

// ST-09: administrator setup flow
TEST_F(ConsoleSystemTest, AdministratorCreatesStudentAndEnrolsThem) {
    const std::string out = session("1\nadmin\nAdminPass1\n1\n4\nnewkid\nNew Kid\nPassword123\n\n2026999\n"
                                    "2\n0\n");
    EXPECT_TRUE(has(out, "[OK] User created with ID"));
    auto kid = app->userRepo.findByUsername("newkid");
    ASSERT_NE(kid, nullptr);
    const std::string out2 = session("1\nadmin\nAdminPass1\n11\n" + std::to_string(kid->id()) + "\n" +
                                     std::to_string(course) + "\n0\n");
    EXPECT_TRUE(has(out2, "[OK] Student enrolled."));
    EXPECT_TRUE(app->courseRepo.isEnrolled(kid->id(), course));
}

// ST-10: first run creates the administrator
TEST(ConsoleFirstRun, CreatesAdministratorOnEmptyDatabase) {
    AppContext app(":memory:", std::make_unique<FixedClock>(toSeconds("2026-11-02 08:00")),
                   std::make_unique<SodiumPasswordHasher>(SodiumPasswordHasher::fastForTesting()));
    std::istringstream in("boss\nThe Boss\nshort\nboss\nThe Boss\nLongEnough1\n1\nboss\nLongEnough1\n0\n0\n");
    std::ostringstream out;
    ConsoleUI ui(app, in, out);
    ui.run();
    EXPECT_NE(out.str().find("at least 8 characters"), std::string::npos);
    EXPECT_NE(out.str().find("Welcome, The Boss (Administrator)"), std::string::npos);
}
