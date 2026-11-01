PRAGMA foreign_keys = ON;

BEGIN IMMEDIATE;

-- ============================================
-- 1. DEPARTMENTS
-- ============================================

CREATE TABLE IF NOT EXISTS departments (
    department_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE
);


-- ============================================
-- 2. USERS
-- Supports Administrator, Lecturer, HoD and Student
-- ============================================

CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY,
    username TEXT NOT NULL UNIQUE,
    department_id INTEGER,
    password_hash TEXT NOT NULL,

    role TEXT NOT NULL CHECK (
        role IN ('ADMINISTRATOR', 'LECTURER', 'HOD', 'STUDENT')
    ),

    display_name TEXT NOT NULL,
    student_number TEXT UNIQUE,

    is_active INTEGER NOT NULL DEFAULT 1
        CHECK (is_active IN (0, 1)),

    failed_login_count INTEGER NOT NULL DEFAULT 0
        CHECK (failed_login_count >= 0),

    locked_until TEXT,
    created_at TEXT NOT NULL,

    FOREIGN KEY (department_id)
        REFERENCES departments(department_id)
);


-- ============================================
-- 3. COURSES
-- ============================================

CREATE TABLE IF NOT EXISTS courses (
    course_id INTEGER PRIMARY KEY,
    department_id INTEGER NOT NULL,
    code TEXT NOT NULL UNIQUE,
    name TEXT NOT NULL,

    FOREIGN KEY (department_id)
        REFERENCES departments(department_id)
);


-- ============================================
-- 4. LECTURER COURSE ALLOCATIONS
-- ============================================

CREATE TABLE IF NOT EXISTS lecturer_courses (
    lecturer_id INTEGER NOT NULL,
    course_id INTEGER NOT NULL,

    PRIMARY KEY (lecturer_id, course_id),

    FOREIGN KEY (lecturer_id)
        REFERENCES users(user_id),

    FOREIGN KEY (course_id)
        REFERENCES courses(course_id)
);


-- ============================================
-- 5. STUDENT ENROLMENTS
-- ============================================

CREATE TABLE IF NOT EXISTS enrolments (
    student_id INTEGER NOT NULL,
    course_id INTEGER NOT NULL,
    enrolled_at TEXT NOT NULL,

    PRIMARY KEY (student_id, course_id),

    FOREIGN KEY (student_id)
        REFERENCES users(user_id),

    FOREIGN KEY (course_id)
        REFERENCES courses(course_id)
);


-- ============================================
-- 6. ASSIGNMENTS
-- ============================================

CREATE TABLE IF NOT EXISTS assignments (
    assignment_id INTEGER PRIMARY KEY,
    course_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    description TEXT NOT NULL DEFAULT '',
    deadline TEXT NOT NULL,

    max_mark REAL NOT NULL
        CHECK (max_mark > 0),

    created_by INTEGER NOT NULL,
    created_at TEXT NOT NULL,

    FOREIGN KEY (course_id)
        REFERENCES courses(course_id),

    FOREIGN KEY (created_by)
        REFERENCES users(user_id)
);


-- ============================================
-- 7. SUBMISSIONS
-- One logical submission per student per assignment
-- ============================================

CREATE TABLE IF NOT EXISTS submissions (
    submission_id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL,
    student_id INTEGER NOT NULL,

    status TEXT NOT NULL CHECK (
        status IN ('SUBMITTED', 'LATE')
    ),

    created_at TEXT NOT NULL,

    UNIQUE (assignment_id, student_id),

    FOREIGN KEY (assignment_id)
        REFERENCES assignments(assignment_id),

    FOREIGN KEY (student_id)
        REFERENCES users(user_id)
);


-- ============================================
-- 8. SUBMISSION VERSIONS
-- Preserves previous submitted versions
-- ============================================

CREATE TABLE IF NOT EXISTS submission_versions (
    version_id INTEGER PRIMARY KEY,
    submission_id INTEGER NOT NULL,
    version_number INTEGER NOT NULL,
    submitted_at TEXT NOT NULL,
    content_reference TEXT NOT NULL,

    is_latest_valid INTEGER NOT NULL DEFAULT 1
        CHECK (is_latest_valid IN (0, 1)),

    UNIQUE (submission_id, version_number),

    FOREIGN KEY (submission_id)
        REFERENCES submissions(submission_id)
);


-- ============================================
-- 9. EXTENSIONS
-- One extension per student per assignment
-- ============================================

CREATE TABLE IF NOT EXISTS extensions (
    extension_id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL,
    student_id INTEGER NOT NULL,
    revised_deadline TEXT NOT NULL,

    reason TEXT NOT NULL
        CHECK (length(trim(reason)) > 0),

    granted_by INTEGER NOT NULL,
    granted_at TEXT NOT NULL,

    UNIQUE (assignment_id, student_id),

    FOREIGN KEY (assignment_id)
        REFERENCES assignments(assignment_id),

    FOREIGN KEY (student_id)
        REFERENCES users(user_id),

    FOREIGN KEY (granted_by)
        REFERENCES users(user_id)
);


-- ============================================
-- 10. MARKS
-- Marks remain hidden until approved and released
-- ============================================

CREATE TABLE IF NOT EXISTS marks (
    mark_id INTEGER PRIMARY KEY,
    submission_id INTEGER NOT NULL UNIQUE,

    raw_mark REAL NOT NULL
        CHECK (raw_mark >= 0),

    penalty REAL NOT NULL DEFAULT 0
        CHECK (penalty >= 0),

    final_mark REAL NOT NULL
        CHECK (final_mark >= 0),

    feedback TEXT NOT NULL DEFAULT '',

    status TEXT NOT NULL CHECK (
        status IN (
            'AWAITING_MODERATION',
            'RETURNED',
            'APPROVED',
            'RELEASED'
        )
    ),

    marked_by INTEGER NOT NULL,
    marked_at TEXT NOT NULL,

    FOREIGN KEY (submission_id)
        REFERENCES submissions(submission_id),

    FOREIGN KEY (marked_by)
        REFERENCES users(user_id)
);


-- ============================================
-- 11. MODERATION DECISIONS
-- Records HoD approval, amendment or return
-- ============================================

CREATE TABLE IF NOT EXISTS moderation_decisions (
    decision_id INTEGER PRIMARY KEY,
    mark_id INTEGER NOT NULL,
    hod_user_id INTEGER NOT NULL,

    decision TEXT NOT NULL CHECK (
        decision IN ('APPROVE', 'AMEND', 'RETURN')
    ),

    previous_mark REAL,
    new_mark REAL,
    reason TEXT NOT NULL,
    decided_at TEXT NOT NULL,

    FOREIGN KEY (mark_id)
        REFERENCES marks(mark_id),

    FOREIGN KEY (hod_user_id)
        REFERENCES users(user_id)
);


-- ============================================
-- 12. AUDIT LOGS
-- Records important system activities
-- ============================================

CREATE TABLE IF NOT EXISTS audit_logs (
    audit_id INTEGER PRIMARY KEY,
    actor_user_id INTEGER NOT NULL,
    event_type TEXT NOT NULL,
    entity_type TEXT NOT NULL,
    entity_id INTEGER NOT NULL,
    timestamp TEXT NOT NULL,
    old_value TEXT,
    new_value TEXT,
    reason TEXT NOT NULL,

    FOREIGN KEY (actor_user_id)
        REFERENCES users(user_id)
);


-- ============================================
-- INDEXES
-- Improve common database searches
-- ============================================

CREATE INDEX IF NOT EXISTS idx_assignments_course
ON assignments(course_id);

CREATE INDEX IF NOT EXISTS idx_submissions_student
ON submissions(student_id);

CREATE INDEX IF NOT EXISTS idx_submissions_assignment
ON submissions(assignment_id);

CREATE INDEX IF NOT EXISTS idx_marks_status
ON marks(status);

CREATE INDEX IF NOT EXISTS idx_audit_timestamp
ON audit_logs(timestamp);

CREATE INDEX IF NOT EXISTS idx_audit_entity
ON audit_logs(entity_type, entity_id);

CREATE UNIQUE INDEX IF NOT EXISTS idx_one_latest_valid_version
ON submission_versions(submission_id)
WHERE is_latest_valid = 1;


-- ============================================
-- DATABASE SCHEMA VERSION
-- ============================================

PRAGMA user_version = 1;

COMMIT;