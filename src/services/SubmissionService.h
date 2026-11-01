#pragma once
// SubmissionService (FR-08, FR-09, FR-15) - implements UC-01 "Submit assignment".
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

struct SubmitResult {
    int submissionId = 0;
    int versionNumber = 0;
    SubmissionStatus status = SubmissionStatus::Submitted;
    std::string submittedAt;
    std::string effectiveDeadline;
    int lateDays = 0;
    bool resubmission = false;
    bool beyondLateWindow = false;  // more than 3 days late -> final mark will be zero
};

class SubmissionService {
public:
    SubmissionService(IAssignmentRepository& assignments, ISubmissionRepository& submissions,
                      IMarkRepository& marks, DeadlineResolver& deadlines, const LatePenaltyPolicy& policy,
                      PermissionPolicy& permissions, AuditService& audit, TransactionManager& tx,
                      const IClock& clock);

    SubmitResult submit(const User& student, int assignmentId, const std::string& content);

    std::vector<SubmissionVersion> myVersions(const User& student, int assignmentId);

    // An assignment is closed once the HoD has released its marks (SRS C15).
    bool isClosed(int assignmentId);

private:
    IAssignmentRepository& assignments_;
    ISubmissionRepository& submissions_;
    IMarkRepository& marks_;
    DeadlineResolver& deadlines_;
    const LatePenaltyPolicy& policy_;
    PermissionPolicy& permissions_;
    AuditService& audit_;
    TransactionManager& tx_;
    const IClock& clock_;
};

}  // namespace mlms
