#include "services/PermissionPolicy.h"

#include "domain/Errors.h"

namespace mlms {

void PermissionPolicy::require(const User& user, Permission permission) const {
    if (!user.isActive()) throw PermissionError("Your account is not active");
    if (!user.hasPermission(permission)) {
        throw PermissionError("Access denied: a " + user.title() + " cannot perform this action");
    }
}

bool PermissionPolicy::teaches(const User& lecturer, int courseId) const {
    return lecturer.role() == Role::Lecturer && courses_.isLecturerOf(lecturer.id(), courseId);
}

bool PermissionPolicy::inDepartment(const User& hod, int courseId) const {
    const std::optional<int> dept = hod.departmentId();
    if (hod.role() != Role::HoD || !dept) return false;
    auto course = courses_.findCourse(courseId);
    return course && course->departmentId == *dept;
}

void PermissionPolicy::requireTeaches(const User& lecturer, int courseId) const {
    if (!teaches(lecturer, courseId)) throw PermissionError("Access denied: you are not assigned to this course");
}

void PermissionPolicy::requireDepartmentScope(const User& hod, int courseId) const {
    if (!inDepartment(hod, courseId)) throw PermissionError("Access denied: this course is not in your department");
}

void PermissionPolicy::requireEnrolled(const User& student, int courseId) const {
    if (student.role() != Role::Student || !courses_.isEnrolled(student.id(), courseId)) {
        throw PermissionError("Access denied: you are not enrolled in this course");
    }
}

}  // namespace mlms
