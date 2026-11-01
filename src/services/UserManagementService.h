#pragma once
// UserManagementService (FR-03).
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "data/Repositories.h"
#include "services/AuthService.h"
#include "services/PermissionPolicy.h"

namespace mlms {

struct NewUserRequest {
    std::string username;
    std::string password;
    std::string displayName;
    Role role = Role::Student;
    std::optional<int> departmentId;
    std::optional<std::string> studentNumber;
};

class UserManagementService {
public:
    UserManagementService(IUserRepository& users, ICourseRepository& courses, const IPasswordHasher& hasher,
                          AuthService& auth, PermissionPolicy& permissions, const IClock& clock);

    int createUser(const User& admin, const NewUserRequest& request);
    // Only allowed while the database has no users (first run).
    int createInitialAdministrator(const std::string& username, const std::string& password,
                                   const std::string& displayName);
    bool hasAnyUsers();

    std::vector<std::shared_ptr<User>> listUsers(const User& admin);
    void setActive(const User& admin, int userId, bool active);
    void updateDetails(const User& admin, int userId, const std::string& displayName,
                       const std::optional<int>& departmentId);
    void resetPassword(const User& admin, int userId, const std::string& newPassword);

private:
    int insertUser(const NewUserRequest& request);
    std::shared_ptr<User> requireUser(int userId);
    void checkDepartment(Role role, const std::optional<int>& departmentId);

    IUserRepository& users_;
    ICourseRepository& courses_;
    const IPasswordHasher& hasher_;
    AuthService& auth_;
    PermissionPolicy& permissions_;
    const IClock& clock_;
};

}  // namespace mlms
