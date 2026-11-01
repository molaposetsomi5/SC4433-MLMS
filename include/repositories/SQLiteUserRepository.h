#ifndef SQLITE_USER_REPOSITORY_H
#define SQLITE_USER_REPOSITORY_H

#include "repositories/UserRepository.h"
#include "repositories/UserManagementRepository.h"

struct sqlite3;

class SQLiteUserRepository :
    public UserRepository,
    public UserManagementRepository
{
private:
    sqlite3* database;

public:
    SQLiteUserRepository(sqlite3* database)
    {
        this->database = database;
    }

    ~SQLiteUserRepository();

    unique_ptr<User> findByUsername(
        const string& username);

    bool updateLoginState(
        int userId,
        int failedLoginCount,
        const string& lockedUntil);

    bool createUser(
        const string& username,
        const string& passwordHash,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber);

    vector<UserSummary> getAllUsers();

    vector<UserSummary> searchUsers(
        const string& searchTerm);

    bool updateUser(
        int userId,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber);

    bool setUserActive(
        int userId,
        bool active);
};

#endif