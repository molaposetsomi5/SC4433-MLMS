#include "services/UserManagementService.h"

#include "security/PasswordHasher.h"


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
    if (!authorizationService.hasPermission(
        administrator, Permission::MANAGE_USERS))
    {
        return false;
    }

    if (username.empty() ||
        password.empty() ||
        displayName.empty())
    {
        return false;
    }

    if (hasDepartment && departmentId <= 0)
    {
        return false;
    }

    if (role == Role::STUDENT && studentNumber.empty())
    {
        return false;
    }

    string passwordHash =
        PasswordHasher::hashPassword(password);

    if (passwordHash.empty())
    {
        return false;
    }

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


vector<UserSummary> UserManagementService::getAllUsers(
    const User& administrator)
{
    if (!authorizationService.hasPermission(
        administrator, Permission::MANAGE_USERS))
    {
        return {};
    }

    return repository.getAllUsers();
}


vector<UserSummary> UserManagementService::searchUsers(
    const User& administrator,
    const string& searchTerm)
{
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


bool UserManagementService::updateUser(
    const User& administrator,
    int userId,
    const string& displayName,
    Role role,
    bool hasDepartment,
    int departmentId,
    const string& studentNumber)
{
    if (!authorizationService.hasPermission(
        administrator, Permission::MANAGE_USERS))
    {
        return false;
    }

    if (userId <= 0 || displayName.empty())
    {
        return false;
    }

    if (hasDepartment && departmentId <= 0)
    {
        return false;
    }

    if (role == Role::STUDENT && studentNumber.empty())
    {
        return false;
    }

    return repository.updateUser(
        userId,
        displayName,
        role,
        hasDepartment,
        departmentId,
        studentNumber
    );
}


bool UserManagementService::setUserActive(
    const User& administrator,
    int userId,
    bool active)
{
    if (!authorizationService.hasPermission(
        administrator, Permission::MANAGE_USERS))
    {
        return false;
    }

    if (userId <= 0)
    {
        return false;
    }

    return repository.setUserActive(userId, active);
}