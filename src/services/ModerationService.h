#pragma once
// ModerationService (FR-12) - first half of UC-03.
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

struct ModerationItem {
    Mark mark;
    Assignment assignment;
    Course course;
    std::string studentName;
    int lateDays = 0;
    DeadlineInfo deadline;  // shows the HoD any extension (SRS C5)
};

class ModerationService {
public:
    ModerationService(IMarkRepository& marks, ISubmissionRepository& submissions,
                      IAssignmentRepository& assignments, ICourseRepository& courses, IUserRepository& users,
                      DeadlineResolver& deadlines, const LatePenaltyPolicy& policy, PermissionPolicy& permissions,
                      AuditService& audit, TransactionManager& tx, const IClock& clock);

    // Marks awaiting moderation in the HoD's department.
    std::vector<ModerationItem> pending(const User& hod);

    void approve(const User& hod, int markId, const std::string& comment);
    void amend(const User& hod, int markId, double newRawMark, const std::string& reason);
    void returnForCorrection(const User& hod, int markId, const std::string& reason);

private:
    struct Context {
        Mark mark;
        Submission submission;
        Assignment assignment;
    };
    Context loadAwaiting(const User& hod, int markId);
    void saveDecision(const User& hod, const Context& before, const Mark& after, ModerationAction action,
                      const std::string& reason, const char* auditEvent);

    IMarkRepository& marks_;
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    IUserRepository& users_;
    DeadlineResolver& deadlines_;
    const LatePenaltyPolicy& policy_;
    PermissionPolicy& permissions_;
    AuditService& audit_;
    TransactionManager& tx_;
    const IClock& clock_;
};

}  // namespace mlms
