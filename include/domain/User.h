#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

// Available user roles
enum class Role
{
    ADMINISTRATOR,
    LECTURER,
    HOD,
    STUDENT
};

class User
{
protected:
    int userId;
    string username;
    string passwordHash;
    Role role;
    bool active;
    int departmentId;
    int failedLoginCount;
    string lockedUntil;

    // Constructor
    User(int userId, const string& username,
        const string& passwordHash, Role role,
        bool active, int departmentId,
        int failedLoginCount, const string& lockedUntil)
    {
        this->userId = userId;
        this->username = username;
        this->passwordHash = passwordHash;
        this->role = role;
        this->active = active;
        this->departmentId = departmentId;
        this->failedLoginCount = failedLoginCount;
        this->lockedUntil = lockedUntil;
    }

public:
    // Destructor
    virtual ~User();

    // Getters
    int getUserId() const;
    string getUsername() const;
    string getPasswordHash() const;
    bool isActive() const;
    int getDepartmentId() const;
    int getFailedLoginCount() const;
    string getLockedUntil() const;

    // Abstract function
    virtual Role getRole() const = 0;

    // Setters
    void setActive(bool active);
    void setDepartmentId(int departmentId);
    void setFailedLoginCount(int failedLoginCount);
    void setLockedUntil(const string& lockedUntil);
    void setPasswordHash(const string& passwordHash);
};

#endif