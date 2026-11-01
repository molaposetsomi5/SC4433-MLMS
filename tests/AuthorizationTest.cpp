
#include <iostream>

#include "domain/RoleUsers.h"
#include "services/AuthorizationService.h"

using namespace std;

int main()
{
    cout << "MLMS AUTHORIZATION TESTS" << endl;
    cout << "------------------------" << endl;

    AuthorizationService authorization;

    // Create test users for all four roles
    Administrator admin(
        1, "admin", "hash", true, 1, 0, ""
    );

    Lecturer lecturer(
        2, "lecturer", "hash", true, 1, 0, ""
    );

    HoD hod(
        3, "hod", "hash", true, 1, 0, ""
    );

    Student student(
        4, "student", "hash", true, 1, 0, "", "TEST001"
    );

    bool allPassed = true;

    // TEST 1: Administrator permissions
    cout << "\nTEST 1: Administrator permissions" << endl;

    bool adminCanManageUsers = authorization.hasPermission(
        admin, Permission::MANAGE_USERS
    );

    bool adminCanModerateMarks = authorization.hasPermission(
        admin, Permission::MODERATE_MARKS
    );

    if (adminCanManageUsers && !adminCanModerateMarks)
    {
        cout << "PASSED: Administrator permissions are correct." << endl;
    }
    else
    {
        cout << "FAILED: Administrator permissions are incorrect." << endl;
        allPassed = false;
    }

    // TEST 2: Lecturer permissions
    cout << "\nTEST 2: Lecturer permissions" << endl;

    bool lecturerCanCreateAssignments = authorization.hasPermission(
        lecturer, Permission::CREATE_ASSIGNMENTS
    );

    bool lecturerCanReleaseMarks = authorization.hasPermission(
        lecturer, Permission::RELEASE_MARKS
    );

    if (lecturerCanCreateAssignments && !lecturerCanReleaseMarks)
    {
        cout << "PASSED: Lecturer permissions are correct." << endl;
    }
    else
    {
        cout << "FAILED: Lecturer permissions are incorrect." << endl;
        allPassed = false;
    }

    // TEST 3: HoD permissions
    cout << "\nTEST 3: HoD permissions" << endl;

    bool hodCanModerateMarks = authorization.hasPermission(
        hod, Permission::MODERATE_MARKS
    );

    bool hodCanReleaseMarks = authorization.hasPermission(
        hod, Permission::RELEASE_MARKS
    );

    bool hodCanGradeSubmissions = authorization.hasPermission(
        hod, Permission::GRADE_SUBMISSIONS
    );

    if (hodCanModerateMarks &&
        hodCanReleaseMarks &&
        !hodCanGradeSubmissions)
    {
        cout << "PASSED: HoD permissions are correct." << endl;
    }
    else
    {
        cout << "FAILED: HoD permissions are incorrect." << endl;
        allPassed = false;
    }

    // TEST 4: Student permissions
    cout << "\nTEST 4: Student permissions" << endl;

    bool studentCanSubmit = authorization.hasPermission(
        student, Permission::SUBMIT_ASSIGNMENT
    );

    bool studentCanManageUsers = authorization.hasPermission(
        student, Permission::MANAGE_USERS
    );

    if (studentCanSubmit && !studentCanManageUsers)
    {
        cout << "PASSED: Student permissions are correct." << endl;
    }
    else
    {
        cout << "FAILED: Student permissions are incorrect." << endl;
        allPassed = false;
    }

    // TEST 5: Check a permission denied to all four roles
    cout << "\nTEST 5: Permission restrictions" << endl;

    bool adminDenied = !authorization.hasPermission(
        admin, Permission::SUBMIT_ASSIGNMENT
    );

    bool lecturerDenied = !authorization.hasPermission(
        lecturer, Permission::VIEW_OWN_RESULTS
    );

    bool hodDenied = !authorization.hasPermission(
        hod, Permission::SUBMIT_ASSIGNMENT
    );

    bool studentDenied = !authorization.hasPermission(
        student, Permission::MANAGE_DEPARTMENTS
    );

    if (adminDenied && lecturerDenied &&
        hodDenied && studentDenied)
    {
        cout << "PASSED: Restricted actions are denied." << endl;
    }
    else
    {
        cout << "FAILED: An unauthorized action was allowed." << endl;
        allPassed = false;
    }

    cout << "\n------------------------" << endl;

    if (allPassed)
    {
        cout << "ALL AUTHORIZATION TESTS PASSED!" << endl;
        return 0;
    }

    cout << "SOME AUTHORIZATION TESTS FAILED!" << endl;
    return 1;
}
