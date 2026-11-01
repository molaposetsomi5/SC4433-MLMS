#pragma once
// CourseManagementService (FR-04, FR-05).
#include <string>
#include <vector>

#include "data/Repositories.h"
#include "domain/DateTime.h"
#include "services/PermissionPolicy.h"

namespace mlms {

class CourseManagementService {
public:
    CourseManagementService(ICourseRepository& courses, IUserRepository& users, PermissionPolicy& permissions,
                            const IClock& clock);

    int createDepartment(const User& admin, const std::string& name);
    std::vector<Department> listDepartments();
    int createCourse(const User& admin, int departmentId, const std::string& code, const std::string& name);
    std::vector<Course> listCourses();
    Course getCourse(int courseId);
    void assignLecturer(const User& admin, int lecturerId, int courseId);
    void enrolStudent(const User& admin, int studentId, int courseId);

    // Courses the user is linked to: taught / enrolled / in department / all (admin).
    std::vector<Course> coursesFor(const User& user);

private:
    ICourseRepository& courses_;
    IUserRepository& users_;
    PermissionPolicy& permissions_;
    const IClock& clock_;
};

}  // namespace mlms
