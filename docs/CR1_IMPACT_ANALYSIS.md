# Change Request CR1 - Class-wide extensions (release v1.1)

> "A lecturer should be able to extend the deadline for a whole class, not just an individual student,
> and extensions should be shown on the student's assignment list."

## 1. Classification
**Perfective maintenance.** The system works correctly and its environment (hardware, OS, libraries) has
not changed, so it is not corrective or adaptive. The users are asking for a new capability that improves
how well the system meets their needs - that is perfective maintenance. It also has a small *preventive*
side: the effective-deadline rule now lives only in `DeadlineResolver`, so later deadline changes stay in
one class.

## 2. Decisions taken for the change
| Question | Decision | Reason |
|---|---|---|
| Store it as many individual extensions or as one record? | One `class_extensions` row per assignment | one fact = one row (3NF); students enrolled later also get it automatically |
| Class extension vs individual extension - which wins? | The **later** of original, class and individual deadline | a class extension must never cut short a student's personal extension |
| Can it be changed? | Yes, until any submission for the assignment is marked; every grant/change is audited | same rule as individual extensions (SRS C2/C5) |
| Who can grant it? | A lecturer assigned to the course | same scope as FR-10 |

## 3. Impact analysis
### Requirements
| Item | Change |
|---|---|
| FR-10 Individual extension | unchanged, but its acceptance test now also covers the "later deadline wins" rule |
| **FR-21 (new) Class extension** | Must: "A Lecturer shall be able to extend the deadline of an assignment for every enrolled student, with a revised deadline and mandatory reason. The later of the original, class and individual deadlines is the student's effective deadline." |
| FR-07 Assignment viewing | extended: the list must show that a deadline was extended and the original deadline |
| FR-09 Late penalty | unchanged rule; the penalty clock now starts from the effective deadline that includes the class extension |
| FR-14 Audit | new events `CLASS_EXTENSION_GRANTED` / `CLASS_EXTENSION_CHANGED` |

### Diagrams (updates in `docs/design_updates_v1.1.md`)
| Diagram | Change |
|---|---|
| Use case | new use case "Grant class extension" (Lecturer), `<<include>>` Authenticate |
| Class diagram | new class `ClassExtension`; `DeadlineInfo` gains `classExtension`; `ExtensionService.grantClass()`; `IExtensionRepository` gains 2 operations; `CourseAssignment` gains `classExtension` |
| ERD | new entity `class_extensions` (1 assignment : 0..1 class extension) |
| State machine | no change (extension is not a submission state) |
| Activity diagram | "Calculate effective deadline" now also loads the class extension |
| Sequence "Submit assignment" | `DeadlineResolver` call also reads `findClassExtension` |
| Deployment | no change |

### Code
| File | Change |
|---|---|
| `domain/Types.h` | + `ClassExtension` |
| `data/Schema.cpp` | + schema version 2 (`class_extensions`), automatic upgrade of v1 databases |
| `data/Repositories.h`, `SqliteRepositories.h`, `SqliteAssessmentRepositories.cpp` | + `findClassExtension`, `saveClassExtension` |
| `services/DeadlineResolver.*` | effective deadline = latest of original / class / individual |
| `services/ExtensionService.*` | + `grantClass` |
| `services/AuditService.h` | + 2 event names |
| `services/AssignmentService.*`, `app/AppContext.cpp` | inject `IExtensionRepository`; lists carry the class extension |
| `ui/ConsoleUI.*` | lecturer option 8 "Extend the deadline for the whole class"; change password moved to 9; lists show "extended for the whole class (was ...)"; HoD moderation list says class/individual |
| `CMakeLists.txt` | version 1.1.0, new test file |

### Tables
`class_extensions` added. No existing table changed, so no data migration is needed beyond creating the table.

### Test cases
| Existing tests affected | Why |
|---|---|
| All `SubmissionTest`, `MarkingTest`, `ModerationTest` that use deadlines | `DeadlineResolver` changed - rerun as regression |
| `SubmissionTest.ExtensionShownInStudentAssignmentList` | uses `DeadlineInfo` - still passes unchanged |
| System tests ST-01..ST-10 | lecturer menu numbering changed (no existing test used option 8) |

New tests (`tests/test_class_extension.cpp`, 12 tests): moves deadline for all students; shown on the
student list (FR-07); shown on lecturer/HoD lists; later individual extension wins; class extension never
shortens an individual one; penalty clock from class deadline; marking waits for class deadline;
input/permission rules; refused after marking; change + audit + HoD audit scope; console end-to-end
(ST-12); v1 database upgraded to v2.

## 4. Regression testing results
| Check | v1.0 | v1.1 |
|---|---|---|
| Automated tests | 116 / 116 pass | 128 / 128 pass (all 116 old tests unchanged and passing) |
| Business-logic line coverage | 95.5% | 95.9% |
| cppcheck / clang-tidy findings | 0 / 0 | 0 / 0 |
| NFR-01 (800 students) | login 74 ms | login 80 ms, list 0.5 ms |
| Upgrade of a real v1.0 database file | - | database created by the v1.0 binary opened with v1.1: schema 1 -> 2, 11 users, 1 submission and its audit row kept, class extension granted and shown to the student |

Full output: `docs/test_results_v1.1.txt`.
