# Configuration Management

## Branching strategy
- `main` - always builds and passes CI. Only changed through merges. Releases are tagged here.
- `feature/<short-name>` - one branch per piece of work (e.g. `feature/data-layer`,
  `feature/cr1-class-extension`). Created from `main`.
- Work is merged back with a pull request (`git merge --no-ff` locally), after CI passes and the
  self-review checklist (`docs/qa/CODE_REVIEW.md`) has been gone through.
- Bug fixes found after a release use `fix/<short-name>` the same way.

## Commit messages
`<area>: <what changed>` in the imperative, e.g. `services: reject release of mixed batch`.
Reference the requirement or change request when there is one (`FR-10`, `CR1`).

## Tags
| Tag | Meaning |
|---|---|
| `v1.0` | First release (Phase 7) |
| `v1.1` | Release containing change request CR1 (Phase 8) |

## Continuous integration
`.github/workflows/ci.yml` runs on every push, pull request and tag:
1. install dependencies, 2. configure and build with CMake, 3. run all GoogleTest tests,
4. run the 800-student performance check, 5. cppcheck (fails on any finding),
6. clang-tidy (fails on any finding), 7. build with coverage and fail if business-logic line
coverage is below 70%, 8. report complexity metrics, 9. upload the coverage report.

## Items under configuration control
Source (`src/`, `tests/`, `tools/`), build files (`CMakeLists.txt`), analyser settings (`.clang-tidy`),
CI workflow, and all documents in `docs/`. The database file and build folders are ignored (`.gitignore`).

## Schema versioning
The database stores its schema version in `PRAGMA user_version`. `migrateSchema()` upgrades older files
step by step inside a transaction, so a v1.0 database opens safely in v1.1.
