# Moderated Learning Management System (MLMS)

CS4433 Software Engineering - individual project. Molapo Setsumi, 202322042.

A menu-driven C++17 console application for a secondary school. Administrators, Lecturers, Heads of
Department and Students each log in and see only the actions their role allows. Students' marks stay
hidden until the HoD has moderated and released them, late work is penalised automatically, and every
mark change is recorded in an append-only audit log. Data is stored locally in SQLite, so the system
works completely offline.

## Build guide

### Requirements
| Tool | Version used |
|---|---|
| C++17 compiler | GCC 13 (MinGW-w64 GCC on Windows) |
| CMake | 3.16 or newer |
| SQLite 3 (dev package) | 3.45 |
| libsodium (dev package) | 1.0.18 |
| GoogleTest | 1.14 (downloaded automatically if not installed) |
| pkg-config | any |

### Linux (Ubuntu / Debian)
```bash
sudo apt install build-essential cmake pkg-config libsqlite3-dev libsodium-dev libgtest-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build          # run all 116 tests
./build/mlms --seed-demo        # first run with sample data
```

### Windows (MSYS2 UCRT64 terminal)
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
          mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-libsodium \
          mingw-w64-ucrt-x86_64-gtest
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
./build/mlms.exe --seed-demo
```

### Running
```
mlms [--db FILE] [--seed-demo]
```
- `--db FILE` chooses the database file (default `mlms.db` in the current folder).
- `--seed-demo` loads sample data into an **empty** database (handy for the viva).
- On an empty database without `--seed-demo`, the program asks you to create the administrator account.

Demo accounts (password for all: `Password123`):

| Username | Role | Notes |
|---|---|---|
| `admin` | Administrator | |
| `hod.cs` | HoD | Mathematics and Computer Science |
| `hod.lang` | HoD | Languages |
| `lec.molefi` | Lecturer | CS101, MA101 |
| `lec.sello` | Lecturer | EN101 |
| `stu1` ... `stu6` | Student | `stu1`-`stu3` also take MA101 |

"Lab 1" in CS101 already has a past deadline, so it can be submitted late and marked straight away.

### Other targets
| Command | What it does |
|---|---|
| `./build/mlms_perf` | NFR-01 check: 800 students, times login and assignment list |
| `cmake -S . -B build-cov -DMLMS_COVERAGE=ON` then `ctest --test-dir build-cov` and `gcovr -r . build-cov --filter src/domain/ --filter src/services/` | coverage of the business-logic layer |
| `cppcheck --enable=warning,style,performance,portability --std=c++17 --inline-suppr --suppress=missingIncludeSystem -I src src` | static analysis |

## Project structure (matches the layered architecture in the design document)
```
src/
  ui/          Presentation: Console (input helpers), ConsoleUI (role menus)
  services/    Application services: Auth, UserManagement, CourseManagement, Assignment,
               Extension, Submission, Marking, Moderation, Release, Audit, PermissionPolicy,
               DeadlineResolver
  domain/      Entities and rules: User hierarchy + UserFactory, LatePenaltyPolicy (Strategy),
               Types, DateTime, Errors, Validation
  data/        Repository interfaces + SQLite implementations, Database/Statement RAII
               wrappers, TransactionManager, Schema (migrations)
  security/    IPasswordHasher + SodiumPasswordHasher (Argon2id)
  app/         AppContext (composition root / dependency injection), DemoData
  main.cpp
tests/         GoogleTest unit, integration and scripted system tests
tools/         perf_test.cpp (NFR-01)
docs/          Coding standard, test plan, defect log, traceability, deployment, manuals, QA reports
```

## Design patterns used
| Pattern | Where | Why |
|---|---|---|
| Strategy | `LatePenaltyPolicy` / `StandardLatePenaltyPolicy` | penalty rules can change without touching the workflow |
| Repository | `I*Repository` + `Sqlite*Repository` | services never see SQL; easy to test |
| Dependency Injection | constructors, wired in `AppContext` | swap the clock, hasher or database in tests |
| Factory | `UserFactory::create` | one place turns a role into the right `User` subclass |

## Policy decisions taken for v1.0 (design review items, section 9)
- Late day = any started 24-hour period after the effective deadline. 10% of the maximum mark per day
  for days 1-3; more than 3 days late = final mark 0. The work is still accepted and recorded.
- Resubmission is allowed any number of times before the effective deadline while the work is unmarked.
  All versions are kept; the latest is marked.
- Marking is only possible after the student's effective deadline has passed.
- One extension per student per assignment. The lecturer can change it until the work is marked; the
  change is audited. The revised deadline must be later than the original.
- Lockout: 5 failed logins lock the account for 15 minutes. Passwords need at least 8 characters.
  An administrator can reset a password, which also unlocks the account.
- Release is per assignment. It is refused if any submission is unmarked or any mark is not yet approved
  (no partial release). Once released, the assignment is closed to new submissions.
- Times use the school computer's local clock, stored as `YYYY-MM-DD HH:MM:SS`.

## Security summary
- Passwords: libsodium `crypto_pwhash_str` (Argon2id, random salt per password, "interactive" limits).
- Every query is parameterised; foreign keys are switched on for every connection.
- Every service checks role **and** resource scope (my course / my department / my own records).
- `audit_logs` is protected by triggers that block UPDATE and DELETE.
