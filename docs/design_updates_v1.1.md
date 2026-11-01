# Design document updates for v1.1 (CR1)

Paste these into the PlantUML sources from the Week 3 design to update the figures.

## Use case diagram - add
```plantuml
usecase "Grant class extension" as UC_ClassExt
Lecturer --> UC_ClassExt
UC_ClassExt ..> (Authenticate) : <<include>>
UC_ClassExt ..> (Record mark-change audit trail) : <<include>>
```

## Class diagram - add / change
```plantuml
class ClassExtension {
  - id : int
  - assignmentId : int
  - revisedDeadline : string
  - reason : string
  - grantedBy : int
  - grantedAt : string
}
Assignment "1" -- "0..1" ClassExtension : extended by >

class DeadlineInfo {
  + original : string
  + effective : string
  + individualExtension : bool
  + classExtension : bool
  + extensionReason : string
  + isExtended() : bool
}

interface IExtensionRepository {
  + findById(id : int) : optional<Extension>
  + find(assignmentId : int, studentId : int) : optional<Extension>
  + save(e : Extension) : void
  + findClassExtension(assignmentId : int) : optional<ClassExtension>
  + saveClassExtension(e : ClassExtension) : void
}

class ExtensionService {
  + grantIndividual(lecturer : User, assignmentId : int, studentId : int, revisedDeadline : string, reason : string) : string
  + grantClass(lecturer : User, assignmentId : int, revisedDeadline : string, reason : string) : string
}

class DeadlineResolver {
  + resolve(a : Assignment, studentId : int) : DeadlineInfo
  + lateDays(a : Assignment, s : Submission) : int
}
DeadlineResolver ..> IExtensionRepository
ExtensionService ..> IExtensionRepository
```

## ERD - add
```plantuml
entity class_extensions {
  * class_extension_id : INTEGER <<PK>>
  --
  * assignment_id : INTEGER <<FK, UQ>>
  * revised_deadline : TEXT
  * reason : TEXT
  * granted_by : INTEGER <<FK>>
  * granted_at : TEXT
}
assignments ||--o| class_extensions : "extended by"
users ||--o{ class_extensions : "granted_by"
```

## SQL schema - version 2
```sql
CREATE TABLE class_extensions (
    class_extension_id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL UNIQUE REFERENCES assignments(assignment_id),
    revised_deadline TEXT NOT NULL,
    reason TEXT NOT NULL CHECK (length(trim(reason)) > 0),
    granted_by INTEGER NOT NULL REFERENCES users(user_id),
    granted_at TEXT NOT NULL
);
PRAGMA user_version = 2;
```
Still in 3NF: every non-key column depends only on the assignment (one class extension per assignment).

## Data dictionary - add
| Table.column | Type / constraint | Meaning |
|---|---|---|
| class_extensions.class_extension_id | INTEGER PRIMARY KEY | identifier |
| class_extensions.assignment_id | INTEGER NOT NULL UNIQUE FK | assignment whose deadline is extended for everyone |
| class_extensions.revised_deadline | TEXT NOT NULL | new deadline for the class |
| class_extensions.reason | TEXT NOT NULL, non-empty CHECK | mandatory reason |
| class_extensions.granted_by / granted_at | FK / TEXT NOT NULL | lecturer and time |

## Effective-deadline rule (replaces design section 2.1, first bullet)
Effective deadline = the latest of (original deadline, class extension, the student's individual extension).

## Sequence "Submit assignment" - change
```plantuml
SubmissionService -> DeadlineResolver : resolve(assignment, studentId)
DeadlineResolver -> IExtensionRepository : findClassExtension(assignmentId)
DeadlineResolver -> IExtensionRepository : find(assignmentId, studentId)
DeadlineResolver --> SubmissionService : DeadlineInfo (latest deadline)
```

## Class-to-table mapping - add
| Domain class | Table | Rationale |
|---|---|---|
| ClassExtension | class_extensions | one row per assignment; UNIQUE(assignment_id) enforces "one class extension" |
