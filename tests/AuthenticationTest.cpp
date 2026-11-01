
#include <iostream>
#include <memory>
#include <string>

#include "domain/RoleUsers.h"
#include "repositories/UserRepository.h"
#include "security/PasswordHasher.h"
#include "services/AuthenticationService.h"

using namespace std;

// A temporary repository used only for testing
class TestUserRepository : public UserRepository
{
private:
    string username;
    string passwordHash;
    bool active;
    int failedLoginCount;
    string lockedUntil;

public:
    TestUserRepository(
        const string& username,
        const string& passwordHash,
        bool active)
    {
        this->username = username;
        this->passwordHash = passwordHash;
        this->active = active;
        failedLoginCount = 0;
        lockedUntil = "";
    }

    unique_ptr<User> findByUsername(const string& username);

    bool updateLoginState(
        int userId,
        int failedLoginCount,
        const string& lockedUntil);

    int getFailedLoginCount() const;
    string getLockedUntil() const;
};


// Find the test user
unique_ptr<User> TestUserRepository::findByUsername(
    const string& username)
{
    if (username != this->username)
    {
        return nullptr;
    }

    return make_unique<Student>(
        1,
        this->username,
        passwordHash,
        active,
        1,
        failedLoginCount,
        lockedUntil,
        "TEST001"
    );
}


// Update the test user's login information
bool TestUserRepository::updateLoginState(
    int userId,
    int failedLoginCount,
    const string& lockedUntil)
{
    if (userId != 1)
    {
        return false;
    }

    this->failedLoginCount = failedLoginCount;
    this->lockedUntil = lockedUntil;

    return true;
}


int TestUserRepository::getFailedLoginCount() const
{
    return failedLoginCount;
}


string TestUserRepository::getLockedUntil() const
{
    return lockedUntil;
}


// Main authentication tests
int main()
{
    cout << "MLMS AUTHENTICATION TESTS" << endl;
    cout << "-------------------------" << endl;

    string correctPassword = "TestPassword123";

    string passwordHash =
        PasswordHasher::hashPassword(correctPassword);

    TestUserRepository repository(
        "test.student",
        passwordHash,
        true
    );

    AuthenticationService authentication(repository);

    // TEST 1: Correct password
    cout << "\nTEST 1: Correct password" << endl;

    unique_ptr<User> user = authentication.login(
        "test.student",
        correctPassword
    );

    if (user != nullptr)
    {
        cout << "PASSED: Login successful." << endl;
    }
    else
    {
        cout << "FAILED: Correct password was rejected." << endl;
        return 1;
    }

    // TEST 2: Incorrect password
    cout << "\nTEST 2: Incorrect password" << endl;

    user = authentication.login(
        "test.student",
        "WrongPassword"
    );

    if (user == nullptr &&
        repository.getFailedLoginCount() == 1)
    {
        cout << "PASSED: Failed attempt recorded." << endl;
    }
    else
    {
        cout << "FAILED: Failed attempt was not recorded." << endl;
        return 1;
    }

    // Reset the test account before testing lockout
    repository.updateLoginState(1, 0, "");

    // TEST 3: Five consecutive incorrect passwords
    cout << "\nTEST 3: Five incorrect passwords" << endl;

    for (int attempt = 1; attempt <= 5; attempt++)
    {
        authentication.login(
            "test.student",
            "WrongPassword"
        );
    }

    if (repository.getFailedLoginCount() == 5 &&
        !repository.getLockedUntil().empty())
    {
        cout << "PASSED: Account locked after five failures." << endl;
        cout << "Lockout expires at UTC: "
            << repository.getLockedUntil() << endl;
    }
    else
    {
        cout << "FAILED: Account was not locked correctly." << endl;
        return 1;
    }

    // TEST 4: Correct password while account is locked
    cout << "\nTEST 4: Login during lockout" << endl;

    user = authentication.login(
        "test.student",
        correctPassword
    );

    if (user == nullptr)
    {
        cout << "PASSED: Locked account cannot log in." << endl;
    }
    else
    {
        cout << "FAILED: Locked account was allowed to log in." << endl;
        return 1;
    }

    cout << "\n-------------------------" << endl;
    cout << "ALL AUTHENTICATION TESTS PASSED!" << endl;

    return 0;
}
