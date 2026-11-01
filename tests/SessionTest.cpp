
#include <iostream>
#include <memory>

#include "domain/RoleUsers.h"
#include "services/Session.h"

using namespace std;

int main()
{
    cout << "MLMS SESSION TESTS" << endl;
    cout << "------------------" << endl;

    Session session;

    // TEST 1: A new session should be empty
    cout << "\nTEST 1: Initial session state" << endl;

    if (!session.isAuthenticated() &&
        session.getCurrentUser() == nullptr)
    {
        cout << "PASSED: No user is logged in." << endl;
    }
    else
    {
        cout << "FAILED: New session should be empty." << endl;
        return 1;
    }

    // TEST 2: Start a session
    cout << "\nTEST 2: Start session" << endl;

    unique_ptr<User> student = make_unique<Student>(
        1,
        "test.student",
        "test_hash",
        true,
        1,
        0,
        "",
        "TEST001"
    );

    session.startSession(move(student));

    if (session.isAuthenticated())
    {
        cout << "PASSED: Session started successfully." << endl;
    }
    else
    {
        cout << "FAILED: Session did not start." << endl;
        return 1;
    }

    // TEST 3: Check the current user
    cout << "\nTEST 3: Current user" << endl;

    User* currentUser = session.getCurrentUser();

    if (currentUser != nullptr &&
        currentUser->getUsername() == "test.student" &&
        currentUser->getRole() == Role::STUDENT)
    {
        cout << "PASSED: Correct user is stored in session." << endl;
    }
    else
    {
        cout << "FAILED: Incorrect current user." << endl;
        return 1;
    }

    // TEST 4: End the session
    cout << "\nTEST 4: End session" << endl;

    session.endSession();

    if (!session.isAuthenticated() &&
        session.getCurrentUser() == nullptr)
    {
        cout << "PASSED: Session ended successfully." << endl;
    }
    else
    {
        cout << "FAILED: Session was not cleared." << endl;
        return 1;
    }

    cout << "\nALL SESSION TESTS PASSED!" << endl;

    return 0;
}
