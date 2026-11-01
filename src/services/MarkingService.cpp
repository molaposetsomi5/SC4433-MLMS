#include "services/MarkingService.h"

#include "domain/Errors.h"
#include "domain/Validation.h"

namespace mlms {

std::string describeMark(const Mark& mark) {
    return "raw=" + formatMark(mark.rawMark) + "; penalty=" + formatMark(mark.penalty) +
           "; final=" + formatMark(mark.finalMark);
}

MarkingService::MarkingService(IAssignmentRepository& assignments, ISubmissionRepository& submissions,
                               IMarkRepository& marks, IUserRepository& users, DeadlineResolver& deadlines,
                               const LatePenaltyPolicy& policy, PermissionPolicy& permissions, AuditService& audit,
                               TransactionManager& tx, const IClock& clock)
    : assignments_(assignments),
      submissions_(submissions),
      marks_(marks),
      users_(users),
      deadlines_(deadlines),
      policy_(policy),
      permissions_(permissions),
      audit_(audit),
      tx_(tx),
      clock_(clock) {}

std::vector<SubmissionView> MarkingService::submissionsFor(const User& lecturer, int assignmentId) {
    permissions_.require(lecturer, Permission::MarkSubmission);
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundError("Assignment not found");
    permissions_.requireTeaches(lecturer, assignment->courseId);

    std::vector<SubmissionView> result;
    for (const Submission& s : submissions_.listForAssignment(assignmentId)) {
        SubmissionView view;
        view.submission = s;
        if (auto student = users_.findById(s.studentId)) {
            view.studentName = student->displayName();
            view.studentNumber = student->record().studentNumber.value_or("");
        }
        view.latest = submissions_.latestVersion(s.id);
        view.versionCount = static_cast<int>(submissions_.versions(s.id).size());
        view.effectiveDeadline = deadlines_.resolve(*assignment, s.studentId).effective;
        view.lateDays = deadlines_.lateDays(*assignment, s);
        view.mark = marks_.findBySubmission(s.id);
        result.push_back(view);
    }
    return result;
}

MarkingService::Context MarkingService::loadForMarking(const User& lecturer, int submissionId) {
    permissions_.require(lecturer, Permission::MarkSubmission);
    auto submission = submissions_.findById(submissionId);
    if (!submission) throw NotFoundError("Submission not found");
    auto assignment = assignments_.findById(submission->assignmentId);
    if (!assignment) throw NotFoundError("Assignment not found");
    permissions_.requireTeaches(lecturer, assignment->courseId);
    return {*submission, *assignment};
}

Mark MarkingService::markSubmission(const User& lecturer, int submissionId, double rawMark,
                                    const std::string& feedback) {
    const Context ctx = loadForMarking(lecturer, submissionId);

    // Marking only after the effective deadline (SRS C2) so students can still resubmit.
    const DeadlineInfo deadline = deadlines_.resolve(ctx.assignment, ctx.submission.studentId);
    if (clock_.now() <= toSeconds(deadline.effective)) {
        throw RuleViolation("This submission cannot be marked until its deadline has passed (" + deadline.effective +
                            ")");
    }
    if (marks_.findBySubmission(submissionId)) {
        throw RuleViolation("This submission already has a mark. Use 'correct a mark' instead");
    }
    requireMarkInRange(rawMark, ctx.assignment.maxMark);

    const PenaltyResult penalty =
        policy_.apply(rawMark, ctx.assignment.maxMark, deadlines_.lateDays(ctx.assignment, ctx.submission));

    Mark mark;
    mark.submissionId = submissionId;
    mark.rawMark = rawMark;
    mark.penalty = penalty.penalty;
    mark.finalMark = penalty.finalMark;
    mark.feedback = trim(feedback);
    mark.status = MarkStatus::AwaitingModeration;
    mark.markedBy = lecturer.id();
    mark.markedAt = clock_.nowText();

    tx_.run([&] {
        mark.id = marks_.insert(mark);
        std::string reason = "Initial mark entry";
        if (penalty.lateDays > 0) {
            reason += "; late penalty applied for " + std::to_string(penalty.lateDays) + " day(s)";
        }
        audit_.record(lecturer.id(), AuditEvent::kMarkEntered, AuditEntity::kMark, mark.id, std::nullopt,
                      describeMark(mark), reason);
    });
    return mark;
}

Mark MarkingService::correctMark(const User& lecturer, int submissionId, double rawMark,
                                 const std::string& feedback, const std::string& reason) {
    const Context ctx = loadForMarking(lecturer, submissionId);
    auto existing = marks_.findBySubmission(submissionId);
    if (!existing) throw NotFoundError("This submission has not been marked yet");
    if (existing->status != MarkStatus::AwaitingModeration && existing->status != MarkStatus::Returned) {
        throw RuleViolation("Only marks awaiting moderation or returned by the HoD can be corrected");
    }
    const std::string cleanReason = requireText(reason, "Reason for the change");
    requireMarkInRange(rawMark, ctx.assignment.maxMark);

    const PenaltyResult penalty =
        policy_.apply(rawMark, ctx.assignment.maxMark, deadlines_.lateDays(ctx.assignment, ctx.submission));

    Mark updated = *existing;
    updated.rawMark = rawMark;
    updated.penalty = penalty.penalty;
    updated.finalMark = penalty.finalMark;
    if (!trim(feedback).empty()) updated.feedback = trim(feedback);
    updated.status = MarkStatus::AwaitingModeration;  // goes back to the HoD
    updated.markedBy = lecturer.id();
    updated.markedAt = clock_.nowText();

    tx_.run([&] {
        marks_.update(updated);
        audit_.record(lecturer.id(), AuditEvent::kMarkCorrected, AuditEntity::kMark, updated.id,
                      describeMark(*existing), describeMark(updated), cleanReason);
    });
    return updated;
}

}  // namespace mlms
