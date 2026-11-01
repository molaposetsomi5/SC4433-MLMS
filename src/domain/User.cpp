#include "domain/User.h"

#include <utility>

#include "domain/Errors.h"

namespace mlms {

User::User(UserRecord record) : record_(std::move(record)) {}

bool Administrator::hasPermission(Permission permission) const {
    return permission == Permission::ManageUsers || permission == Permission::ManageCourses ||
           permission == Permission::ViewAllAudit;
}

bool Lecturer::hasPermission(Permission permission) const {
    return permission == Permission::CreateAssignment || permission == Permission::GrantExtension ||
           permission == Permission::MarkSubmission;
}

bool HeadOfDepartment::hasPermission(Permission permission) const {
    return permission == Permission::ModerateMarks || permission == Permission::ReleaseMarks ||
           permission == Permission::ViewDepartmentAudit;
}

bool Student::hasPermission(Permission permission) const {
    return permission == Permission::SubmitWork || permission == Permission::ViewOwnResults;
}

std::shared_ptr<User> UserFactory::create(const UserRecord& record) {
    switch (record.role) {
        case Role::Administrator: return std::make_shared<Administrator>(record);
        case Role::Lecturer: return std::make_shared<Lecturer>(record);
        case Role::HoD: return std::make_shared<HeadOfDepartment>(record);
        case Role::Student: return std::make_shared<Student>(record);
    }
    throw ValidationError("Cannot create user with unknown role");
}

}  // namespace mlms
