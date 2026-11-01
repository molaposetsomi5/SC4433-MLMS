
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
    // Constructor
    UserManagementService(
        UserManagementRepository& repository,
        AuthorizationService& authorizationService)
        : repository(repository),
        authorizationService(authorizationService)
    {
    }

    // Create a new user account
    bool createUser(
        const User& administrator,
        const string& username,
        const string& password,
        const string& displayName,
        Role role,
        bool hasDepartment,
        int departmentId,
        const string& studentNumber);

    // Retrieve all user accounts
    vector<UserSummary> getAllUsers(
        const User& administrator);

    // Search for user accounts
    vector<UserSummary> searchUsers(
        const User& administrator,
        const string& searchTerm);
};

#endif
