#include "domain/RoleUsers.h"

// Administrator
Role Administrator::getRole() const
{
    return Role::ADMINISTRATOR;
}


// Lecturer
Role Lecturer::getRole() const
{
    return Role::LECTURER;
}


// Head of Department
Role HoD::getRole() const
{
    return Role::HOD;
}


// Student
Role Student::getRole() const
{
    return Role::STUDENT;
}

string Student::getStudentNumber() const
{
    return studentNumber;
}