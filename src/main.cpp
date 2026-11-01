
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>

#include "sqlite3.h"
#include "security/PasswordHasher.h"
#include "repositories/SQLiteUserRepository.h"
#include "domain/RoleUsers.h"

using namespace std;

int main()
{
    sqlite3* database = nullptr;

    // Open the database
    int result = sqlite3_open("mlms.db", &database);

    if (result != SQLITE_OK)
    {
        cout << "Database connection failed!" << endl;
        sqlite3_close(database);
        return 1;
    }

    cout << "Database connected successfully!" << endl;

    // Enable foreign keys
    char* errorMessage = nullptr;

    result = sqlite3_exec(
        database,
        "PRAGMA foreign_keys = ON;",
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        cout << "Foreign key setup failed: "
            << errorMessage << endl;

        sqlite3_free(errorMessage);
        sqlite3_close(database);
        return 1;
    }

    // Read the database schema
    ifstream schemaFile("schema.sql");

    if (!schemaFile.is_open())
    {
        cout << "Could not open schema.sql!" << endl;
        sqlite3_close(database);
        return 1;
    }

    stringstream buffer;
    buffer << schemaFile.rdbuf();

    string schema = buffer.str();
    schemaFile.close();

    // Execute the schema
    result = sqlite3_exec(
        database,
        schema.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        cout << "Schema execution failed: "
            << errorMessage << endl;

        sqlite3_free(errorMessage);
        sqlite3_close(database);
        return 1;
    }

    cout << "Database schema ready!" << endl;


    // ==========================================
    // CREATE TEST DEPARTMENT
    // ==========================================

    const char* departmentSQL =
        "INSERT OR IGNORE INTO departments (name) "
        "VALUES ('MLMS Test Department');";

    result = sqlite3_exec(
        database,
        departmentSQL,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        cout << "Department creation failed: "
            << errorMessage << endl;

        sqlite3_free(errorMessage);
        sqlite3_close(database);
        return 1;
    }

    // Retrieve the department ID
    sqlite3_stmt* statement = nullptr;

    const char* departmentQuery =
        "SELECT department_id FROM departments "
        "WHERE name = 'MLMS Test Department';";

    result = sqlite3_prepare_v2(
        database,
        departmentQuery,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK ||
        sqlite3_step(statement) != SQLITE_ROW)
    {
        cout << "Could not retrieve test department!" << endl;

        sqlite3_finalize(statement);
        sqlite3_close(database);
        return 1;
    }

    int departmentId = sqlite3_column_int(statement, 0);

    sqlite3_finalize(statement);


    // ==========================================
    // CREATE TEST STUDENT
    // ==========================================

    string testPassword = "TestPassword123";

    string passwordHash =
        PasswordHasher::hashPassword(testPassword);

    const char* userSQL =
        "INSERT OR IGNORE INTO users "
        "(department_id, username, password_hash, role, "
        "display_name, student_number, is_active, "
        "failed_login_count, locked_until, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    result = sqlite3_prepare_v2(
        database,
        userSQL,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        cout << "Could not prepare student creation!" << endl;
        sqlite3_close(database);
        return 1;
    }

    sqlite3_bind_int(statement, 1, departmentId);

    sqlite3_bind_text(
        statement, 2, "test.student", -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        statement, 3, passwordHash.c_str(),
        -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement, 4, "STUDENT", -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        statement, 5, "Test Student", -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        statement, 6, "TEST001", -1, SQLITE_STATIC
    );

    sqlite3_bind_int(statement, 7, 1);
    sqlite3_bind_int(statement, 8, 0);

    sqlite3_bind_null(statement, 9);

    sqlite3_bind_text(
        statement, 10, "2026-11-01 00:00:00",
        -1, SQLITE_STATIC
    );

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        cout << "Student creation failed!" << endl;
        sqlite3_close(database);
        return 1;
    }

    cout << "\nTest student is ready!" << endl;
    cout << "Username: test.student" << endl;
    cout << "Student number: TEST001" << endl;


    // ==========================================
    // TEST THE USER REPOSITORY
    // ==========================================

    cout << "\n========== USER REPOSITORY TEST =========="
        << endl;

    SQLiteUserRepository repository(database);

    // Find the student
    unique_ptr<User> user =
        repository.findByUsername("test.student");

    if (user != nullptr)
    {
        cout << "\nFind existing user: PASSED" << endl;
        cout << "Username: " << user->getUsername() << endl;

        // Check whether the returned object is a Student
        Student* student =
            dynamic_cast<Student*>(user.get());

        if (student != nullptr)
        {
            cout << "Correct user role: PASSED" << endl;
            cout << "Student number: "
                << student->getStudentNumber() << endl;
        }
        else
        {
            cout << "Correct user role: FAILED" << endl;
        }
    }
    else
    {
        cout << "\nFind existing user: FAILED" << endl;
    }


    // ==========================================
    // TEST LOGIN STATE UPDATE
    // ==========================================

    if (user != nullptr)
    {
        string lockoutTime = "2026-11-01 15:00:00";

        bool updated = repository.updateLoginState(
            user->getUserId(),
            2,
            lockoutTime
        );

        cout << "\nUpdate login information: "
            << (updated ? "PASSED" : "FAILED") << endl;

        // Retrieve the student again
        unique_ptr<User> updatedUser =
            repository.findByUsername("test.student");

        if (updatedUser != nullptr)
        {
            cout << "Failed login count: "
                << updatedUser->getFailedLoginCount() << endl;

            cout << "Lockout time: "
                << updatedUser->getLockedUntil() << endl;

            if (updatedUser->getFailedLoginCount() == 2 &&
                updatedUser->getLockedUntil() == lockoutTime)
            {
                cout << "Login information verification: PASSED"
                    << endl;
            }
            else
            {
                cout << "Login information verification: FAILED"
                    << endl;
            }
        }
    }


    // ==========================================
    // TEST NONEXISTENT USER
    // ==========================================

    unique_ptr<User> missingUser =
        repository.findByUsername("unknown.user");

    cout << "\nNonexistent username test: "
        << (missingUser == nullptr ? "PASSED" : "FAILED")
        << endl;


    cout << "\n========== TEST COMPLETED ==========" << endl;

    sqlite3_close(database);

    return 0;
}
