#ifndef USER_MANAGEMENT_REPOSITORY_H
#define USER_MANAGEMENT_REPOSITORY_H

#include <string>
#include <vector>

#include "domain/UserSummary.h"

using namespace std;

class UserManagementRepository
{
public:
    // Destructor
    virtual ~UserManagementRepository();

    // Create a new user account
    virtual bool createUser(
        const string& username,
        const string& passwordHash,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber
    ) = 0;

    // Retrieve all user accounts
    virtual vector<UserSummary> getAllUsers() = 0;

    // Search for users by username or display name
    virtual vector<UserSummary> searchUsers(
        const string& searchTerm
    ) = 0;
};

#endif