#pragma once
// Creates and queries audit records (FR-14, FR-18, NFR-06).
#include <optional>
#include <string>
#include <vector>

#include "data/Repositories.h"
#include "domain/DateTime.h"
#include "services/PermissionPolicy.h"

namespace mlms {

namespace AuditEvent {
constexpr const char* kSubmissionRecorded = "SUBMISSION_RECORDED";
constexpr const char* kMarkEntered = "MARK_ENTERED";
constexpr const char* kMarkCorrected = "MARK_CORRECTED";
constexpr const char* kModerationApprove = "MODERATION_APPROVE";
constexpr const char* kModerationAmend = "MODERATION_AMEND";
constexpr const char* kModerationReturn = "MODERATION_RETURN";
constexpr const char* kMarkReleased = "MARK_RELEASED";
constexpr const char* kExtensionGranted = "EXTENSION_GRANTED";
constexpr const char* kExtensionChanged = "EXTENSION_CHANGED";
constexpr const char* kClassExtensionGranted = "CLASS_EXTENSION_GRANTED";  // v1.1
constexpr const char* kClassExtensionChanged = "CLASS_EXTENSION_CHANGED";  // v1.1
}  // namespace AuditEvent

namespace AuditEntity {
constexpr const char* kMark = "MARK";
constexpr const char* kSubmission = "SUBMISSION";
constexpr const char* kExtension = "EXTENSION";
constexpr const char* kAssignment = "ASSIGNMENT";
}  // namespace AuditEntity

struct AuditFilter {
    std::optional<int> courseId;
    std::optional<int> assignmentId;
    std::optional<int> studentId;
};

struct AuditEntryView {
    AuditRecord record;
    std::string actorName;
    std::optional<int> courseId;
    std::optional<int> assignmentId;
    std::optional<int> studentId;
};

class AuditService {
public:
    AuditService(IAuditRepository& audit, IUserRepository& users, ICourseRepository& courses,
                 IAssignmentRepository& assignments, ISubmissionRepository& submissions, IMarkRepository& marks,
                 IExtensionRepository& extensions, PermissionPolicy& permissions, const IClock& clock);

    void record(int actorId, const std::string& eventType, const std::string& entityType, int entityId,
                const std::optional<std::string>& oldValue, const std::optional<std::string>& newValue,
                const std::string& reason);

    // Administrator: whole system. HoD: courses in own department only.
    std::vector<AuditEntryView> query(const User& actor, const AuditFilter& filter);

private:
    void resolveScope(AuditEntryView& view);

    IAuditRepository& audit_;
    IUserRepository& users_;
    ICourseRepository& courses_;
    IAssignmentRepository& assignments_;
    ISubmissionRepository& submissions_;
    IMarkRepository& marks_;
    IExtensionRepository& extensions_;
    PermissionPolicy& permissions_;
    const IClock& clock_;
};

}  // namespace mlms
