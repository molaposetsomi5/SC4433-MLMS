#include "repositories/UserManagementRepository.h"


UserManagementRepository::~UserManagementRepository()
{
}


bool UserManagementRepository::updateUser(
    int userId,
    const string& displayName,
    Role role,
    bool hasDepartment,
    int departmentId,
    const string& studentNumber)
{
    return false;
}


bool UserManagementRepository::setUserActive(
    int userId,
    bool active)
{
    return false;
}