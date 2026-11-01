#include "data/Schema.h"

namespace mlms {

namespace {

// Version 1: the schema from the design document (section 6.5),
// plus triggers that make audit_logs append-only.
const char* const kSchemaV1 = R"SQL(
CREATE TABLE departments (
    department_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE users (
    user_id INTEGER PRIMARY KEY,
    username TEXT NOT NULL UNIQUE,
    department_id INTEGER REFERENCES departments(department_id),
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL CHECK (role IN ('ADMINISTRATOR','LECTURER','HOD','STUDENT')),
    display_name TEXT NOT NULL,
    student_number TEXT UNIQUE,
    is_active INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),
    failed_login_count INTEGER NOT NULL DEFAULT 0 CHECK (failed_login_count >= 0),
    locked_until TEXT,
    created_at TEXT NOT NULL
);

CREATE TABLE courses (
    course_id INTEGER PRIMARY KEY,
    department_id INTEGER NOT NULL REFERENCES departments(department_id),
    code TEXT NOT NULL UNIQUE,
    name TEXT NOT NULL
);

CREATE TABLE lecturer_courses (
    lecturer_id INTEGER NOT NULL REFERENCES users(user_id),
    course_id INTEGER NOT NULL REFERENCES courses(course_id),
    PRIMARY KEY (lecturer_id, course_id)
);

CREATE TABLE enrolments (
    student_id INTEGER NOT NULL REFERENCES users(user_id),
    course_id INTEGER NOT NULL REFERENCES courses(course_id),
    enrolled_at TEXT NOT NULL,
    PRIMARY KEY (student_id, course_id)
);

CREATE TABLE assignments (
    assignment_id INTEGER PRIMARY KEY,
    course_id INTEGER NOT NULL REFERENCES courses(course_id),
    title TEXT NOT NULL,
    description TEXT NOT NULL DEFAULT '',
    deadline TEXT NOT NULL,
    max_mark REAL NOT NULL CHECK (max_mark > 0),
    created_by INTEGER NOT NULL REFERENCES users(user_id),
    created_at TEXT NOT NULL
);

CREATE TABLE submissions (
    submission_id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL REFERENCES assignments(assignment_id),
    student_id INTEGER NOT NULL REFERENCES users(user_id),
    status TEXT NOT NULL CHECK (status IN ('SUBMITTED','LATE')),
    created_at TEXT NOT NULL,
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE submission_versions (
    version_id INTEGER PRIMARY KEY,
    submission_id INTEGER NOT NULL REFERENCES submissions(submission_id),
    version_number INTEGER NOT NULL,
    submitted_at TEXT NOT NULL,
    content_reference TEXT NOT NULL,
    is_latest_valid INTEGER NOT NULL DEFAULT 1 CHECK (is_latest_valid IN (0,1)),
    UNIQUE (submission_id, version_number)
);

CREATE TABLE extensions (
    extension_id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL REFERENCES assignments(assignment_id),
    student_id INTEGER NOT NULL REFERENCES users(user_id),
    revised_deadline TEXT NOT NULL,
    reason TEXT NOT NULL CHECK (length(trim(reason)) > 0),
    granted_by INTEGER NOT NULL REFERENCES users(user_id),
    granted_at TEXT NOT NULL,
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE marks (
    mark_id INTEGER PRIMARY KEY,
    submission_id INTEGER NOT NULL UNIQUE REFERENCES submissions(submission_id),
    raw_mark REAL NOT NULL CHECK (raw_mark >= 0),
    penalty REAL NOT NULL DEFAULT 0 CHECK (penalty >= 0),
    final_mark REAL NOT NULL CHECK (final_mark >= 0),
    feedback TEXT NOT NULL DEFAULT '',
    status TEXT NOT NULL CHECK (status IN ('AWAITING_MODERATION','RETURNED','APPROVED','RELEASED')),
    marked_by INTEGER NOT NULL REFERENCES users(user_id),
    marked_at TEXT NOT NULL
);

CREATE TABLE moderation_decisions (
    decision_id INTEGER PRIMARY KEY,
    mark_id INTEGER NOT NULL REFERENCES marks(mark_id),
    hod_user_id INTEGER NOT NULL REFERENCES users(user_id),
    decision TEXT NOT NULL CHECK (decision IN ('APPROVE','AMEND','RETURN')),
    previous_mark REAL,
    new_mark REAL,
    reason TEXT NOT NULL,
    decided_at TEXT NOT NULL
);

CREATE TABLE audit_logs (
    audit_id INTEGER PRIMARY KEY,
    actor_user_id INTEGER NOT NULL REFERENCES users(user_id),
    event_type TEXT NOT NULL,
    entity_type TEXT NOT NULL,
    entity_id INTEGER NOT NULL,
    timestamp TEXT NOT NULL,
    old_value TEXT,
    new_value TEXT,
    reason TEXT NOT NULL
);

CREATE TRIGGER audit_logs_no_update BEFORE UPDATE ON audit_logs
BEGIN SELECT RAISE(ABORT, 'audit_logs is append-only'); END;

CREATE TRIGGER audit_logs_no_delete BEFORE DELETE ON audit_logs
BEGIN SELECT RAISE(ABORT, 'audit_logs is append-only'); END;

CREATE INDEX idx_assignments_course ON assignments(course_id);
CREATE INDEX idx_submissions_student ON submissions(student_id);
CREATE INDEX idx_submissions_assignment ON submissions(assignment_id);
CREATE INDEX idx_versions_submission ON submission_versions(submission_id);
CREATE INDEX idx_marks_status ON marks(status);
CREATE INDEX idx_enrolments_course ON enrolments(course_id);
CREATE INDEX idx_audit_timestamp ON audit_logs(timestamp);
CREATE INDEX idx_audit_entity ON audit_logs(entity_type, entity_id);
)SQL";

}  // namespace

int schemaVersion(Database& db) {
    Statement stmt = db.prepare("PRAGMA user_version;");
    return stmt.step() ? stmt.getInt(0) : 0;
}

void migrateSchema(Database& db) {
    TransactionManager tx(db);
    tx.run([&] {
        const int version = schemaVersion(db);
        if (version < 1) {
            db.exec(kSchemaV1);
            db.exec("PRAGMA user_version = 1;");
        }
    });
}

}  // namespace mlms
