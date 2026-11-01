
#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include <string>
#include <memory>

#include "domain/User.h"

using namespace std;

class UserRepository
{
public:
    // Destructor
    virtual ~UserRepository();

    // Find a user using their username
    virtual unique_ptr<User> findByUsername(
        const string& username) = 0;

    // Update failed login attempts and account lockout time
    virtual bool updateLoginState(
        int userId,
        int failedLoginCount,
        const string& lockedUntil) = 0;
};

#endif
