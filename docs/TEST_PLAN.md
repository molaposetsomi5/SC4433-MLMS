# MLMS Test Plan (V-Model)

## 1. V-Model mapping
| Level | Verifies (left side of the V) | How | Where | Entry criteria | Exit criteria |
|---|---|---|---|---|---|
| Unit | Detailed design: single classes (`StandardLatePenaltyPolicy`, `DateTime`, `User` hierarchy) | GoogleTest, no database | `test_late_penalty.cpp`, `test_datetime.cpp`, `test_users_and_auth.cpp` (UserFactory/UserPermissions) | class compiles with no warnings; design reviewed (Gate 2) | all unit tests pass; business-logic line coverage >= 70% |
| Integration | Architecture: services + repositories + SQLite working together | GoogleTest fixture `LmsTest` with an in-memory database, fixed clock and fast hasher | `test_admin.cpp`, `test_submission.cpp`, `test_marking.cpp`, `test_moderation_release.cpp`, `test_audit_and_integrity.cpp`, `AuthTest` | unit level passed; schema migrates cleanly | all pass; every Must FR has at least one test; no open Critical/Major defects |
| System | Requirements (SRS) via the use cases, through the real console | Scripted keyboard input into `ConsoleUI`, output checked (ST-01..ST-10) | `test_system_console.cpp`, `mlms_perf` (NFR-01) | integration passed; build is the release candidate | all system tests pass; NFR-01 under 2 s |
| Acceptance | Stakeholder needs (Principal, HoD, Lecturer, Student rep, IT officer) | Walkthrough of the demo database, one session per role, using the acceptance checklist (section 5) | manual, recorded in section 5 | system level passed; v1.0 tagged | every acceptance item passed in the walkthrough |

All automated levels run in CI on every push (`.github/workflows/ci.yml`).

## 2. Test design techniques

### 2.1 Equivalence partitioning - submission time vs effective deadline
| Partition | Example input | Expected |
|---|---|---|
| P1 on time (t <= deadline) | 1 h before | status SUBMITTED, 0 late days, no penalty |
| P2 late within window (0 < t - deadline <= 72 h) | 30 h late | status LATE, 2 days, 20% of max deducted |
| P3 beyond window (> 72 h) | 4 days late | accepted, status LATE, final mark 0 |

### 2.2 Boundary value analysis - late days
| Offset from deadline | Late days | Test |
|---|---|---|
| -1 s | 0 | `BVA/.../minus1s` |
| 0 s (exactly at deadline) | 0 | `BVA/.../0s`, `SubmissionTest.ExactlyAtDeadlineIsOnTime` |
| +1 s | 1 | `BVA/.../1s`, `SubmissionTest.OneSecondAfterDeadlineIsLate` |
| +24 h exactly | 1 | `BVA/.../86400s` |
| +24 h + 1 s | 2 | `BVA/.../86401s` |
| +72 h exactly | 3 | `BVA/.../259200s` |
| +72 h + 1 s | 4 -> zero | `BVA/.../259201s`, `MarkingTest.MoreThanThreeDaysLateGetsZero` |

### 2.3 Boundary value analysis - mark range (max 50)
| Input | Expected | Test |
|---|---|---|
| -0.5 | rejected | `MarkingTest.MarkMustBeWithinRange` |
| 0 | accepted | `MarkingTest.BoundaryMarksAccepted` |
| 50 | accepted | `MarkingTest.BoundaryMarksAccepted` |
| 50.01 | rejected | `MarkingTest.MarkMustBeWithinRange` |

### 2.4 Penalty amounts (EP + BVA) - `EP_BVA/PenaltyAmounts`
Raw 40/50 at 0,1,2,3,4 days -> 40, 35, 30, 25, 0. Penalty larger than the mark floors at 0 (raw 10, 3 days).

## 3. System test cases (derived from the use-case specifications)
| ID | Use case / flow | Steps (summary) | Expected | Result |
|---|---|---|---|---|
| ST-01 | UC-01 main + A1 | student submits, resubmits before deadline, lists assignments | "Submitted", "Resubmitted", version 2 | Pass |
| ST-02 | UC-01 A3 + E1 | submit 1 day late; another student 5 days late | LATE + "1 day(s) late"; "final mark will be 0" | Pass |
| ST-03 | UC-01 E2, FR-16 | unknown assignment, letters for a number, blank content, bad menu option | clear `[!]` messages, menu continues, clean exit | Pass |
| ST-04 | UC-02 main + E1, UC-03 main | invalid mark 60 rejected; mark 40 (2 days late -> 30); student sees nothing; HoD approves + releases; student sees 30/50 + feedback | as stated | Pass |
| ST-05 | UC-03 A1 | HoD returns mark; lecturer corrects | status back to AWAITING_MODERATION | Pass |
| ST-06 | UC-03 A2 + E1 | release with one unmarked submission; other HoD tries to approve | "Release refused"; "not in your department" | Pass |
| ST-07 | UC-02 E2 | lecturer of another course tries to mark | denied, no mark stored | Pass |
| ST-08 | FR-01 / NFR-03 | 5 wrong passwords then correct one | "Account locked", no welcome | Pass |
| ST-09 | FR-03, FR-05 | admin creates a student and enrols them | user created and enrolled | Pass |
| ST-10 | first run | empty database: weak password rejected, admin created, login works | as stated | Pass |
| ST-11 | NFR-01 | `mlms_perf`: 800 students, login + assignment list | both under 2000 ms (measured 74 ms and 0.3 ms) | Pass |

## 3a. Added in v1.1 (CR1)
| ID | Use case / flow | Steps (summary) | Expected | Result |
|---|---|---|---|---|
| ST-12 | Grant class extension | lecturer option 8 extends an assignment; a student lists assignments | "Deadline extended for the whole class"; student sees "extended for the whole class (was ...)" | Pass |
| ST-13 | Upgrade | v1.0 database file opened with v1.1 | schema 1 -> 2, all data kept | Pass (manual run, see CR1 analysis section 4; automated as `ClassExtensionTest.VersionOneDatabaseIsUpgraded`) |

Regression rule: on every change the whole automated suite is rerun in CI, not only the new tests.

## 4. Results summary (v1.0)
- 116 automated tests, 116 passed (`docs/test_results_v1.0.txt`).
- Business-logic line coverage (src/domain + src/services): **95.5%** (863/904 lines), function coverage 97.7%.
- Whole `src/` line coverage: 79.9% (console and demo-data code are covered mainly by system tests).
- NFR-01: login 74 ms, assignment list 0.3 ms with 800 students (Argon2id interactive settings).

## 4a. Results summary (v1.1)
- 128 automated tests, 128 passed - the 116 v1.0 tests unchanged plus 12 CR1 tests (`docs/test_results_v1.1.txt`).
- Business-logic line coverage: **95.9%** (902/941 lines).
- NFR-01: login 80 ms, assignment list 0.5 ms.

## 5. Acceptance checklist (run with the demo database: `mlms --seed-demo`)
The automated equivalent of each item already passes. Fill in the last column when the walkthrough is done
(e.g. with the supervisor at Gate 3 or during the viva).

| # | Stakeholder need | Steps | Automated equivalent | Walkthrough result |
|---|---|---|---|---|
| A1 | Principal: no mark visible before HoD check | stu1 submits Lab 1, lec.molefi marks it, stu1 checks results | ST-04, `ModerationTest.StudentCannotSeeUnreleasedMark` | |
| A2 | HoD: 10%/day penalty up to 3 days, then zero | submit Lab 1 late, mark it, view in moderation list | ST-04, `MarkingTest.LatePenaltyWorkedExample` | |
| A3 | HoD: see every mark before release | hod.cs option 1 | `ModerationTest.PendingListShowsDepartmentMarksOnly` | |
| A4 | Lecturer: grant individual extensions | lec.molefi option 7 for stu2, stu2 option 1 | `SubmissionTest.ExtensionShownInStudentAssignmentList` | |
| A5 | Student rep: resubmit before deadline, written feedback, late not rejected | stu1 resubmits Lab 2; late Lab 1 accepted; feedback visible after release | ST-01, ST-02, ST-04 | |
| A6 | IT officer: offline, protected passwords, fast | disconnect network and repeat A1; inspect `users.password_hash`; run `mlms_perf` | `AuthTest.PasswordsAreStoredAsSaltedArgon2idHashes`, ST-11 | |
| A7 | Principal: simple to use | each role completes its main task using only the on-screen menu | - | |
