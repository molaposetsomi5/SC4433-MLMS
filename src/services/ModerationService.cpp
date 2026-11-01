#include "services/ModerationService.h"

#include "domain/Errors.h"
#include "domain/Validation.h"
#include "services/MarkingService.h"

namespace mlms {

ModerationService::ModerationService(IMarkRepository& marks, ISubmissionRepository& submissions,
                                     IAssignmentRepository& assignments, ICourseRepository& courses,
                                     IUserRepository& users, DeadlineResolver& deadlines,
                                     const LatePenaltyPolicy& policy, PermissionPolicy& permissions,
                                     AuditService& audit, TransactionManager& tx, const IClock& clock)
    : marks_(marks),
      submissions_(submissions),
      assignments_(assignments),
      courses_(courses),
      users_(users),
      deadlines_(deadlines),
      policy_(policy),
      permissions_(permissions),
      audit_(audit),
      tx_(tx),
      clock_(clock) {}

std::vector<ModerationItem> ModerationService::pending(const User& hod) {
    permissions_.require(hod, Permission::ModerateMarks);
    std::vector<ModerationItem> result;
    const std::optional<int> dept = hod.departmentId();
    if (!dept) return result;

    for (const Course& course : courses_.coursesForDepartment(*dept)) {
        for (const Assignment& assignment : assignments_.listForCourse(course.id)) {
            for (const Mark& mark : marks_.listForAssignment(assignment.id)) {
                if (mark.status != MarkStatus::AwaitingModeration) continue;
                auto submission = submissions_.findById(mark.submissionId);
                if (!submission) continue;
                ModerationItem item;
                item.mark = mark;
                item.assignment = assignment;
                item.course = course;
                if (auto student = users_.findById(submission->studentId)) item.studentName = student->displayName();
                item.lateDays = deadlines_.lateDays(assignment, *submission);
                item.deadline = deadlines_.resolve(assignment, submission->studentId);
                result.push_back(item);
            }
        }
    }
    return result;
}

ModerationService::Context ModerationService::loadAwaiting(const User& hod, int markId) {
    permissions_.require(hod, Permission::ModerateMarks);
    auto mark = marks_.findById(markId);
    if (!mark) throw NotFoundError("Mark not found");
    auto submission = submissions_.findById(mark->submissionId);
    auto assignment = submission ? assignments_.findById(submission->assignmentId) : std::nullopt;
    if (!submission || !assignment) throw NotFoundError("Submission for this mark not found");
    // E1: wrong department - checked before anything is recorded
    permissions_.requireDepartmentScope(hod, assignment->courseId);
    if (mark->status != MarkStatus::AwaitingModeration) {
        throw RuleViolation("Only marks awaiting moderation can be moderated (current status: " +
                            toString(mark->status) + ")");
    }
    return {*mark, *submission, *assignment};
}

void ModerationService::saveDecision(const User& hod, const Context& before, const Mark& after,
                                     ModerationAction action, const std::string& reason, const char* auditEvent) {
    ModerationDecision decision;
    decision.markId = before.mark.id;
    decision.hodUserId = hod.id();
    decision.decision = action;
    decision.previousMark = before.mark.finalMark;
    decision.newMark = after.finalMark;
    decision.reason = reason;
    decision.decidedAt = clock_.nowText();

    tx_.run([&] {
        marks_.update(after);
        marks_.addDecision(decision);
        audit_.record(hod.id(), auditEvent, AuditEntity::kMark, before.mark.id,
                      describeMark(before.mark) + "; status=" + toString(before.mark.status),
                      describeMark(after) + "; status=" + toString(after.status), reason);
    });
}

void ModerationService::approve(const User& hod, int markId, const std::string& comment) {
    const Context ctx = loadAwaiting(hod, markId);
    Mark after = ctx.mark;
    after.status = MarkStatus::Approved;
    const std::string reason = trim(comment).empty() ? "Approved by HoD" : trim(comment);
    saveDecision(hod, ctx, after, ModerationAction::Approve, reason, AuditEvent::kModerationApprove);
}

void ModerationService::amend(const User& hod, int markId, double newRawMark, const std::string& reason) {
    const Context ctx = loadAwaiting(hod, markId);
    const std::string cleanReason = requireText(reason, "Reason for the amendment");
    requireMarkInRange(newRawMark, ctx.assignment.maxMark);

    // late penalty is recalculated on the amended raw mark
    const PenaltyResult penalty =
        policy_.apply(newRawMark, ctx.assignment.maxMark, deadlines_.lateDays(ctx.assignment, ctx.submission));
    Mark after = ctx.mark;
    after.rawMark = newRawMark;
    after.penalty = penalty.penalty;
    after.finalMark = penalty.finalMark;
    after.status = MarkStatus::Approved;
    saveDecision(hod, ctx, after, ModerationAction::Amend, cleanReason, AuditEvent::kModerationAmend);
}

void ModerationService::returnForCorrection(const User& hod, int markId, const std::string& reason) {
    const Context ctx = loadAwaiting(hod, markId);
    const std::string cleanReason = requireText(reason, "Reason for returning the mark");
    Mark after = ctx.mark;
    after.status = MarkStatus::Returned;
    saveDecision(hod, ctx, after, ModerationAction::Return, cleanReason, AuditEvent::kModerationReturn);
}

}  // namespace mlms
