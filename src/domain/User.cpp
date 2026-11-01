#include "domain/User.h"

// Destructor
User::~User()
{
}

// Getters
int User::getUserId() const
{
    return userId;
}

string User::getUsername() const
{
    return username;
}

string User::getPasswordHash() const
{
    return passwordHash;
}

bool User::isActive() const
{
    return active;
}

int User::getDepartmentId() const
{
    return departmentId;
}

int User::getFailedLoginCount() const
{
    return failedLoginCount;
}

string User::getLockedUntil() const
{
    return lockedUntil;
}

// Setters
void User::setActive(bool active)
{
    this->active = active;
}

void User::setDepartmentId(int departmentId)
{
    this->departmentId = departmentId;
}

void User::setFailedLoginCount(int failedLoginCount)
{
    this->failedLoginCount = failedLoginCount;
}

void User::setLockedUntil(const string& lockedUntil)
{
    this->lockedUntil = lockedUntil;
}

void User::setPasswordHash(const string& passwordHash)
{
    this->passwordHash = passwordHash;
}