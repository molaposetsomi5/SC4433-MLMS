#include "services/CourseManagementService.h"

#include "domain/Errors.h"
#include "domain/Validation.h"

namespace mlms {

CourseManagementService::CourseManagementService(ICourseRepository& courses, IUserRepository& users,
                                                 PermissionPolicy& permissions, const IClock& clock)
    : courses_(courses), users_(users), permissions_(permissions), clock_(clock) {}

int CourseManagementService::createDepartment(const User& admin, const std::string& name) {
    permissions_.require(admin, Permission::ManageCourses);
    const std::string cleaned = requireText(name, "Department name");
    if (courses_.findDepartmentByName(cleaned)) throw ValidationError("Department already exists");
    return courses_.addDepartment(cleaned);
}

std::vector<Department> CourseManagementService::listDepartments() { return courses_.listDepartments(); }

int CourseManagementService::createCourse(const User& admin, int departmentId, const std::string& code,
                                          const std::string& name) {
    permissions_.require(admin, Permission::ManageCourses);
    if (!courses_.findDepartment(departmentId)) throw NotFoundError("Department not found");
    Course course;
    course.departmentId = departmentId;
    course.code = requireText(code, "Course code");
    course.name = requireText(name, "Course name");
    if (courses_.findCourseByCode(course.code)) throw ValidationError("Course code already exists");
    return courses_.addCourse(course);
}

std::vector<Course> CourseManagementService::listCourses() { return courses_.listCourses(); }

Course CourseManagementService::getCourse(int courseId) {
    auto course = courses_.findCourse(courseId);
    if (!course) throw NotFoundError("Course not found");
    return *course;
}

void CourseManagementService::assignLecturer(const User& admin, int lecturerId, int courseId) {
    permissions_.require(admin, Permission::ManageCourses);
    auto lecturer = users_.findById(lecturerId);
    if (!lecturer || lecturer->role() != Role::Lecturer) throw ValidationError("That user is not a lecturer");
    if (!lecturer->isActive()) throw RuleViolation("That lecturer account is not active");
    getCourse(courseId);
    courses_.assignLecturer(lecturerId, courseId);
}

void CourseManagementService::enrolStudent(const User& admin, int studentId, int courseId) {
    permissions_.require(admin, Permission::ManageCourses);
    auto student = users_.findById(studentId);
    if (!student || student->role() != Role::Student) throw ValidationError("That user is not a student");
    if (!student->isActive()) throw RuleViolation("That student account is not active");
    getCourse(courseId);
    courses_.enrolStudent(studentId, courseId, clock_.nowText());
}

std::vector<Course> CourseManagementService::coursesFor(const User& user) {
    switch (user.role()) {
        case Role::Lecturer: return courses_.coursesForLecturer(user.id());
        case Role::Student: return courses_.coursesForStudent(user.id());
        case Role::HoD: {
            const std::optional<int> dept = user.departmentId();
            return dept ? courses_.coursesForDepartment(*dept) : std::vector<Course>{};
        }
        case Role::Administrator: return courses_.listCourses();
    }
    return {};
}

}  // namespace mlms
