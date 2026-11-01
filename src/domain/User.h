#pragma once
// Abstract User with one concrete subclass per role (design section 4.2).
// Each subclass decides which role-level permissions it has (polymorphism);
// resource scope (my course, my department, my own record) is checked
// separately in PermissionPolicy.
#include <memory>
#include <optional>
#include <string>

#include "domain/Types.h"

namespace mlms {

enum class Permission {
    ManageUsers,
    ManageCourses,
    ViewAllAudit,
    CreateAssignment,
    GrantExtension,
    MarkSubmission,
    ModerateMarks,
    ReleaseMarks,
    ViewDepartmentAudit,
    SubmitWork,
    ViewOwnResults
};

struct UserRecord {
    int id = 0;
    std::string username;
    std::string passwordHash;
    Role role = Role::Student;
    std::string displayName;
    std::optional<int> departmentId;
    std::optional<std::string> studentNumber;
    bool active = true;
    int failedLoginCount = 0;
    std::optional<std::string> lockedUntil;
    std::string createdAt;
};

class User {
public:
    explicit User(UserRecord record);
    virtual ~User() = default;

    virtual Role role() const = 0;
    virtual bool hasPermission(Permission permission) const = 0;
    virtual std::string title() const = 0;

    int id() const { return record_.id; }
    const std::string& username() const { return record_.username; }
    const std::string& displayName() const { return record_.displayName; }
    const std::optional<int>& departmentId() const { return record_.departmentId; }
    bool isActive() const { return record_.active; }
    const UserRecord& record() const { return record_; }
    UserRecord& record() { return record_; }

protected:
    UserRecord record_;
};

class Administrator final : public User {
public:
    using User::User;
    Role role() const override { return Role::Administrator; }
    bool hasPermission(Permission permission) const override;
    std::string title() const override { return "Administrator"; }
};

class Lecturer final : public User {
public:
    using User::User;
    Role role() const override { return Role::Lecturer; }
    bool hasPermission(Permission permission) const override;
    std::string title() const override { return "Lecturer"; }
};

class HeadOfDepartment final : public User {
public:
    using User::User;
    Role role() const override { return Role::HoD; }
    bool hasPermission(Permission permission) const override;
    std::string title() const override { return "Head of Department"; }
};

class Student final : public User {
public:
    using User::User;
    Role role() const override { return Role::Student; }
    bool hasPermission(Permission permission) const override;
    std::string title() const override { return "Student"; }
    std::string studentNumber() const { return record_.studentNumber.value_or(""); }
};

// Factory pattern: the only place that turns a role into a concrete User type.
class UserFactory {
public:
    static std::shared_ptr<User> create(const UserRecord& record);
};

}  // namespace mlms
