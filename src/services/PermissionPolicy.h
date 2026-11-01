#pragma once
// AuthorizationService / PermissionPolicy (design section 3.3).
// Every service calls this before doing work - hiding a menu item is not
// treated as access control.
#include "data/Repositories.h"
#include "domain/User.h"

namespace mlms {

class PermissionPolicy {
public:
    explicit PermissionPolicy(ICourseRepository& courses) : courses_(courses) {}

    // Role-level check (uses the polymorphic User::hasPermission).
    void require(const User& user, Permission permission) const;

    // Resource-scope checks.
    void requireTeaches(const User& lecturer, int courseId) const;
    void requireDepartmentScope(const User& hod, int courseId) const;
    void requireEnrolled(const User& student, int courseId) const;

    bool teaches(const User& lecturer, int courseId) const;
    bool inDepartment(const User& hod, int courseId) const;

private:
    ICourseRepository& courses_;
};

}  // namespace mlms
