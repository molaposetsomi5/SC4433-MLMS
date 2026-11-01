
#ifndef SQLITE_USER_REPOSITORY_H
#define SQLITE_USER_REPOSITORY_H

#include "repositories/UserRepository.h"

// Forward declaration of SQLite database structure
struct sqlite3;

class SQLiteUserRepository : public UserRepository
{
private:
    sqlite3* database;

public:
    // Constructor
    SQLiteUserRepository(sqlite3* database)
    {
        this->database = database;
    }

    // Destructor
    ~SQLiteUserRepository();

    // Find a user using their username
    unique_ptr<User> findByUsername(
        const string& username);

    // Update failed login attempts and lockout time
    bool updateLoginState(
        int userId,
        int failedLoginCount,
        const string& lockedUntil);
};

#endif
