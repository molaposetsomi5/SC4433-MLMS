#ifndef USER_SUMMARY_H
#define USER_SUMMARY_H

#include <string>

#include "domain/User.h"

using namespace std;

// Stores user information for management and display.
// It does not contain the user's password hash.
class UserSummary
{
public:
    int userId;
    string username;
    string displayName;
    Role role;
    bool active;

    // Indicates whether the user belongs to a department.
    bool hasDepartment;
    int departmentId;

    // Only students have a student number.
    string studentNumber;

    UserSummary()
    {
        userId = 0;
        role = Role::STUDENT;
        active = true;
        hasDepartment = false;
        departmentId = 0;
    }
};

#endif