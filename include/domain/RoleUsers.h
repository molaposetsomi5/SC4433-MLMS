#ifndef ROLE_USERS_H
#define ROLE_USERS_H

#include "User.h"

// Administrator class
class Administrator : public User
{
public:
    Administrator(int userId, const string& username,
        const string& passwordHash, bool active,
        int departmentId, int failedLoginCount,
        const string& lockedUntil)
        : User(userId, username, passwordHash,
            Role::ADMINISTRATOR, active, departmentId,
            failedLoginCount, lockedUntil)
    {
    }

    Role getRole() const;
};


// Lecturer class
class Lecturer : public User
{
public:
    Lecturer(int userId, const string& username,
        const string& passwordHash, bool active,
        int departmentId, int failedLoginCount,
        const string& lockedUntil)
        : User(userId, username, passwordHash,
            Role::LECTURER, active, departmentId,
            failedLoginCount, lockedUntil)
    {
    }

    Role getRole() const;
};


// Head of Department class
class HoD : public User
{
public:
    HoD(int userId, const string& username,
        const string& passwordHash, bool active,
        int departmentId, int failedLoginCount,
        const string& lockedUntil)
        : User(userId, username, passwordHash,
            Role::HOD, active, departmentId,
            failedLoginCount, lockedUntil)
    {
    }

    Role getRole() const;
};


// Student class
class Student : public User
{
private:
    string studentNumber;

public:
    Student(int userId, const string& username,
        const string& passwordHash, bool active,
        int departmentId, int failedLoginCount,
        const string& lockedUntil, const string& studentNumber)
        : User(userId, username, passwordHash,
            Role::STUDENT, active, departmentId,
            failedLoginCount, lockedUntil)
    {
        this->studentNumber = studentNumber;
    }

    Role getRole() const;

    string getStudentNumber() const;
};

#endif