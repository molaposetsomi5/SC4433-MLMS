#include "repositories/SQLiteUserRepository.h"

#include "domain/RoleUsers.h"
#include "sqlite3.h"


// Convert a role into the text stored in the database
string roleToString(Role role)
{
    if (role == Role::ADMINISTRATOR)
    {
        return "ADMINISTRATOR";
    }
    else if (role == Role::LECTURER)
    {
        return "LECTURER";
    }
    else if (role == Role::HOD)
    {
        return "HOD";
    }

    return "STUDENT";
}


// Read a role from the database
Role stringToRole(const string& role)
{
    if (role == "ADMINISTRATOR")
    {
        return Role::ADMINISTRATOR;
    }
    else if (role == "LECTURER")
    {
        return Role::LECTURER;
    }
    else if (role == "HOD")
    {
        return Role::HOD;
    }

    return Role::STUDENT;
}


// Safely read text from a database column
string getColumnText(sqlite3_stmt* statement, int column)
{
    const unsigned char* value =
        sqlite3_column_text(statement, column);

    if (value == nullptr)
    {
        return "";
    }

    return reinterpret_cast<const char*>(value);
}


// Destructor
SQLiteUserRepository::~SQLiteUserRepository()
{
}


// ==========================================
// AUTHENTICATION OPERATIONS
// ==========================================

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

    int userId = sqlite3_column_int(statement, 0);

    string storedUsername = getColumnText(statement, 1);
    string passwordHash = getColumnText(statement, 2);
    string role = getColumnText(statement, 3);

    bool active = sqlite3_column_int(statement, 4);

    int departmentId = 0;

    if (sqlite3_column_type(statement, 5) != SQLITE_NULL)
    {
        departmentId = sqlite3_column_int(statement, 5);
    }

    int failedLoginCount =
        sqlite3_column_int(statement, 6);

    string lockedUntil = getColumnText(statement, 7);
    string studentNumber = getColumnText(statement, 8);

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


// ==========================================
// USER MANAGEMENT OPERATIONS
// ==========================================

bool SQLiteUserRepository::createUser(
    const string& username,
    const string& passwordHash,
    const string& displayName,
    Role role,
    bool hasDepartment,
    int departmentId,
    const string& studentNumber)
{
    const char* sql =
        "INSERT INTO users "
        "(username, password_hash, role, display_name, "
        "department_id, student_number, is_active, "
        "failed_login_count, locked_until, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, 1, 0, NULL, datetime('now'));";

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

    sqlite3_bind_text(
        statement, 1, username.c_str(),
        -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement, 2, passwordHash.c_str(),
        -1, SQLITE_TRANSIENT
    );

    string roleText = roleToString(role);

    sqlite3_bind_text(
        statement, 3, roleText.c_str(),
        -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement, 4, displayName.c_str(),
        -1, SQLITE_TRANSIENT
    );

    if (hasDepartment)
    {
        sqlite3_bind_int(statement, 5, departmentId);
    }
    else
    {
        sqlite3_bind_null(statement, 5);
    }

    if (role == Role::STUDENT && !studentNumber.empty())
    {
        sqlite3_bind_text(
            statement, 6, studentNumber.c_str(),
            -1, SQLITE_TRANSIENT
        );
    }
    else
    {
        sqlite3_bind_null(statement, 6);
    }

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}


vector<UserSummary> SQLiteUserRepository::getAllUsers()
{
    vector<UserSummary> users;

    const char* sql =
        "SELECT user_id, username, display_name, role, "
        "is_active, department_id, student_number "
        "FROM users ORDER BY user_id;";

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
        return users;
    }

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        UserSummary user;

        user.userId = sqlite3_column_int(statement, 0);
        user.username = getColumnText(statement, 1);
        user.displayName = getColumnText(statement, 2);

        string roleText = getColumnText(statement, 3);
        user.role = stringToRole(roleText);

        user.active = sqlite3_column_int(statement, 4);

        if (sqlite3_column_type(statement, 5) != SQLITE_NULL)
        {
            user.hasDepartment = true;
            user.departmentId = sqlite3_column_int(statement, 5);
        }
        else
        {
            user.hasDepartment = false;
        }

        user.studentNumber = getColumnText(statement, 6);

        users.push_back(user);
    }

    sqlite3_finalize(statement);

    return users;
}


vector<UserSummary> SQLiteUserRepository::searchUsers(
    const string& searchTerm)
{
    vector<UserSummary> users;

    const char* sql =
        "SELECT user_id, username, display_name, role, "
        "is_active, department_id, student_number "
        "FROM users "
        "WHERE username LIKE ? OR display_name LIKE ? "
        "ORDER BY user_id;";

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
        return users;
    }

    string searchPattern = "%" + searchTerm + "%";

    sqlite3_bind_text(
        statement, 1, searchPattern.c_str(),
        -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement, 2, searchPattern.c_str(),
        -1, SQLITE_TRANSIENT
    );

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        UserSummary user;

        user.userId = sqlite3_column_int(statement, 0);
        user.username = getColumnText(statement, 1);
        user.displayName = getColumnText(statement, 2);

        string roleText = getColumnText(statement, 3);
        user.role = stringToRole(roleText);

        user.active = sqlite3_column_int(statement, 4);

        if (sqlite3_column_type(statement, 5) != SQLITE_NULL)
        {
            user.hasDepartment = true;
            user.departmentId = sqlite3_column_int(statement, 5);
        }
        else
        {
            user.hasDepartment = false;
        }

        user.studentNumber = getColumnText(statement, 6);

        users.push_back(user);
    }

    sqlite3_finalize(statement);

    return users;
}


// ==========================================
// UPDATE USER DETAILS
// ==========================================

bool SQLiteUserRepository::updateUser(
    int userId,
    const string& displayName,
    Role role,
    bool hasDepartment,
    int departmentId,
    const string& studentNumber)
{
    const char* sql =
        "UPDATE users "
        "SET display_name = ?, role = ?, "
        "department_id = ?, student_number = ? "
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

    sqlite3_bind_text(
        statement, 1, displayName.c_str(),
        -1, SQLITE_TRANSIENT
    );

    string roleText = roleToString(role);

    sqlite3_bind_text(
        statement, 2, roleText.c_str(),
        -1, SQLITE_TRANSIENT
    );

    if (hasDepartment)
    {
        sqlite3_bind_int(statement, 3, departmentId);
    }
    else
    {
        sqlite3_bind_null(statement, 3);
    }

    if (role == Role::STUDENT && !studentNumber.empty())
    {
        sqlite3_bind_text(
            statement, 4, studentNumber.c_str(),
            -1, SQLITE_TRANSIENT
        );
    }
    else
    {
        sqlite3_bind_null(statement, 4);
    }

    sqlite3_bind_int(statement, 5, userId);

    result = sqlite3_step(statement);

    int changedRows = sqlite3_changes(database);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE && changedRows > 0;
}


// ==========================================
// ACTIVATE OR DEACTIVATE USER
// ==========================================

bool SQLiteUserRepository::setUserActive(
    int userId,
    bool active)
{
    const char* sql =
        "UPDATE users SET is_active = ? "
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

    sqlite3_bind_int(statement, 1, active ? 1 : 0);
    sqlite3_bind_int(statement, 2, userId);

    result = sqlite3_step(statement);

    int changedRows = sqlite3_changes(database);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE && changedRows > 0;
}