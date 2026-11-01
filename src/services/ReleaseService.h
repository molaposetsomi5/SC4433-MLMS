#pragma once
// ReleaseService (FR-13) - second half of UC-03, plus the student's results view.
#include <string>
#include <vector>

#include "data/Database.h"
#include "data/Repositories.h"
#include "domain/DateTime.h"
#include "services/AuditService.h"
#include "services/PermissionPolicy.h"

namespace mlms {

struct ResultView {
    Assignment assignment;
    Course course;
    Mark mark;
};

struct ReleaseSummary {
    int submissions = 0;
    int unmarked = 0;
    int awaitingOrReturned = 0;
    int approved = 0;
    int released = 0;
};

class ReleaseService {
public:
    ReleaseService(IMarkRepository& marks, ISubmissionRepository& submissions, IAssignmentRepository& assignments,
                   ICourseRepository& courses, PermissionPolicy& permissions, AuditService& audit,
                   TransactionManager& tx, const IClock& clock);

    // Counts used by the console before asking the HoD to confirm.
    ReleaseSummary summarise(const User& hod, int assignmentId);

    // Releases every approved mark of the assignment in one transaction.
    // A mixed batch (anything unmarked or not yet approved) is rejected as a whole.
    int releaseAssignment(const User& hod, int assignmentId);

    // Only the student's own RELEASED marks.
    std::vector<ResultView> resultsFor(const User& student);

private:
    Assignment loadInScope(const User& hod, int assignmentId);

    IMarkRepository& marks_;
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    PermissionPolicy& permissions_;
    AuditService& audit_;
    TransactionManager& tx_;
    const IClock& clock_;
};

}  // namespace mlms
