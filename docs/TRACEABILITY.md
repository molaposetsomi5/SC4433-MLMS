# Requirements Traceability Matrix

FR/NFR IDs and use cases are from the Week 2 SRS. Classes and tables are from the Week 3 design.

| Req | Priority | Use case | Design elements (classes / diagrams) | Tables | Source files | Test cases |
|---|---|---|---|---|---|---|
| FR-01 Authentication | Must | all (include Authenticate) | AuthService, IPasswordHasher, User | users | services/AuthService.cpp, security/PasswordHasher.cpp | AuthTest.LoginWithValidCredentialsReturnsRoleUser, AuthTest.WrongPasswordAndUnknownUserGiveSameMessage, ST-08, ST-10 |
| FR-02 Role-based access | Must | all | PermissionPolicy, User::hasPermission (Administrator/Lecturer/HeadOfDepartment/Student) | users, lecturer_courses, enrolments | services/PermissionPolicy.cpp, domain/User.cpp | UserPermissions.EachRoleHasOnlyItsOwnPermissions, AdminTest.OnlyAdministratorCanCreateUsers, SubmissionTest.StaffCannotSubmit, ModerationTest.OnlyHodCanRelease, ST-06, ST-07 |
| FR-03 User administration | Must | Manage users | UserManagementService, UserFactory | users | services/UserManagementService.cpp, domain/User.cpp | AdminTest.RejectsInvalidUserDetails, AdminTest.UpdateDetailsChangesNameAndDepartment, AuthTest.DeactivatedAccountCannotLogIn, ST-09 |
| FR-04 Departments and courses | Must | Manage departments and courses | CourseManagementService | departments, courses | services/CourseManagementService.cpp, data/SqliteCourseRepository.cpp | AdminTest.CourseNeedsValidDepartmentAndUniqueCode, AdminTest.DuplicateDepartmentRejected |
| FR-05 Lecturer assignment and enrolment | Must | Assign lecturer / enrol students | CourseManagementService | lecturer_courses, enrolments | services/CourseManagementService.cpp | AdminTest.AssignAndEnrolCheckRoles, AdminTest.CoursesForEachRole, ST-09 |
| FR-06 Assignment creation | Must | Create assignment | AssignmentService, Assignment | assignments | services/AssignmentService.cpp | AdminTest.AssignmentCreationValidatesInput |
| FR-07 Assignment / deadline viewing | Must | UC-01 | AssignmentService::listForStudent, DeadlineResolver | assignments, extensions | services/AssignmentService.cpp, services/DeadlineResolver.cpp | SubmissionTest.ExtensionShownInStudentAssignmentList, ST-01 |
| FR-08 Submission and resubmission | Must | UC-01 main, A1 | SubmissionService, Submission, SubmissionVersion; state machine; sequence "Submit assignment" | submissions, submission_versions | services/SubmissionService.cpp, data/SqliteAssessmentRepositories.cpp | SubmissionTest.OnTimeSubmissionIsRecorded, SubmissionTest.ResubmissionBeforeDeadlineKeepsHistory, SubmissionTest.ResubmissionAfterDeadlineIsRejected, ST-01 |
| FR-09 Late submission and penalty | Must | UC-01 A3/E1, UC-02 A1 | LatePenaltyPolicy (Strategy), StandardLatePenaltyPolicy | marks (penalty, final_mark) | domain/LatePenaltyPolicy.cpp, services/MarkingService.cpp | BVA/LateDaysBoundaries.*, EP_BVA/PenaltyAmounts.*, MarkingTest.LatePenaltyWorkedExample, MarkingTest.MoreThanThreeDaysLateGetsZero, ST-02, ST-04 |
| FR-10 Individual extension | Must | UC-01 A2 | ExtensionService, Extension, DeadlineResolver | extensions | services/ExtensionService.cpp | SubmissionTest.IndividualExtensionMovesThatStudentsDeadlineOnly, SubmissionTest.ExtensionRulesAreEnforced, SubmissionTest.ExtensionCanBeChangedAndIsAudited, MarkingTest.ExtensionCountsTowardsPenaltyClock |
| FR-11 Marking and feedback | Must | UC-02 | MarkingService, Mark | marks | services/MarkingService.cpp | MarkingTest.* (14 tests), ST-04 |
| FR-12 Mark moderation | Must | UC-03 main, A1, E1 | ModerationService, ModerationDecision; sequence "Moderate and release" | moderation_decisions, marks | services/ModerationService.cpp | ModerationTest.ApproveRecordsDecision, ModerationTest.AmendNeedsReasonAndRecalculates, ModerationTest.ReturnSendsMarkBackToLecturer, ModerationTest.WrongDepartmentIsDeniedAndNothingRecorded, ST-05, ST-06 |
| FR-13 Release and student viewing | Must | UC-03 A2, E3 | ReleaseService | marks | services/ReleaseService.cpp | ModerationTest.MixedBatchIsRejectedAsAWhole, ModerationTest.ReleaseShowsEachStudentOnlyTheirOwnMark, ModerationTest.StudentCannotSeeUnreleasedMark, ST-04 |
| FR-14 Mark-change audit trail | Must | include Record audit | AuditService, AuditRecord | audit_logs | services/AuditService.cpp, data/Schema.cpp (triggers) | MarkingTest.NewMarkAwaitsModerationAndIsAudited, MarkingTest.CorrectionNeedsReasonAndIsAudited, AuditTest.EveryMarkChangeHasActorTimeOldNewAndReason |
| FR-15 Submission lifecycle | Must | UC-01..03 | state machine (Submitted/Late; Awaiting/Returned/Approved/Released) | submissions.status, marks.status (CHECK) | services/*Service.cpp | ModerationTest.ReturnSendsMarkBackToLecturer, ModerationTest.LecturerCannotChangeApprovedMark, ModerationTest.ReleasedMarkIsFinal |
| FR-16 Validation and error handling | Must | all E-flows | Errors.h hierarchy, Validation.h, ConsoleUI::attempt | CHECK / FK constraints | domain/Validation.h, ui/ConsoleUI.cpp | ST-03, AuditTest.ForeignKeysAndChecksAreEnforced, BadInputs/InvalidDateTime.* |
| FR-17 Record lookup | Should | - | MarkingService::submissionsFor, AuditFilter | - | services/MarkingService.cpp, services/AuditService.cpp | MarkingTest.LecturerSeesSubmissionsWithLatenessAndOwnMarks, AuditTest.FiltersByAssignmentAndStudent |
| FR-18 Audit log access | Must | - | AuditService::query (system / department scope) | audit_logs | services/AuditService.cpp | AuditTest.AdministratorSeesEverythingHodSeesOwnDepartment, AuditTest.LecturersAndStudentsCannotReadAudit |
| FR-19 Password management | Should | - | AuthService::changePassword | users | services/AuthService.cpp | AuthTest.ChangePasswordNeedsCurrentPassword |
| FR-20 Release notification | Could | - | not implemented in v1.0 (Could item, deferred under the slippage rule) | - | - | - |
| NFR-01 Performance | Must | - | SQLite indexes, local deployment | indexes in Schema.cpp | tools/perf_test.cpp | ST-11 (`mlms_perf`) |
| NFR-02 Offline | Must | - | no network code; local SQLite | - | whole program | acceptance A6 |
| NFR-03 Password protection | Must | - | SodiumPasswordHasher (Argon2id) | users.password_hash | security/PasswordHasher.cpp | AuthTest.PasswordsAreStoredAsSaltedArgon2idHashes |
| NFR-04 Authorisation / confidentiality | Must | - | PermissionPolicy scope checks | - | services/PermissionPolicy.cpp | ModerationTest.ReleaseShowsEachStudentOnlyTheirOwnMark, SubmissionTest.NotEnrolledStudentCannotSubmit, AuditTest.QueriesAreParameterisedAgainstInjection |
| NFR-05 Data integrity | Must | - | TransactionManager, PK/FK/CHECK | all | data/Database.cpp, data/Schema.cpp | AuditTest.FailedTransactionRollsBackEverything, AuditTest.ForeignKeysAndChecksAreEnforced |
| NFR-06 Auditability | Must | - | append-only triggers | audit_logs | data/Schema.cpp | AuditTest.AuditLogCannotBeUpdatedOrDeleted |
| NFR-07 Usability | Should | - | role menus, plain messages | - | ui/ConsoleUI.cpp | ST-03, acceptance A7 |
| NFR-08 Maintainability / portability | Should | - | layered architecture, CMake | - | CMakeLists.txt | CI build, static analysis |
| NFR-09 Backup and recovery | Should | - | single-file database | - | docs/INSTALLATION_GUIDE.md s5-6 | manual restore drill |
| NFR-10 Installation | Should | - | install guide | - | docs/INSTALLATION_GUIDE.md | timed install trial |

## Class-to-code and table-to-class check
Every class in the Week 3 class diagram exists in code with the same name, except:
- `AuthorizationService / PermissionPolicy` is implemented as one class, `PermissionPolicy`.
- `TransactionManager` is in `data/Database.h` next to the RAII wrappers it uses.
- `DeadlineResolver` was added during implementation to avoid repeating the effective-deadline and
  late-day calculation in three services (Submission, Marking, Moderation).
Every table in the Week 3 SQL schema exists in `data/Schema.cpp` with the same columns and constraints, plus
two triggers that make `audit_logs` append-only (the design said database-level protection "can be added").
