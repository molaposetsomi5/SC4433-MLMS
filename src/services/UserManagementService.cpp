#include "services/UserManagementService.h"

#include "domain/Errors.h"
#include "domain/Validation.h"

namespace mlms {

UserManagementService::UserManagementService(IUserRepository& users, ICourseRepository& courses,
                                             const IPasswordHasher& hasher, AuthService& auth,
                                             PermissionPolicy& permissions, const IClock& clock)
    : users_(users), courses_(courses), hasher_(hasher), auth_(auth), permissions_(permissions), clock_(clock) {}

void UserManagementService::checkDepartment(Role role, const std::optional<int>& departmentId) {
    if (role == Role::HoD && !departmentId) throw ValidationError("A Head of Department must have a department");
    if (role == Role::Administrator && departmentId) {
        throw ValidationError("An administrator is not linked to a department");
    }
    if (departmentId && !courses_.findDepartment(*departmentId)) throw NotFoundError("Department not found");
}

int UserManagementService::insertUser(const NewUserRequest& request) {
    UserRecord rec;
    rec.username = requireText(request.username, "Username");
    if (rec.username.find(' ') != std::string::npos) throw ValidationError("Username cannot contain spaces");
    if (users_.findByUsername(rec.username)) throw ValidationError("Username '" + rec.username + "' is taken");

    rec.displayName = requireText(request.displayName, "Display name");
    rec.role = request.role;
    checkDepartment(rec.role, request.departmentId);
    rec.departmentId = request.departmentId;

    if (rec.role == Role::Student) {
        if (!request.studentNumber) throw ValidationError("Student number is required for a student");
        const std::string number = requireText(*request.studentNumber, "Student number");
        if (users_.studentNumberExists(number)) throw ValidationError("Student number already exists");
        rec.studentNumber = number;
    } else if (request.studentNumber && !trim(*request.studentNumber).empty()) {
        throw ValidationError("Only students have a student number");
    }

    auth_.validateNewPassword(request.password);
    rec.passwordHash = hasher_.hash(request.password);
    rec.createdAt = clock_.nowText();
    return users_.insert(rec);
}

int UserManagementService::createUser(const User& admin, const NewUserRequest& request) {
    permissions_.require(admin, Permission::ManageUsers);
    return insertUser(request);
}

bool UserManagementService::hasAnyUsers() { return users_.count() > 0; }

int UserManagementService::createInitialAdministrator(const std::string& username, const std::string& password,
                                                      const std::string& displayName) {
    if (hasAnyUsers()) throw PermissionError("The system already has users");
    NewUserRequest request;
    request.username = username;
    request.password = password;
    request.displayName = displayName;
    request.role = Role::Administrator;
    return insertUser(request);
}

std::vector<std::shared_ptr<User>> UserManagementService::listUsers(const User& admin) {
    permissions_.require(admin, Permission::ManageUsers);
    return users_.listAll();
}

std::shared_ptr<User> UserManagementService::requireUser(int userId) {
    auto user = users_.findById(userId);
    if (!user) throw NotFoundError("User not found");
    return user;
}

void UserManagementService::setActive(const User& admin, int userId, bool active) {
    permissions_.require(admin, Permission::ManageUsers);
    if (userId == admin.id() && !active) throw RuleViolation("You cannot deactivate your own account");
    auto user = requireUser(userId);
    user->record().active = active;
    if (active) {
        user->record().failedLoginCount = 0;
        user->record().lockedUntil.reset();
    }
    users_.update(user->record());
}

void UserManagementService::updateDetails(const User& admin, int userId, const std::string& displayName,
                                          const std::optional<int>& departmentId) {
    permissions_.require(admin, Permission::ManageUsers);
    auto user = requireUser(userId);
    checkDepartment(user->role(), departmentId);
    user->record().displayName = requireText(displayName, "Display name");
    user->record().departmentId = departmentId;
    users_.update(user->record());
}

void UserManagementService::resetPassword(const User& admin, int userId, const std::string& newPassword) {
    permissions_.require(admin, Permission::ManageUsers);
    auto user = requireUser(userId);
    auth_.validateNewPassword(newPassword);
    user->record().passwordHash = hasher_.hash(newPassword);
    user->record().failedLoginCount = 0;
    user->record().lockedUntil.reset();
    users_.update(user->record());
}

}  // namespace mlms
