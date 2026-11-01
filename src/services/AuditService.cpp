#include "services/AuditService.h"

#include <map>
#include <string>

#include "domain/Errors.h"

namespace mlms {

AuditService::AuditService(IAuditRepository& audit, IUserRepository& users, ICourseRepository& courses,
                           IAssignmentRepository& assignments, ISubmissionRepository& submissions,
                           IMarkRepository& marks, IExtensionRepository& extensions, PermissionPolicy& permissions,
                           const IClock& clock)
    : audit_(audit),
      users_(users),
      courses_(courses),
      assignments_(assignments),
      submissions_(submissions),
      marks_(marks),
      extensions_(extensions),
      permissions_(permissions),
      clock_(clock) {}

void AuditService::record(int actorId, const std::string& eventType, const std::string& entityType, int entityId,
                          const std::optional<std::string>& oldValue, const std::optional<std::string>& newValue,
                          const std::string& reason) {
    AuditRecord r;
    r.actorUserId = actorId;
    r.eventType = eventType;
    r.entityType = entityType;
    r.entityId = entityId;
    r.timestamp = clock_.nowText();
    r.oldValue = oldValue;
    r.newValue = newValue;
    r.reason = reason;
    audit_.append(r);
}

void AuditService::resolveScope(AuditEntryView& view) {
    const AuditRecord& r = view.record;
    std::optional<Submission> submission;

    if (r.entityType == AuditEntity::kMark) {
        if (auto mark = marks_.findById(r.entityId)) submission = submissions_.findById(mark->submissionId);
    } else if (r.entityType == AuditEntity::kSubmission) {
        submission = submissions_.findById(r.entityId);
    } else if (r.entityType == AuditEntity::kExtension) {
        if (auto ext = extensions_.findById(r.entityId)) {
            view.assignmentId = ext->assignmentId;
            view.studentId = ext->studentId;
        }
    } else if (r.entityType == AuditEntity::kAssignment) {
        view.assignmentId = r.entityId;
    }

    if (submission) {
        view.assignmentId = submission->assignmentId;
        view.studentId = submission->studentId;
    }
    if (view.assignmentId) {
        if (auto assignment = assignments_.findById(*view.assignmentId)) view.courseId = assignment->courseId;
    }
}

std::vector<AuditEntryView> AuditService::query(const User& actor, const AuditFilter& filter) {
    const bool systemWide = actor.hasPermission(Permission::ViewAllAudit);
    if (!systemWide) permissions_.require(actor, Permission::ViewDepartmentAudit);

    std::map<int, std::string> names;
    std::vector<AuditEntryView> result;
    for (const AuditRecord& r : audit_.listAll()) {
        AuditEntryView view;
        view.record = r;
        resolveScope(view);

        if (!systemWide && (!view.courseId || !permissions_.inDepartment(actor, *view.courseId))) continue;
        if (filter.courseId && view.courseId != filter.courseId) continue;
        if (filter.assignmentId && view.assignmentId != filter.assignmentId) continue;
        if (filter.studentId && view.studentId != filter.studentId) continue;

        if (names.find(r.actorUserId) == names.end()) {
            auto user = users_.findById(r.actorUserId);
            names[r.actorUserId] = user ? user->username() : "user#" + std::to_string(r.actorUserId);
        }
        view.actorName = names[r.actorUserId];
        result.push_back(view);
    }
    return result;
}

}  // namespace mlms
