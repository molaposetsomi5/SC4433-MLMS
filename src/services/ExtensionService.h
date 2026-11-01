#pragma once
// ExtensionService (FR-10).
#include <string>

#include "data/Database.h"
#include "data/Repositories.h"
#include "domain/DateTime.h"
#include "services/AuditService.h"
#include "services/PermissionPolicy.h"

namespace mlms {

class ExtensionService {
public:
    ExtensionService(IAssignmentRepository& assignments, ICourseRepository& courses,
                     ISubmissionRepository& submissions, IExtensionRepository& extensions, IMarkRepository& marks,
                     PermissionPolicy& permissions, AuditService& audit, TransactionManager& tx,
                     const IClock& clock);

    // Grants (or changes) one student's extension. Returns the stored revised deadline.
    std::string grantIndividual(const User& lecturer, int assignmentId, int studentId,
                                const std::string& revisedDeadline, const std::string& reason);

private:
    Assignment loadTaughtAssignment(const User& lecturer, int assignmentId);

    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    ISubmissionRepository& submissions_;
    IExtensionRepository& extensions_;
    IMarkRepository& marks_;
    PermissionPolicy& permissions_;
    AuditService& audit_;
    TransactionManager& tx_;
    const IClock& clock_;
};

}  // namespace mlms
