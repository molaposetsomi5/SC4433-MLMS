
#ifndef AUTHORIZATION_SERVICE_H
#define AUTHORIZATION_SERVICE_H

#include "domain/User.h"

// Actions that require permission
enum class Permission
{
    MANAGE_USERS,
    MANAGE_DEPARTMENTS,
    MANAGE_COURSES,
    MANAGE_ENROLMENTS,
    CREATE_ASSIGNMENTS,
    MANAGE_ASSIGNMENTS,
    VIEW_COURSE_SUBMISSIONS,
    GRADE_SUBMISSIONS,
    GRANT_EXTENSIONS,
    MODERATE_MARKS,
    RELEASE_MARKS,
    SUBMIT_ASSIGNMENT,
    VIEW_OWN_RESULTS,
    VIEW_DEPARTMENT_RECORDS
};

class AuthorizationService
{
public:
    // Check whether a user's role allows an action
    bool hasPermission(
        const User& user,
        Permission permission
    ) const;
};

#endif
