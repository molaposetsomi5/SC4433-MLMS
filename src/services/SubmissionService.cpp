#include "services/SubmissionService.h"

#include <algorithm>

#include "domain/Errors.h"
#include "domain/Validation.h"

namespace mlms {

SubmissionService::SubmissionService(IAssignmentRepository& assignments, ISubmissionRepository& submissions,
                                     IMarkRepository& marks, DeadlineResolver& deadlines,
                                     const LatePenaltyPolicy& policy, PermissionPolicy& permissions,
                                     AuditService& audit, TransactionManager& tx, const IClock& clock)
    : assignments_(assignments),
      submissions_(submissions),
      marks_(marks),
      deadlines_(deadlines),
      policy_(policy),
      permissions_(permissions),
      audit_(audit),
      tx_(tx),
      clock_(clock) {}

bool SubmissionService::isClosed(int assignmentId) {
    const auto marks = marks_.listForAssignment(assignmentId);
    return std::any_of(marks.begin(), marks.end(),
                       [](const Mark& m) { return m.status == MarkStatus::Released; });
}

SubmitResult SubmissionService::submit(const User& student, int assignmentId, const std::string& content) {
    // UC-01 step 3: validate references and permissions
    permissions_.require(student, Permission::SubmitWork);
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundError("Assignment not found");
    permissions_.requireEnrolled(student, assignment->courseId);
    const std::string cleanContent = requireText(content, "Submission content");
    if (isClosed(assignmentId)) throw RuleViolation("This assignment is closed - its marks have been released");

    // UC-01 step 4/5: timestamp and on-time / late decision
    const Seconds now = clock_.now();
    const DeadlineInfo deadline = deadlines_.resolve(*assignment, student.id());
    const int lateDays = policy_.lateDays(now, toSeconds(deadline.effective));

    SubmitResult result;
    result.submittedAt = formatDateTime(now);
    result.effectiveDeadline = deadline.effective;
    result.lateDays = lateDays;

    auto existing = submissions_.find(assignmentId, student.id());
    if (existing) {
        // A1: resubmission only before the effective deadline and while unmarked
        if (marks_.findBySubmission(existing->id)) {
            throw RuleViolation("This work has already been marked, so it cannot be resubmitted");
        }
        if (lateDays > 0) {
            throw RuleViolation("Resubmission is only allowed before the deadline (" + deadline.effective + ")");
        }
        result.resubmission = true;
    }

    tx_.run([&] {
        if (existing) {
            result.submissionId = existing->id;
            submissions_.updateStatus(existing->id, SubmissionStatus::Submitted);
        } else {
            Submission s;
            s.assignmentId = assignmentId;
            s.studentId = student.id();
            s.status = lateDays > 0 ? SubmissionStatus::Late : SubmissionStatus::Submitted;
            s.createdAt = result.submittedAt;
            result.submissionId = submissions_.create(s);
        }
        result.versionNumber = submissions_.addVersion(result.submissionId, result.submittedAt, cleanContent);
        result.status = lateDays > 0 ? SubmissionStatus::Late : SubmissionStatus::Submitted;
        audit_.record(student.id(), AuditEvent::kSubmissionRecorded, AuditEntity::kSubmission, result.submissionId,
                      std::nullopt,
                      "version " + std::to_string(result.versionNumber) + "; " + toString(result.status),
                      lateDays > 0 ? "Late submission (" + std::to_string(lateDays) + " day(s) late)"
                                   : (result.resubmission ? "Resubmission before deadline" : "Submission"));
    });

    result.beyondLateWindow = lateDays > StandardLatePenaltyPolicy::kMaxLateDays;
    return result;
}

std::vector<SubmissionVersion> SubmissionService::myVersions(const User& student, int assignmentId) {
    permissions_.require(student, Permission::SubmitWork);
    auto submission = submissions_.find(assignmentId, student.id());
    if (!submission) return {};
    return submissions_.versions(submission->id);
}

}  // namespace mlms
