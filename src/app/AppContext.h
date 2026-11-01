#pragma once
// Composition root: builds every object once and injects dependencies
// through constructors (design section 4.4, pattern 3).
#include <memory>
#include <string>

#include "data/Database.h"
#include "data/SqliteRepositories.h"
#include "domain/DateTime.h"
#include "domain/LatePenaltyPolicy.h"
#include "security/PasswordHasher.h"
#include "services/AssignmentService.h"
#include "services/AuditService.h"
#include "services/AuthService.h"
#include "services/CourseManagementService.h"
#include "services/DeadlineResolver.h"
#include "services/ExtensionService.h"
#include "services/MarkingService.h"
#include "services/ModerationService.h"
#include "services/PermissionPolicy.h"
#include "services/ReleaseService.h"
#include "services/SubmissionService.h"
#include "services/UserManagementService.h"

namespace mlms {

class AppContext {
public:
    AppContext(const std::string& databasePath, std::unique_ptr<IClock> clock,
               std::unique_ptr<IPasswordHasher> hasher);

    // order matters: members are built top to bottom
    std::unique_ptr<IClock> clockPtr;
    std::unique_ptr<IPasswordHasher> hasherPtr;
    IClock& clock;
    IPasswordHasher& hasher;

    Database db;
    TransactionManager tx;

    SqliteUserRepository userRepo;
    SqliteCourseRepository courseRepo;
    SqliteAssignmentRepository assignmentRepo;
    SqliteSubmissionRepository submissionRepo;
    SqliteExtensionRepository extensionRepo;
    SqliteMarkRepository markRepo;
    SqliteAuditRepository auditRepo;

    StandardLatePenaltyPolicy penaltyPolicy;
    PermissionPolicy permissions;
    DeadlineResolver deadlines;

    AuditService audit;
    AuthService auth;
    UserManagementService users;
    CourseManagementService courses;
    AssignmentService assignments;
    ExtensionService extensions;
    SubmissionService submissions;
    MarkingService marking;
    ModerationService moderation;
    ReleaseService release;
};

}  // namespace mlms
