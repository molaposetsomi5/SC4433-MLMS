# Defect Log

Severity: Critical (data loss / security), Major (wrong result or rule broken), Minor (quality / maintainability), Trivial (cosmetic).

| ID | Found by | Severity | Description | Cause | Fix | Retest |
|---|---|---|---|---|---|---|
| D-01 | clang-tidy (bugprone-empty-catch) | Major | `TransactionManager::rollback` swallowed every database error with an empty `catch`, so a real rollback failure would be hidden | defensive try/catch added because SQLite sometimes rolls back by itself | added `Database::inTransaction()` (sqlite3_get_autocommit) and only issue `ROLLBACK` when a transaction is open; no catch | `AuditTest.FailedTransactionRollsBackEverything` Pass |
| D-02 | clang-tidy (bugprone-implicit-widening) | Major | `hour * 3600 + minute * 60` was calculated in `int` and then widened to 64-bit seconds - overflow risk in date maths | arithmetic done before conversion | cast to `Seconds` before multiplying (`DateTime.cpp`, parse and SystemClock) | DateTime and BVA tests Pass |
| D-03 | clang-tidy (bugprone-unchecked-optional-access) | Minor | `departmentId()` was checked and then dereferenced through a second call in 4 places | pattern `if (u.departmentId()) ... *u.departmentId()` | copy the optional into a local, check and use the local | ModerationTest, AdminTest Pass |
| D-04 | cppcheck (constParameterReference) | Minor | 20 console handler functions took `User&` although they do not change the user | copy of the menu signature | changed to `const User&`; only `changeMyPassword` and the menu loops keep `User&` | full suite Pass |
| D-05 | cppcheck (useStlAlgorithm) | Trivial | raw loop in `SubmissionService::isClosed` | style | replaced with `std::any_of` | SubmissionTest / ModerationTest.ReleasedMarkIsFinal Pass |
| D-06 | lizard (CCN 16 > 15) | Minor | `parseDateTime` too complex to read and test | pattern check and range checks in one function | split into `matchesPattern` and `fieldsInRange` (CCN now 4/7/8) | `BadInputs/InvalidDateTime.*` Pass |
| D-07 | self code review | Major | test `ModerationTest.AmendKeepsLatePenalty` had no assertions, so the "amend recalculates penalty" rule was not really tested | test stub left during writing | rewrote the test: 2-days-late submission, HoD amends 40->45, checks penalty 10 and final 35 | Pass |
| D-08 | self code review | Trivial | parameterised test names printed as hex bytes, so a failure would be hard to identify | no name generator | added `PrintTo` and name generators (`.../86401s`, `case2_2days`) | Pass |
| D-09 | coverage tool | Trivial | lcov 2.0 reported impossible values (117%, 0 functions) | lcov/gcov version mismatch on the build machine | switched the coverage report to gcovr (also used in CI) | report 95.5% consistent with gcov |
| D-10 | CI pipeline (cppcheck step) after pushing v1.1 | Minor | cppcheck `useStlAlgorithm` reported again in `AssignmentService::listVisibleTo`, so the v1.1 CI run failed | the CR1 change rewrote the loop and dropped the inline suppression that v1.0 had | restored the `cppcheck-suppress useStlAlgorithm` comment (with its reason) above the reported line | cppcheck 0 findings, 128/128 tests, CI green |

No Critical defects were found. All defects are closed.
