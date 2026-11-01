#ifndef USER_MANAGEMENT_SERVICE_H
#define USER_MANAGEMENT_SERVICE_H

#include <string>
#include <vector>

#include "repositories/UserManagementRepository.h"
#include "services/AuthorizationService.h"

using namespace std;

class UserManagementService
{
private:
    UserManagementRepository& repository;
    AuthorizationService& authorizationService;

public:
    UserManagementService(
        UserManagementRepository& repository,
        AuthorizationService& authorizationService)
        : repository(repository),
        authorizationService(authorizationService)
    {
    }

    bool createUser(
        const User& administrator,
        const string& username,
        const string& password,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber);

    vector<UserSummary> getAllUsers(
        const User& administrator);

    vector<UserSummary> searchUsers(
        const User& administrator,
        const string& searchTerm);

    bool updateUser(
        const User& administrator,
        int userId,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber);

    bool setUserActive(
        const User& administrator,
        int userId,
        bool active);
};

#endif