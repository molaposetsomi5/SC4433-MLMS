
#include "services/UserManagementService.h"

#include "security/PasswordHasher.h"


// Create a new user account
bool UserManagementService::createUser(
    const User& administrator,
    const string& username,
    const string& password,
    const string& displayName,
    Role role,
    bool hasDepartment,
    int departmentId,
    const string& studentNumber)
{
    // Check whether the user can manage accounts
    if (!authorizationService.hasPermission(
        administrator, Permission::MANAGE_USERS))
    {
        return false;
    }

    // Check required account information
    if (username.empty() ||
        password.empty() ||
        displayName.empty())
    {
        return false;
    }

    // A department ID must be valid when supplied
    if (hasDepartment && departmentId <= 0)
    {
        return false;
    }

    // Students must have a student number
    if (role == Role::STUDENT && studentNumber.empty())
    {
        return false;
    }

    // Hash the password before storing it
    string passwordHash =
        PasswordHasher::hashPassword(password);

    if (passwordHash.empty())
    {
        return false;
    }

    // Save the new account through the repository
    return repository.createUser(
        username,
        passwordHash,
        displayName,
        role,
        hasDepartment,
        departmentId,
        studentNumber
    );
}


// Retrieve all user accounts
vector<UserSummary> UserManagementService::getAllUsers(
    const User& administrator)
{
    // Check permission
    if (!authorizationService.hasPermission(
        administrator, Permission::MANAGE_USERS))
    {
        return {};
    }

    return repository.getAllUsers();
}


// Search for user accounts
vector<UserSummary> UserManagementService::searchUsers(
    const User& administrator,
    const string& searchTerm)
{
    // Check permission
    if (!authorizationService.hasPermission(
        administrator, Permission::MANAGE_USERS))
    {
        return {};
    }

    if (searchTerm.empty())
    {
        return {};
    }

    return repository.searchUsers(searchTerm);
}
