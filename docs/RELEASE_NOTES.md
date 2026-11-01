# Release Notes

## v1.1 - change request CR1
**New**
- Lecturers can extend an assignment's deadline for the **whole class** (lecturer menu option 8), with a
  mandatory reason. It can be changed until marking starts; every grant or change is in the audit log.
- Students' assignment lists now show when a deadline has been extended ("extended for you" or
  "extended for the whole class") together with the original deadline. Lecturer and HoD lists show it too.
- If a student has both, the later deadline applies - a class extension never shortens a personal one.

**Changed**
- Lecturer menu: "Change my password" moved from option 8 to option 9.

**Upgrade from v1.0**: back up `mlms.db`, install v1.1, start it with the same database file. The
database is upgraded automatically (schema version 1 -> 2); no data is lost.

**Quality**: 128 automated tests passing (116 regression + 12 new), 95.9% business-logic line coverage,
zero cppcheck and clang-tidy findings.


## v1.0 - first release
**What it does**
- Login for Administrators, Lecturers, Heads of Department and Students, each with their own menu.
- Administrators manage user accounts, departments, courses, lecturer allocations and enrolments.
- Lecturers create assignments, grant individual extensions (with a reason), mark submissions after the
  deadline and give written feedback.
- Students view their assignments and deadlines, submit, resubmit before the deadline, and view released
  marks with feedback.
- HoDs approve, amend (with a reason) or return marks for their department and release them per assignment.
- Late work: 10% of the maximum mark per started day late for up to 3 days; after that the final mark is 0.
- Every mark entry, correction, moderation decision and release is written to an append-only audit log.

**Security**: Argon2id password hashing (libsodium), lockout after 5 failed attempts for 15 minutes,
parameterised SQL, role and scope checks in every service.

**Quality**: 116 automated tests passing, 95.5% line coverage of the business-logic layer, zero
cppcheck and clang-tidy findings, login in about 74 ms with 800 students stored.

**Known limitations**
- Single computer only; do not place the database on a network share.
- Passwords are visible while typed (plain console input).
- FR-20 (notify students of newly released marks at login) is not included (Could requirement).

**Upgrade**: new installation - follow `docs/INSTALLATION_GUIDE.md`.
