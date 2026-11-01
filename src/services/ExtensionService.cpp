#include "services/ExtensionService.h"

#include "domain/Errors.h"
#include "domain/Validation.h"

namespace mlms {

ExtensionService::ExtensionService(IAssignmentRepository& assignments, ICourseRepository& courses,
                                   ISubmissionRepository& submissions, IExtensionRepository& extensions,
                                   IMarkRepository& marks, PermissionPolicy& permissions, AuditService& audit,
                                   TransactionManager& tx, const IClock& clock)
    : assignments_(assignments),
      courses_(courses),
      submissions_(submissions),
      extensions_(extensions),
      marks_(marks),
      permissions_(permissions),
      audit_(audit),
      tx_(tx),
      clock_(clock) {}

Assignment ExtensionService::loadTaughtAssignment(const User& lecturer, int assignmentId) {
    permissions_.require(lecturer, Permission::GrantExtension);
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundError("Assignment not found");
    permissions_.requireTeaches(lecturer, assignment->courseId);
    return *assignment;
}

std::string ExtensionService::grantIndividual(const User& lecturer, int assignmentId, int studentId,
                                              const std::string& revisedDeadline, const std::string& reason) {
    const Assignment assignment = loadTaughtAssignment(lecturer, assignmentId);
    if (!courses_.isEnrolled(studentId, assignment.courseId)) {
        throw ValidationError("That student is not enrolled in this course");
    }

    Extension ext;
    ext.assignmentId = assignmentId;
    ext.studentId = studentId;
    ext.revisedDeadline = normaliseDateTime(trim(revisedDeadline));
    ext.reason = requireText(reason, "Reason for the extension");
    ext.grantedBy = lecturer.id();
    ext.grantedAt = clock_.nowText();

    if (toSeconds(ext.revisedDeadline) <= toSeconds(assignment.deadline)) {
        throw ValidationError("The revised deadline must be after the original deadline (" + assignment.deadline +
                              ")");
    }
    if (auto submission = submissions_.find(assignmentId, studentId)) {
        if (marks_.findBySubmission(submission->id)) {
            throw RuleViolation("This submission has already been marked; the deadline can no longer change");
        }
    }

    auto previous = extensions_.find(assignmentId, studentId);
    tx_.run([&] {
        extensions_.save(ext);
        auto saved = extensions_.find(assignmentId, studentId);
        audit_.record(lecturer.id(), previous ? AuditEvent::kExtensionChanged : AuditEvent::kExtensionGranted,
                      AuditEntity::kExtension, saved->id,
                      previous ? std::optional<std::string>(previous->revisedDeadline) : std::nullopt,
                      ext.revisedDeadline, ext.reason);
    });
    return ext.revisedDeadline;
}

}  // namespace mlms
