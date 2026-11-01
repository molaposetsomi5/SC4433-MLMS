#ifndef USER_MANAGEMENT_REPOSITORY_H
#define USER_MANAGEMENT_REPOSITORY_H

#include <string>
#include <vector>

#include "domain/UserSummary.h"

using namespace std;

class UserManagementRepository
{
public:
    virtual ~UserManagementRepository();

    virtual bool createUser(
        const string& username,
        const string& passwordHash,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber
    ) = 0;

    virtual vector<UserSummary> getAllUsers() = 0;

    virtual vector<UserSummary> searchUsers(
        const string& searchTerm
    ) = 0;

    virtual bool updateUser(
        int userId,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber
    );

    virtual bool setUserActive(
        int userId,
        bool active
    );
};

#endif