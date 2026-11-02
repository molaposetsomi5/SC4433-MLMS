# Risk Register (updated at each gate)

Scores: likelihood (L) and impact (I) from 1 to 5; exposure = L x I. Low 1-6, Medium 7-12, High 13-25.
G0 = initial plan (Week 1), G1 = after requirements (Gate 1), G2 = after design (Gate 2), G3 = after testing and release v1.0 (Gate 3).
All owners are the student (single-person project), wearing the role shown.

| ID | Risk | Type | G0 | G1 | G2 | G3 | Status at G3 | What changed / evidence |
|---|---|---|---|---|---|---|---|---|
| R1 | Stakeholder conflicts left unresolved, inconsistent SRS | Project | 16 | 6 | 4 | 2 | Closed | 15 conflicts (C1-C15) resolved and recorded in the Phase 2 SRS |
| R2 | Schedule overrun because documents take longer than planned | Project | 16 | 16 | 12 | 9 | Open | Implementation finished late in Week 5; slippage rule used (FR-20 Could item dropped) |
| R3 | Scope creep beyond the brief | Project | 12 | 8 | 6 | 3 | Closed | Out-of-scope list kept; only FR-20 (Could) not built |
| R4 | Gaps in C++ / SQLite API knowledge cause defects | People | 12 | 12 | 8 | 4 | Reduced | SQLite wrapped in RAII `Database`/`Statement` classes; no `new`/`delete` in the code |
| R5 | Security flaw (SQL injection, weak passwords) | Technical | 15 | 15 | 10 | 5 | Reduced | Parameterised queries only, Argon2id via libsodium, injection test `QueriesAreParameterisedAgainstInjection` |
| R6 | Illness or personal circumstances reduce time | People | 12 | 12 | 12 | 12 | Open | No change - lighter Weeks 6-7 kept as slack |
| R7 | Late-penalty, extension or moderation rules implemented wrongly | Technical | 12 | 8 | 6 | 3 | Reduced | Rules made exact in SRS (C3-C6); 19 boundary-value tests pass |
| R8 | Design, code, database and tests drift apart | Project | 12 | 9 | 6 | 4 | Reduced | Traceability matrix kept up to date (docs/TRACEABILITY.md) |
| R9 | Gate 2 fails (design incomplete, ERD not 3NF) | Project | 15 | 15 | 3 | - | Closed | Design passed Gate 2; ERD in 3NF |
| R10 | System too slow with 800 students | Technical | 6 | 6 | 4 | 2 | Closed | Measured: login 80 ms, assignment list 0.5 ms (target 2 s) |
| R11 | Build or CI problems | Technical | 9 | 9 | 9 | 6 | Reduced | CI set up early; it caught defect D-10 after v1.1, fixed and re-run green |
| R12 | Business-logic coverage below 70% | Technical | 9 | 9 | 6 | 1 | Closed | Measured 95.9% line coverage (gcovr) |
| R13 | Loss of work or computer failure | Technical | 10 | 10 | 10 | 4 | Reduced | Code on GitHub, copy on USB drive |
| R14 | SQLite file damaged or locked over a network share | Technical | 12 | 12 | 8 | 6 | Reduced | v1.x limited to one computer; busy timeout added; stated in install guide |
| R15 | **New at G3:** unable to explain own code at the viva | People | - | - | - | 15 | Open | Mitigation: practise the demo script, walk through each design pattern in the code before the viva |
| R16 | **New at G3:** change request after release breaks existing features | Technical | - | - | - | 9 | Reduced | Whole suite rerun for CR1: 128/128 pass (116 old + 12 new) |
