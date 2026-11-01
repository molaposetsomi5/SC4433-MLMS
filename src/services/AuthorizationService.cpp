
#include "services/AuthorizationService.h"

// Check whether a user's role allows an action
bool AuthorizationService::hasPermission(
    const User& user,
    Permission permission) const
{
    Role role = user.getRole();

    switch (role)
    {
    case Role::ADMINISTRATOR:

        switch (permission)
        {
        case Permission::MANAGE_USERS:
        case Permission::MANAGE_DEPARTMENTS:
        case Permission::MANAGE_COURSES:
            return true;

        default:
            return false;
        }


    case Role::LECTURER:

        switch (permission)
        {
        case Permission::CREATE_ASSIGNMENTS:
        case Permission::MANAGE_ASSIGNMENTS:
        case Permission::VIEW_COURSE_SUBMISSIONS:
        case Permission::GRADE_SUBMISSIONS:
        case Permission::GRANT_EXTENSIONS:
            return true;

        default:
            return false;
        }


    case Role::HOD:

        switch (permission)
        {
        case Permission::MANAGE_COURSES:
        case Permission::MANAGE_ENROLMENTS:
        case Permission::MODERATE_MARKS:
        case Permission::RELEASE_MARKS:
        case Permission::VIEW_DEPARTMENT_RECORDS:
            return true;

        default:
            return false;
        }


    case Role::STUDENT:

        switch (permission)
        {
        case Permission::SUBMIT_ASSIGNMENT:
        case Permission::VIEW_OWN_RESULTS:
            return true;

        default:
            return false;
        }

    default:
        return false;
    }
}
