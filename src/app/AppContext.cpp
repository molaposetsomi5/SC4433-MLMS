#include "app/AppContext.h"

#include <utility>

#include "data/Schema.h"

namespace mlms {

AppContext::AppContext(const std::string& databasePath, std::unique_ptr<IClock> clockIn,
                       std::unique_ptr<IPasswordHasher> hasherIn)
    : clockPtr(std::move(clockIn)),
      hasherPtr(std::move(hasherIn)),
      clock(*clockPtr),
      hasher(*hasherPtr),
      db(databasePath),
      tx(db),
      userRepo(db),
      courseRepo(db),
      assignmentRepo(db),
      submissionRepo(db),
      extensionRepo(db),
      markRepo(db),
      auditRepo(db),
      permissions(courseRepo),
      deadlines(extensionRepo, submissionRepo, penaltyPolicy),
      audit(auditRepo, userRepo, courseRepo, assignmentRepo, submissionRepo, markRepo, extensionRepo, permissions,
            clock),
      auth(userRepo, hasher, clock),
      users(userRepo, courseRepo, hasher, auth, permissions, clock),
      courses(courseRepo, userRepo, permissions, clock),
      assignments(assignmentRepo, courseRepo, submissionRepo, markRepo, deadlines, permissions, clock),
      extensions(assignmentRepo, courseRepo, submissionRepo, extensionRepo, markRepo, permissions, audit, tx,
                 clock),
      submissions(assignmentRepo, submissionRepo, markRepo, deadlines, penaltyPolicy, permissions, audit, tx,
                  clock),
      marking(assignmentRepo, submissionRepo, markRepo, userRepo, deadlines, penaltyPolicy, permissions, audit, tx,
              clock),
      moderation(markRepo, submissionRepo, assignmentRepo, courseRepo, userRepo, deadlines, penaltyPolicy,
                 permissions, audit, tx, clock),
      release(markRepo, submissionRepo, assignmentRepo, courseRepo, permissions, audit, tx, clock) {
    migrateSchema(db);
}

}  // namespace mlms
