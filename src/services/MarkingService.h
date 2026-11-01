#pragma once
// MarkingService (FR-11, FR-14) - implements UC-02 "Mark submission and provide feedback".
#include <optional>
#include <string>
#include <vector>

#include "data/Database.h"
#include "data/Repositories.h"
#include "domain/DateTime.h"
#include "domain/LatePenaltyPolicy.h"
#include "services/AuditService.h"
#include "services/DeadlineResolver.h"
#include "services/PermissionPolicy.h"

namespace mlms {

struct SubmissionView {
    Submission submission;
    std::string studentName;
    std::string studentNumber;
    std::optional<SubmissionVersion> latest;
    int versionCount = 0;
    std::string effectiveDeadline;
    int lateDays = 0;
    std::optional<Mark> mark;  // lecturers see their own marks straight away (SRS C1)
};

class MarkingService {
public:
    MarkingService(IAssignmentRepository& assignments, ISubmissionRepository& submissions, IMarkRepository& marks,
                   IUserRepository& users, DeadlineResolver& deadlines, const LatePenaltyPolicy& policy,
                   PermissionPolicy& permissions, AuditService& audit, TransactionManager& tx,
                   const IClock& clock);

    std::vector<SubmissionView> submissionsFor(const User& lecturer, int assignmentId);

    Mark markSubmission(const User& lecturer, int submissionId, double rawMark, const std::string& feedback);

    // Change a mark that is awaiting moderation or was returned by the HoD. Reason is mandatory.
    Mark correctMark(const User& lecturer, int submissionId, double rawMark, const std::string& feedback,
                     const std::string& reason);

private:
    struct Context {
        Submission submission;
        Assignment assignment;
    };
    Context loadForMarking(const User& lecturer, int submissionId);

    IAssignmentRepository& assignments_;
    ISubmissionRepository& submissions_;
    IMarkRepository& marks_;
    IUserRepository& users_;
    DeadlineResolver& deadlines_;
    const LatePenaltyPolicy& policy_;
    PermissionPolicy& permissions_;
    AuditService& audit_;
    TransactionManager& tx_;
    const IClock& clock_;
};

// "raw=40.00; penalty=10.00; final=30.00" - used in audit records.
std::string describeMark(const Mark& mark);

}  // namespace mlms
