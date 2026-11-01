
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

#include "sqlite3.h"
#include "repositories/SQLiteUserRepository.h"
#include "security/PasswordHasher.h"
#include "domain/RoleUsers.h"

using namespace std;


int main()
{
    sqlite3* database = nullptr;

    // Create an in-memory database for testing
    int result = sqlite3_open(":memory:", &database);

    assert(result == SQLITE_OK);


    // ==========================================
    // LOAD DATABASE SCHEMA
    // ==========================================

    ifstream schemaFile("schema.sql");

    assert(schemaFile.is_open());

    stringstream buffer;
    buffer << schemaFile.rdbuf();

    string schema = buffer.str();

    schemaFile.close();

    char* errorMessage = nullptr;

    result = sqlite3_exec(
        database,
        schema.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        cout << "Schema error: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        sqlite3_close(database);
        return 1;
    }

    cout << "Database schema setup: PASSED" << endl;


    // ==========================================
    // CREATE TEST DEPARTMENT
    // ==========================================

    const char* departmentSQL =
        "INSERT INTO departments (name) "
        "VALUES ('SQLite Test Department');";

    result = sqlite3_exec(
        database,
        departmentSQL,
        nullptr,
        nullptr,
        nullptr
    );

    assert(result == SQLITE_OK);

    int departmentId =
        static_cast<int>(sqlite3_last_insert_rowid(database));


    // ==========================================
    // CREATE REPOSITORY
    // ==========================================

    SQLiteUserRepository repository(database);


    // ==========================================
    // TEST 1: CREATE LECTURER
    // ==========================================

    string lecturerPassword = "LecturerPassword123";

    string lecturerHash =
        PasswordHasher::hashPassword(lecturerPassword);

    assert(!lecturerHash.empty());

    bool lecturerCreated = repository.createUser(
        "sqlite.lecturer",
        lecturerHash,
        "SQLite Test Lecturer",
        Role::LECTURER,
        true,
        departmentId,
        ""
    );

    assert(lecturerCreated == true);

    cout << "SQLite lecturer creation: PASSED" << endl;


    // ==========================================
    // TEST 2: CREATE STUDENT
    // ==========================================

    string studentPassword = "StudentPassword123";

    string studentHash =
        PasswordHasher::hashPassword(studentPassword);

    assert(!studentHash.empty());

    bool studentCreated = repository.createUser(
        "sqlite.student",
        studentHash,
        "SQLite Test Student",
        Role::STUDENT,
        true,
        departmentId,
        "SQL001"
    );

    assert(studentCreated == true);

    cout << "SQLite student creation: PASSED" << endl;


    // ==========================================
    // TEST 3: RETRIEVE ALL USERS
    // ==========================================

    vector<UserSummary> users =
        repository.getAllUsers();

    assert(users.size() == 2);

    cout << "SQLite user listing: PASSED" << endl;


    // ==========================================
    // TEST 4: SEARCH BY USERNAME
    // ==========================================

    vector<UserSummary> usernameResults =
        repository.searchUsers("sqlite.lecturer");

    assert(usernameResults.size() == 1);
    assert(usernameResults[0].username == "sqlite.lecturer");

    cout << "SQLite username search: PASSED" << endl;


    // ==========================================
    // TEST 5: SEARCH BY DISPLAY NAME
    // ==========================================

    vector<UserSummary> nameResults =
        repository.searchUsers("Test Student");

    assert(nameResults.size() == 1);
    assert(nameResults[0].displayName == "SQLite Test Student");

    cout << "SQLite display name search: PASSED" << endl;


    // ==========================================
    // TEST 6: REJECT DUPLICATE USERNAME
    // ==========================================

    bool duplicateCreated = repository.createUser(
        "sqlite.lecturer",
        lecturerHash,
        "Duplicate Lecturer",
        Role::LECTURER,
        true,
        departmentId,
        ""
    );

    assert(duplicateCreated == false);

    cout << "Duplicate username rejection: PASSED" << endl;


    // ==========================================
    // TEST 7: VERIFY STORED PASSWORD HASH
    // ==========================================

    unique_ptr<User> storedUser =
        repository.findByUsername("sqlite.lecturer");

    assert(storedUser != nullptr);

    bool passwordVerified =
        PasswordHasher::verifyPassword(
            lecturerPassword,
            storedUser->getPasswordHash()
        );

    assert(passwordVerified == true);

    cout << "SQLite stored password verification: PASSED"
        << endl;


    // ==========================================
    // TEST 8: VERIFY STUDENT NUMBER
    // ==========================================

    unique_ptr<User> storedStudent =
        repository.findByUsername("sqlite.student");

    assert(storedStudent != nullptr);

    Student* student =
        dynamic_cast<Student*>(storedStudent.get());

    assert(student != nullptr);
    assert(student->getStudentNumber() == "SQL001");

    cout << "SQLite student number retrieval: PASSED"
        << endl;


    // ==========================================
    // TEST 9: VERIFY DEPARTMENT INFORMATION
    // ==========================================

    assert(users[0].hasDepartment == true);
    assert(users[1].hasDepartment == true);

    cout << "SQLite department information: PASSED"
        << endl;


    cout << "\n========== ALL SQLITE USER MANAGEMENT TESTS PASSED =========="
        << endl;

    sqlite3_close(database);

    return 0;
}
