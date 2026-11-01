
#include "repositories/SQLiteUserRepository.h"

#include "domain/RoleUsers.h"
#include "sqlite3.h"

// Destructor
SQLiteUserRepository::~SQLiteUserRepository()
{
}


// Find a user using their username
unique_ptr<User> SQLiteUserRepository::findByUsername(
    const string& username)
{
    const char* sql =
        "SELECT user_id, username, password_hash, role, "
        "is_active, department_id, failed_login_count, "
        "locked_until, student_number "
        "FROM users WHERE username = ?;";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        database,
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return nullptr;
    }

    // Safely insert the username into the query
    sqlite3_bind_text(
        statement,
        1,
        username.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    result = sqlite3_step(statement);

    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(statement);
        return nullptr;
    }

    // Read user information from the database
    int userId = sqlite3_column_int(statement, 0);

    string storedUsername =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 1)
            );

    string passwordHash =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 2)
            );

    string role =
        reinterpret_cast<const char*>(
            sqlite3_column_text(statement, 3)
            );

    bool active = sqlite3_column_int(statement, 4);

    int departmentId = sqlite3_column_int(statement, 5);

    int failedLoginCount =
        sqlite3_column_int(statement, 6);

    const unsigned char* lockedUntilText =
        sqlite3_column_text(statement, 7);

    string lockedUntil = "";

    if (lockedUntilText != nullptr)
    {
        lockedUntil =
            reinterpret_cast<const char*>(lockedUntilText);
    }

    const unsigned char* studentNumberText =
        sqlite3_column_text(statement, 8);

    string studentNumber = "";

    if (studentNumberText != nullptr)
    {
        studentNumber =
            reinterpret_cast<const char*>(studentNumberText);
    }

    // Create the correct user object
    unique_ptr<User> user = nullptr;

    if (role == "ADMINISTRATOR")
    {
        user = make_unique<Administrator>(
            userId, storedUsername, passwordHash,
            active, departmentId, failedLoginCount,
            lockedUntil
        );
    }
    else if (role == "LECTURER")
    {
        user = make_unique<Lecturer>(
            userId, storedUsername, passwordHash,
            active, departmentId, failedLoginCount,
            lockedUntil
        );
    }
    else if (role == "HOD")
    {
        user = make_unique<HoD>(
            userId, storedUsername, passwordHash,
            active, departmentId, failedLoginCount,
            lockedUntil
        );
    }
    else if (role == "STUDENT")
    {
        user = make_unique<Student>(
            userId, storedUsername, passwordHash,
            active, departmentId, failedLoginCount,
            lockedUntil, studentNumber
        );
    }

    sqlite3_finalize(statement);

    return user;
}


// Update failed login attempts and lockout time
bool SQLiteUserRepository::updateLoginState(
    int userId,
    int failedLoginCount,
    const string& lockedUntil)
{
    const char* sql =
        "UPDATE users "
        "SET failed_login_count = ?, locked_until = ? "
        "WHERE user_id = ?;";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        database,
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(statement, 1, failedLoginCount);

    if (lockedUntil.empty())
    {
        sqlite3_bind_null(statement, 2);
    }
    else
    {
        sqlite3_bind_text(
            statement,
            2,
            lockedUntil.c_str(),
            -1,
            SQLITE_TRANSIENT
        );
    }

    sqlite3_bind_int(statement, 3, userId);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}
