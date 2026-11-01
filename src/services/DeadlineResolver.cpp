#include "services/DeadlineResolver.h"

#include "domain/DateTime.h"

namespace mlms {

DeadlineInfo DeadlineResolver::resolve(const Assignment& assignment, int studentId) const {
    DeadlineInfo info;
    info.original = assignment.deadline;
    info.effective = assignment.deadline;
    if (auto ext = extensions_.find(assignment.id, studentId)) {
        info.effective = ext->revisedDeadline;
        info.individualExtension = true;
        info.extensionReason = ext->reason;
    }
    return info;
}

int DeadlineResolver::lateDays(const Assignment& assignment, const Submission& submission) const {
    auto latest = submissions_.latestVersion(submission.id);
    if (!latest) return 0;
    const DeadlineInfo info = resolve(assignment, submission.studentId);
    return policy_.lateDays(toSeconds(latest->submittedAt), toSeconds(info.effective));
}

}  // namespace mlms
