#include "services/AssignmentService.h"

#include "domain/Errors.h"
#include "domain/Validation.h"

namespace mlms {

AssignmentService::AssignmentService(IAssignmentRepository& assignments, ICourseRepository& courses,
                                     ISubmissionRepository& submissions, IMarkRepository& marks,
                                     IExtensionRepository& extensions, DeadlineResolver& deadlines,
                                     PermissionPolicy& permissions, const IClock& clock)
    : assignments_(assignments),
      courses_(courses),
      submissions_(submissions),
      marks_(marks),
      extensions_(extensions),
      deadlines_(deadlines),
      permissions_(permissions),
      clock_(clock) {}

int AssignmentService::createAssignment(const User& lecturer, int courseId, const std::string& title,
                                        const std::string& description, const std::string& deadline,
                                        double maxMark) {
    permissions_.require(lecturer, Permission::CreateAssignment);
    if (!courses_.findCourse(courseId)) throw NotFoundError("Course not found");
    permissions_.requireTeaches(lecturer, courseId);

    Assignment a;
    a.courseId = courseId;
    a.title = requireText(title, "Title");
    a.description = trim(description);
    a.deadline = normaliseDateTime(trim(deadline));
    if (toSeconds(a.deadline) <= clock_.now()) throw ValidationError("The deadline must be in the future");
    if (maxMark <= 0) throw ValidationError("Maximum mark must be greater than zero");
    a.maxMark = maxMark;
    a.createdBy = lecturer.id();
    a.createdAt = clock_.nowText();
    return assignments_.insert(a);
}

Assignment AssignmentService::get(int assignmentId) {
    auto a = assignments_.findById(assignmentId);
    if (!a) throw NotFoundError("Assignment not found");
    return *a;
}

std::vector<CourseAssignment> AssignmentService::listVisibleTo(const User& staff) {
    std::vector<Course> courses;
    if (staff.role() == Role::Lecturer) {
        courses = courses_.coursesForLecturer(staff.id());
    } else if (const std::optional<int> dept = staff.departmentId(); staff.role() == Role::HoD && dept) {
        courses = courses_.coursesForDepartment(*dept);
    } else {
        throw PermissionError("Access denied: only lecturers and HoDs can list course assignments");
    }
    std::vector<CourseAssignment> result;
    for (const Course& c : courses) {
        for (const Assignment& a : assignments_.listForCourse(c.id)) {
            result.push_back({c, a, extensions_.findClassExtension(a.id)});
        }
    }
    return result;
}

std::vector<StudentAssignmentView> AssignmentService::listForStudent(const User& student) {
    permissions_.require(student, Permission::SubmitWork);
    std::vector<StudentAssignmentView> result;
    for (const Course& c : courses_.coursesForStudent(student.id())) {
        for (const Assignment& a : assignments_.listForCourse(c.id)) {
            StudentAssignmentView view;
            view.assignment = a;
            view.course = c;
            view.deadline = deadlines_.resolve(a, student.id());
            view.submission = submissions_.find(a.id, student.id());
            if (view.submission) {
                view.versionCount = static_cast<int>(submissions_.versions(view.submission->id).size());
                auto mark = marks_.findBySubmission(view.submission->id);
                view.resultReleased = mark && mark->status == MarkStatus::Released;
            }
            result.push_back(view);
        }
    }
    return result;
}

}  // namespace mlms
