# Self Code Review (v1.0)

Checklist applied to every file before tagging v1.0. "Changed" lists what the review caused.

| # | Checklist item | Result | Changed |
|---|---|---|---|
| 1 | Does the code follow the layering rules (UI -> services -> repository interfaces -> SQLite)? | Yes. `grep sqlite3 src/ui src/services src/domain` finds nothing. | - |
| 2 | Is every SQL statement parameterised? | Yes, all go through `Statement::bind`. Only fixed DDL uses `exec`. | - |
| 3 | Does every service method check role and resource scope first? | Yes | - |
| 4 | Are multi-step writes inside one transaction (mark + audit, decision + audit, release batch)? | Yes | Rollback no longer hides errors (D-01) |
| 5 | Can a student see an unreleased mark by any route? | No - `resultsFor` filters on RELEASED and own submissions; no other student-facing query returns marks | - |
| 6 | Are all inputs validated (blank, range, date format, references)? | Yes | Date parser split for readability (D-06) |
| 7 | Are numeric conversions safe? | Not fully | Widening fixed in DateTime (D-02) |
| 8 | Are optionals checked before use? | Mostly | Local copies used (D-03) |
| 9 | Const-correctness of parameters | No in the UI | 20 handlers changed to `const User&` (D-04) |
| 10 | Is every rule from the SRS conflict table (C1-C15) implemented? | Yes: C1 lecturer sees own marks, C2 mark after deadline, C3 started-day rule, C4 >3 days = 0 but accepted, C5 extension moves the penalty clock, C6 unlimited resubmission + history, C7 approve/amend/return, C8 every entry/change audited, C10 single computer, C11 measured, C12 Argon2id + lockout, C13 admin cannot see marks, C14 feedback released with mark, C15 closed after release | - |
| 11 | Does every test assert something meaningful? | No - one stub test | Rewritten (D-07) |
| 12 | Are names and comments clear; does each file say what it is for? | Yes | Test parameter names made readable (D-08) |
| 13 | Are there functions with cyclomatic complexity over 15? | 2 | `parseDateTime` refactored; `adminMenu` kept (a flat switch with one line per option - splitting it would make it harder to read) |
| 14 | Memory and resource safety | SQLite handles only in RAII classes; no `new`/`delete` anywhere; objects shared with `shared_ptr` / owned by `AppContext` | - |
| 15 | Do the class diagram and ERD still match the code? | Yes, see `docs/TRACEABILITY.md` | `DeadlineResolver` noted as an addition |
