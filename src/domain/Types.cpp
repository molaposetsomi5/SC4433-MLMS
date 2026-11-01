#include "domain/Types.h"

#include "domain/Errors.h"

namespace mlms {

std::string toString(Role role) {
    switch (role) {
        case Role::Administrator: return "ADMINISTRATOR";
        case Role::Lecturer: return "LECTURER";
        case Role::HoD: return "HOD";
        case Role::Student: return "STUDENT";
    }
    return "UNKNOWN";
}

std::string toString(SubmissionStatus status) {
    return status == SubmissionStatus::Submitted ? "SUBMITTED" : "LATE";
}

std::string toString(MarkStatus status) {
    switch (status) {
        case MarkStatus::AwaitingModeration: return "AWAITING_MODERATION";
        case MarkStatus::Returned: return "RETURNED";
        case MarkStatus::Approved: return "APPROVED";
        case MarkStatus::Released: return "RELEASED";
    }
    return "UNKNOWN";
}

std::string toString(ModerationAction action) {
    switch (action) {
        case ModerationAction::Approve: return "APPROVE";
        case ModerationAction::Amend: return "AMEND";
        case ModerationAction::Return: return "RETURN";
    }
    return "UNKNOWN";
}

Role roleFromString(const std::string& text) {
    if (text == "ADMINISTRATOR") return Role::Administrator;
    if (text == "LECTURER") return Role::Lecturer;
    if (text == "HOD") return Role::HoD;
    if (text == "STUDENT") return Role::Student;
    throw ValidationError("Unknown role: " + text);
}

SubmissionStatus submissionStatusFromString(const std::string& text) {
    if (text == "SUBMITTED") return SubmissionStatus::Submitted;
    if (text == "LATE") return SubmissionStatus::Late;
    throw ValidationError("Unknown submission status: " + text);
}

MarkStatus markStatusFromString(const std::string& text) {
    if (text == "AWAITING_MODERATION") return MarkStatus::AwaitingModeration;
    if (text == "RETURNED") return MarkStatus::Returned;
    if (text == "APPROVED") return MarkStatus::Approved;
    if (text == "RELEASED") return MarkStatus::Released;
    throw ValidationError("Unknown mark status: " + text);
}

ModerationAction moderationActionFromString(const std::string& text) {
    if (text == "APPROVE") return ModerationAction::Approve;
    if (text == "AMEND") return ModerationAction::Amend;
    if (text == "RETURN") return ModerationAction::Return;
    throw ValidationError("Unknown moderation action: " + text);
}

}  // namespace mlms
