# Quality Assurance Report

v1.1 re-check after CR1: cppcheck 0 findings, clang-tidy 0 findings, 128/128 tests, business-logic line
coverage 95.9% (902/941), 287 functions, average CCN 2.5. `grantClass` CCN 3, `DeadlineResolver::resolve` CCN 5.
The detailed figures below are from v1.0; `coverage_business_logic.txt`, `lizard_full.txt` and `coverage/`
have been regenerated for v1.1.

# v1.0 details

## Static analysis
| Tool | Settings | Findings before review | After |
|---|---|---|---|
| cppcheck 2.13 | `--enable=warning,style,performance,portability --std=c++17` | 23 (see `cppcheck_before.txt`) | 0 |
| clang-tidy 18 | checks in `.clang-tidy` (bugprone-*, performance-*, selected modernize/readability) | 13 (see `clang_tidy_before.txt`) | 0 |

Two findings were accepted with an inline suppression and a reason:
- `constParameter` on `main(int argc, char* argv[])` - this is the standard signature of `main`.
- `useStlAlgorithm` in `AssignmentService::listVisibleTo` - a nested loop is clearer than `std::transform` here.
`performance-enum-size` (use `uint8_t` for enums) was switched off: saving 3 bytes per enum is not worth
the readability cost. All other fixes are listed in `docs/DEFECT_LOG.md` (D-01 to D-06).

## Code metrics (lizard, after review)
| Metric | Value |
|---|---|
| Lines of code (non-blank, non-comment) | src ~3,700 (domain 504, data 1,033, security 52, services 1,291, app 177, ui 597, main 40); tests ~950 |
| Functions | 282 |
| Average cyclomatic complexity (CCN) | 2.5 |
| Average function length | 8 lines |

Key business functions:
| Function | CCN | Comment |
|---|---|---|
| `StandardLatePenaltyPolicy::apply` | 3 | |
| `StandardLatePenaltyPolicy::lateDays` | 2 | |
| `SubmissionService::submit` | 11 | follows UC-01 steps and flows A1/A3/E1/E2 one by one |
| `AuthService::login` | 9 | inactive, locked, wrong password, lockout, success |
| `AuditService::query` | 14 | role scope + 3 filters |
| `ReleaseService::releaseAssignment` | 6 | |
| `parseDateTime` | 4 (was 16) | split into `matchesPattern` (7) and `fieldsInRange` (8) |
| `ConsoleUI::adminMenu` | 16 | menu switch, accepted (see code review item 13) |

## Coverage (gcovr, business-logic layer = src/domain + src/services)
| | Covered | Total | % |
|---|---|---|---|
| Lines | 863 | 904 | **95.5%** |
| Functions | 126 | 129 | 97.7% |
| Branches | 894 | 1,498 | 59.7% |

Target was at least 70% line coverage. Branch coverage is lower mainly because of compiler-generated
exception branches. Per-file figures: `coverage_business_logic.txt`; browsable report: `coverage/index.html`.
