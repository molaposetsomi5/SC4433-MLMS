
#include <iostream>
#include <cassert>
#include <vector>

#include "domain/RoleUsers.h"
#include "repositories/UserManagementRepository.h"
#include "services/UserManagementService.h"
#include "security/PasswordHasher.h"

using namespace std;


// Fake repository used for testing
class TestUserManagementRepository :
    public UserManagementRepository
{
public:
    vector<UserSummary> users;
    string savedPasswordHash;
    int createCalls;

    TestUserManagementRepository()
    {
        createCalls = 0;
    }

    bool createUser(
        const string& username,
        const string& passwordHash,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber)
    {
        createCalls++;

        savedPasswordHash = passwordHash;

        UserSummary user;

        user.userId = static_cast<int>(users.size()) + 1;
        user.username = username;
        user.displayName = displayName;
        user.role = role;
        user.active = true;
        user.hasDepartment = hasDepartment;
        user.departmentId = departmentId;
        user.studentNumber = studentNumber;

        users.push_back(user);

        return true;
    }

    vector<UserSummary> getAllUsers()
    {
        return users;
    }

    vector<UserSummary> searchUsers(
        const string& searchTerm)
    {
        vector<UserSummary> results;

        for (int i = 0; i < users.size(); i++)
        {
            if (users[i].username.find(searchTerm) != string::npos ||
                users[i].displayName.find(searchTerm) != string::npos)
            {
                results.push_back(users[i]);
            }
        }

        return results;
    }
};


int main()
{
    TestUserManagementRepository repository;

    AuthorizationService authorizationService;

    UserManagementService service(
        repository,
        authorizationService
    );


    // Create test administrator
    Administrator administrator(
        1,
        "admin",
        "adminHash",
        true,
        1,
        0,
        ""
    );


    // ==========================================
    // TEST 1: ADMINISTRATOR CREATES A USER
    // ==========================================

    bool created = service.createUser(
        administrator,
        "test.lecturer",
        "TestPassword123",
        "Test Lecturer",
        Role::LECTURER,
        true,
        1,
        ""
    );

    assert(created == true);
    assert(repository.createCalls == 1);
    assert(repository.users.size() == 1);

    cout << "Administrator user creation: PASSED" << endl;


    // ==========================================
    // TEST 2: PASSWORD IS HASHED
    // ==========================================

    assert(repository.savedPasswordHash != "");
    assert(repository.savedPasswordHash != "TestPassword123");

    bool passwordCorrect = PasswordHasher::verifyPassword(
        "TestPassword123",
        repository.savedPasswordHash
    );

    assert(passwordCorrect == true);

    cout << "Password hashing: PASSED" << endl;


    // ==========================================
    // TEST 3: ADMINISTRATOR LISTS USERS
    // ==========================================

    vector<UserSummary> allUsers =
        service.getAllUsers(administrator);

    assert(allUsers.size() == 1);
    assert(allUsers[0].username == "test.lecturer");
    assert(allUsers[0].displayName == "Test Lecturer");

    cout << "Administrator user listing: PASSED" << endl;


    // ==========================================
    // TEST 4: ADMINISTRATOR SEARCHES USERS
    // ==========================================

    vector<UserSummary> searchResults =
        service.searchUsers(administrator, "lecturer");

    assert(searchResults.size() == 1);
    assert(searchResults[0].username == "test.lecturer");

    cout << "Administrator user search: PASSED" << endl;


    // ==========================================
    // TEST 5: STUDENT CANNOT MANAGE USERS
    // ==========================================

    Student student(
        2,
        "test.student",
        "studentHash",
        true,
        1,
        0,
        "",
        "TEST001"
    );

    bool studentCreated = service.createUser(
        student,
        "another.user",
        "AnotherPassword123",
        "Another User",
        Role::STUDENT,
        true,
        1,
        "TEST002"
    );

    assert(studentCreated == false);
    assert(repository.createCalls == 1);

    vector<UserSummary> studentUsers =
        service.getAllUsers(student);

    assert(studentUsers.empty());

    vector<UserSummary> studentSearch =
        service.searchUsers(student, "lecturer");

    assert(studentSearch.empty());

    cout << "Student access restriction: PASSED" << endl;


    // ==========================================
    // TEST 6: EMPTY REQUIRED FIELDS ARE REJECTED
    // ==========================================

    bool emptyUsername = service.createUser(
        administrator,
        "",
        "Password123",
        "Another User",
        Role::LECTURER,
        true,
        1,
        ""
    );

    assert(emptyUsername == false);

    bool emptyPassword = service.createUser(
        administrator,
        "another.user",
        "",
        "Another User",
        Role::LECTURER,
        true,
        1,
        ""
    );

    assert(emptyPassword == false);

    bool emptyDisplayName = service.createUser(
        administrator,
        "another.user",
        "Password123",
        "",
        Role::LECTURER,
        true,
        1,
        ""
    );

    assert(emptyDisplayName == false);

    assert(repository.createCalls == 1);

    cout << "Required field validation: PASSED" << endl;


    // ==========================================
    // TEST 7: STUDENT NUMBER IS REQUIRED
    // ==========================================

    bool missingStudentNumber = service.createUser(
        administrator,
        "new.student",
        "Password123",
        "New Student",
        Role::STUDENT,
        true,
        1,
        ""
    );

    assert(missingStudentNumber == false);
    assert(repository.createCalls == 1);

    cout << "Student number validation: PASSED" << endl;


    cout << "\n========== ALL USER MANAGEMENT TESTS PASSED =========="
        << endl;

    return 0;
}
