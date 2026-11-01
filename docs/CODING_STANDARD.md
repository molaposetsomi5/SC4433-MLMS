# MLMS Coding Standard

Based on the C++ Core Guidelines, kept short so it can actually be followed.

## Language and build
- C++17, built with CMake. Code must compile with **no warnings** under `-Wall -Wextra -Wpedantic -Wshadow -Wconversion`.
- All code is inside `namespace mlms`.

## Naming
| Item | Style | Example |
|---|---|---|
| Classes, structs, enums | PascalCase | `SubmissionService`, `MarkStatus` |
| Interfaces | `I` + PascalCase | `IMarkRepository` |
| Functions, variables | camelCase | `releaseAssignment`, `lateDays` |
| Private members | trailing `_` | `marks_` |
| Constants | `k` + PascalCase | `kMaxLateDays` |
| Files | match the main class | `MarkingService.h/.cpp` |

## Layering rules (from the design document, section 3.4)
1. `ui/` may call `services/` only - never repositories or SQL.
2. `services/` depend on repository **interfaces** (`data/Repositories.h`), not on SQLite classes.
3. `domain/` has no dependency on SQLite or the console.
4. Only `data/` includes `<sqlite3.h>`.
5. Objects are wired together in one place: `app/AppContext.cpp` (constructor injection).

## Safety rules
- **Every SQL statement uses bound parameters** (`Statement::bind`). Never build SQL by joining user input.
- Raw SQLite handles are only held by the RAII classes `Database` and `Statement`.
- Multi-step changes (e.g. mark + audit record) run inside `TransactionManager::run`.
- Every service method starts with a permission check (`PermissionPolicy`).
- Errors are reported with exceptions derived from `MlmsError`; the console catches them and returns to the menu.
- Passwords are only ever handled by `IPasswordHasher`; they are never logged or stored in plain text.

## Style
- 4-space indentation, 120-column limit, braces on the same line.
- Prefer `const` references for parameters that are not changed.
- Use `std::optional` for values that may be missing (not magic numbers like `-1`).
- One short comment at the top of each file says what it is for and which requirement(s) it covers.

## Checks run in CI
- cppcheck (warning, style, performance, portability) - must report 0 findings.
- clang-tidy with the checks in `.clang-tidy` - must report 0 findings.
- Tests must pass and business-logic line coverage must stay at or above 70%.
