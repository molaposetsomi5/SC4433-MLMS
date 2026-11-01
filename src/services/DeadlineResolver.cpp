#include "services/DeadlineResolver.h"

#include "domain/DateTime.h"

namespace mlms {

DeadlineInfo DeadlineResolver::resolve(const Assignment& assignment, int studentId) const {
    DeadlineInfo info;
    info.original = assignment.deadline;
    info.effective = assignment.deadline;
    Seconds effective = toSeconds(assignment.deadline);
    if (auto cls = extensions_.findClassExtension(assignment.id)) {
        if (toSeconds(cls->revisedDeadline) > effective) {
            effective = toSeconds(cls->revisedDeadline);
            info.effective = cls->revisedDeadline;
            info.classExtension = true;
            info.extensionReason = cls->reason;
        }
    }
    if (auto ext = extensions_.find(assignment.id, studentId)) {
        if (toSeconds(ext->revisedDeadline) > effective) {
            info.effective = ext->revisedDeadline;
            info.individualExtension = true;
            info.classExtension = false;
            info.extensionReason = ext->reason;
        }
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
