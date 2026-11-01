#pragma once
// Plain domain records. Each struct maps to one table (see design section 6.4).
#include <optional>
#include <string>

namespace mlms {

enum class Role { Administrator, Lecturer, HoD, Student };
enum class SubmissionStatus { Submitted, Late };
enum class MarkStatus { AwaitingModeration, Returned, Approved, Released };
enum class ModerationAction { Approve, Amend, Return };

std::string toString(Role role);
std::string toString(SubmissionStatus status);
std::string toString(MarkStatus status);
std::string toString(ModerationAction action);

Role roleFromString(const std::string& text);
SubmissionStatus submissionStatusFromString(const std::string& text);
MarkStatus markStatusFromString(const std::string& text);
ModerationAction moderationActionFromString(const std::string& text);

struct Department {
    int id = 0;
    std::string name;
};

struct Course {
    int id = 0;
    int departmentId = 0;
    std::string code;
    std::string name;
};

struct Assignment {
    int id = 0;
    int courseId = 0;
    std::string title;
    std::string description;
    std::string deadline;  // original deadline
    double maxMark = 0;
    int createdBy = 0;
    std::string createdAt;
};

struct Submission {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    SubmissionStatus status = SubmissionStatus::Submitted;
    std::string createdAt;
};

struct SubmissionVersion {
    int id = 0;
    int submissionId = 0;
    int versionNumber = 0;
    std::string submittedAt;
    std::string contentReference;
    bool isLatestValid = true;
};

struct Extension {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    std::string revisedDeadline;
    std::string reason;
    int grantedBy = 0;
    std::string grantedAt;
};

struct Mark {
    int id = 0;
    int submissionId = 0;
    double rawMark = 0;
    double penalty = 0;
    double finalMark = 0;
    std::string feedback;
    MarkStatus status = MarkStatus::AwaitingModeration;
    int markedBy = 0;
    std::string markedAt;
};

struct ModerationDecision {
    int id = 0;
    int markId = 0;
    int hodUserId = 0;
    ModerationAction decision = ModerationAction::Approve;
    std::optional<double> previousMark;
    std::optional<double> newMark;
    std::string reason;
    std::string decidedAt;
};

struct AuditRecord {
    int id = 0;
    int actorUserId = 0;
    std::string eventType;
    std::string entityType;
    int entityId = 0;
    std::string timestamp;
    std::optional<std::string> oldValue;
    std::optional<std::string> newValue;
    std::string reason;
};

}  // namespace mlms
