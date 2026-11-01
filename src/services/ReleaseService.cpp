#include "services/ReleaseService.h"

#include "domain/Errors.h"
#include "services/MarkingService.h"

namespace mlms {

ReleaseService::ReleaseService(IMarkRepository& marks, ISubmissionRepository& submissions,
                               IAssignmentRepository& assignments, ICourseRepository& courses,
                               PermissionPolicy& permissions, AuditService& audit, TransactionManager& tx,
                               const IClock& clock)
    : marks_(marks),
      submissions_(submissions),
      assignments_(assignments),
      courses_(courses),
      permissions_(permissions),
      audit_(audit),
      tx_(tx),
      clock_(clock) {}

Assignment ReleaseService::loadInScope(const User& hod, int assignmentId) {
    permissions_.require(hod, Permission::ReleaseMarks);
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundError("Assignment not found");
    permissions_.requireDepartmentScope(hod, assignment->courseId);
    return *assignment;
}

ReleaseSummary ReleaseService::summarise(const User& hod, int assignmentId) {
    loadInScope(hod, assignmentId);
    ReleaseSummary summary;
    for (const Submission& s : submissions_.listForAssignment(assignmentId)) {
        ++summary.submissions;
        auto mark = marks_.findBySubmission(s.id);
        if (!mark) {
            ++summary.unmarked;
        } else if (mark->status == MarkStatus::Approved) {
            ++summary.approved;
        } else if (mark->status == MarkStatus::Released) {
            ++summary.released;
        } else {
            ++summary.awaitingOrReturned;
        }
    }
    return summary;
}

int ReleaseService::releaseAssignment(const User& hod, int assignmentId) {
    const ReleaseSummary summary = summarise(hod, assignmentId);
    if (summary.submissions == 0) throw RuleViolation("There are no submissions for this assignment");
    if (summary.unmarked > 0 || summary.awaitingOrReturned > 0) {
        throw RuleViolation("Release refused: " + std::to_string(summary.unmarked) + " submission(s) not marked and " +
                            std::to_string(summary.awaitingOrReturned) +
                            " mark(s) not yet approved. No marks were released.");
    }
    if (summary.approved == 0) throw RuleViolation("All marks for this assignment have already been released");

    int released = 0;
    tx_.run([&] {
        for (Mark mark : marks_.listForAssignment(assignmentId)) {
            if (mark.status != MarkStatus::Approved) continue;  // only APPROVED -> RELEASED
            const Mark before = mark;
            mark.status = MarkStatus::Released;
            marks_.update(mark);
            audit_.record(hod.id(), AuditEvent::kMarkReleased, AuditEntity::kMark, mark.id,
                          describeMark(before) + "; status=APPROVED", describeMark(mark) + "; status=RELEASED",
                          "Released by HoD");
            ++released;
        }
    });
    return released;
}

std::vector<ResultView> ReleaseService::resultsFor(const User& student) {
    permissions_.require(student, Permission::ViewOwnResults);
    std::vector<ResultView> result;
    for (const Course& course : courses_.coursesForStudent(student.id())) {
        for (const Assignment& assignment : assignments_.listForCourse(course.id)) {
            auto submission = submissions_.find(assignment.id, student.id());
            if (!submission) continue;
            auto mark = marks_.findBySubmission(submission->id);
            if (mark && mark->status == MarkStatus::Released) result.push_back({assignment, course, *mark});
        }
    }
    return result;
}

}  // namespace mlms
