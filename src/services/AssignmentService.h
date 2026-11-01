#pragma once
// AssignmentService (FR-06, FR-07).
#include <optional>
#include <string>
#include <vector>

#include "data/Repositories.h"
#include "domain/DateTime.h"
#include "services/DeadlineResolver.h"
#include "services/PermissionPolicy.h"

namespace mlms {

struct CourseAssignment {
    Course course;
    Assignment assignment;
};

struct StudentAssignmentView {
    Assignment assignment;
    Course course;
    DeadlineInfo deadline;
    std::optional<Submission> submission;
    int versionCount = 0;
    bool resultReleased = false;
};

class AssignmentService {
public:
    AssignmentService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      ISubmissionRepository& submissions, IMarkRepository& marks, DeadlineResolver& deadlines,
                      PermissionPolicy& permissions, const IClock& clock);

    int createAssignment(const User& lecturer, int courseId, const std::string& title,
                         const std::string& description, const std::string& deadline, double maxMark);

    Assignment get(int assignmentId);

    // Lecturer: courses they teach. HoD: courses in their department.
    std::vector<CourseAssignment> listVisibleTo(const User& staff);

    // Student: assignments for enrolled courses with their effective deadline.
    std::vector<StudentAssignmentView> listForStudent(const User& student);

private:
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    ISubmissionRepository& submissions_;
    IMarkRepository& marks_;
    DeadlineResolver& deadlines_;
    PermissionPolicy& permissions_;
    const IClock& clock_;
};

}  // namespace mlms
