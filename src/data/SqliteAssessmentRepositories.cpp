// Assignments, submissions, extensions and marks.
#include "data/SqliteRepositories.h"

namespace mlms {

// ---------------------------------------------------------------- assignments

namespace {

const char* const kAssignmentColumns =
    "assignment_id, course_id, title, description, deadline, max_mark, created_by, created_at";

Assignment readAssignment(const Statement& s) {
    Assignment a;
    a.id = s.getInt(0);
    a.courseId = s.getInt(1);
    a.title = s.getText(2);
    a.description = s.getText(3);
    a.deadline = s.getText(4);
    a.maxMark = s.getDouble(5);
    a.createdBy = s.getInt(6);
    a.createdAt = s.getText(7);
    return a;
}

}  // namespace

int SqliteAssignmentRepository::insert(const Assignment& a) {
    Statement stmt = db_.prepare(
        "INSERT INTO assignments (course_id, title, description, deadline, max_mark, created_by, created_at) "
        "VALUES (?,?,?,?,?,?,?)");
    stmt.bind(1, a.courseId)
        .bind(2, a.title)
        .bind(3, a.description)
        .bind(4, a.deadline)
        .bind(5, a.maxMark)
        .bind(6, a.createdBy)
        .bind(7, a.createdAt);
    stmt.run();
    return static_cast<int>(db_.lastInsertId());
}

std::optional<Assignment> SqliteAssignmentRepository::findById(int assignmentId) {
    Statement stmt =
        db_.prepare(std::string("SELECT ") + kAssignmentColumns + " FROM assignments WHERE assignment_id = ?");
    stmt.bind(1, assignmentId);
    if (!stmt.step()) return std::nullopt;
    return readAssignment(stmt);
}

std::vector<Assignment> SqliteAssignmentRepository::listForCourse(int courseId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kAssignmentColumns +
                                 " FROM assignments WHERE course_id = ? ORDER BY deadline");
    stmt.bind(1, courseId);
    std::vector<Assignment> result;
    while (stmt.step()) result.push_back(readAssignment(stmt));
    return result;
}

// ---------------------------------------------------------------- submissions

namespace {

const char* const kSubmissionColumns = "submission_id, assignment_id, student_id, status, created_at";

Submission readSubmission(const Statement& s) {
    Submission sub;
    sub.id = s.getInt(0);
    sub.assignmentId = s.getInt(1);
    sub.studentId = s.getInt(2);
    sub.status = submissionStatusFromString(s.getText(3));
    sub.createdAt = s.getText(4);
    return sub;
}

SubmissionVersion readVersion(const Statement& s) {
    SubmissionVersion v;
    v.id = s.getInt(0);
    v.submissionId = s.getInt(1);
    v.versionNumber = s.getInt(2);
    v.submittedAt = s.getText(3);
    v.contentReference = s.getText(4);
    v.isLatestValid = s.getInt(5) == 1;
    return v;
}

const char* const kVersionColumns =
    "version_id, submission_id, version_number, submitted_at, content_reference, is_latest_valid";

}  // namespace

std::optional<Submission> SqliteSubmissionRepository::findById(int submissionId) {
    Statement stmt =
        db_.prepare(std::string("SELECT ") + kSubmissionColumns + " FROM submissions WHERE submission_id = ?");
    stmt.bind(1, submissionId);
    if (!stmt.step()) return std::nullopt;
    return readSubmission(stmt);
}

std::optional<Submission> SqliteSubmissionRepository::find(int assignmentId, int studentId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kSubmissionColumns +
                                 " FROM submissions WHERE assignment_id = ? AND student_id = ?");
    stmt.bind(1, assignmentId).bind(2, studentId);
    if (!stmt.step()) return std::nullopt;
    return readSubmission(stmt);
}

std::vector<Submission> SqliteSubmissionRepository::listForAssignment(int assignmentId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kSubmissionColumns +
                                 " FROM submissions WHERE assignment_id = ? ORDER BY submission_id");
    stmt.bind(1, assignmentId);
    std::vector<Submission> result;
    while (stmt.step()) result.push_back(readSubmission(stmt));
    return result;
}

int SqliteSubmissionRepository::create(const Submission& s) {
    Statement stmt = db_.prepare(
        "INSERT INTO submissions (assignment_id, student_id, status, created_at) VALUES (?,?,?,?)");
    stmt.bind(1, s.assignmentId).bind(2, s.studentId).bind(3, toString(s.status)).bind(4, s.createdAt);
    stmt.run();
    return static_cast<int>(db_.lastInsertId());
}

void SqliteSubmissionRepository::updateStatus(int submissionId, SubmissionStatus status) {
    Statement stmt = db_.prepare("UPDATE submissions SET status = ? WHERE submission_id = ?");
    stmt.bind(1, toString(status)).bind(2, submissionId);
    stmt.run();
}

int SqliteSubmissionRepository::addVersion(int submissionId, const std::string& submittedAt,
                                           const std::string& content) {
    int next = 1;
    {
        Statement maxStmt =
            db_.prepare("SELECT COALESCE(MAX(version_number), 0) FROM submission_versions WHERE submission_id = ?");
        maxStmt.bind(1, submissionId);
        if (maxStmt.step()) next = maxStmt.getInt(0) + 1;
    }
    {
        Statement clear = db_.prepare("UPDATE submission_versions SET is_latest_valid = 0 WHERE submission_id = ?");
        clear.bind(1, submissionId);
        clear.run();
    }
    Statement insert = db_.prepare(
        "INSERT INTO submission_versions (submission_id, version_number, submitted_at, content_reference, "
        "is_latest_valid) VALUES (?,?,?,?,1)");
    insert.bind(1, submissionId).bind(2, next).bind(3, submittedAt).bind(4, content);
    insert.run();
    return next;
}

std::optional<SubmissionVersion> SqliteSubmissionRepository::latestVersion(int submissionId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kVersionColumns +
                                 " FROM submission_versions WHERE submission_id = ? AND is_latest_valid = 1");
    stmt.bind(1, submissionId);
    if (!stmt.step()) return std::nullopt;
    return readVersion(stmt);
}

std::vector<SubmissionVersion> SqliteSubmissionRepository::versions(int submissionId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kVersionColumns +
                                 " FROM submission_versions WHERE submission_id = ? ORDER BY version_number");
    stmt.bind(1, submissionId);
    std::vector<SubmissionVersion> result;
    while (stmt.step()) result.push_back(readVersion(stmt));
    return result;
}

// ---------------------------------------------------------------- extensions

namespace {

const char* const kExtensionColumns =
    "extension_id, assignment_id, student_id, revised_deadline, reason, granted_by, granted_at";

Extension readExtension(const Statement& s) {
    Extension e;
    e.id = s.getInt(0);
    e.assignmentId = s.getInt(1);
    e.studentId = s.getInt(2);
    e.revisedDeadline = s.getText(3);
    e.reason = s.getText(4);
    e.grantedBy = s.getInt(5);
    e.grantedAt = s.getText(6);
    return e;
}

}  // namespace

std::optional<Extension> SqliteExtensionRepository::findById(int extensionId) {
    Statement stmt =
        db_.prepare(std::string("SELECT ") + kExtensionColumns + " FROM extensions WHERE extension_id = ?");
    stmt.bind(1, extensionId);
    if (!stmt.step()) return std::nullopt;
    return readExtension(stmt);
}

std::optional<Extension> SqliteExtensionRepository::find(int assignmentId, int studentId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kExtensionColumns +
                                 " FROM extensions WHERE assignment_id = ? AND student_id = ?");
    stmt.bind(1, assignmentId).bind(2, studentId);
    if (!stmt.step()) return std::nullopt;
    return readExtension(stmt);
}

void SqliteExtensionRepository::save(const Extension& e) {
    // one extension per student/assignment pair: a new grant replaces the old one
    Statement stmt = db_.prepare(
        "INSERT INTO extensions (assignment_id, student_id, revised_deadline, reason, granted_by, granted_at) "
        "VALUES (?,?,?,?,?,?) ON CONFLICT(assignment_id, student_id) DO UPDATE SET "
        "revised_deadline = excluded.revised_deadline, reason = excluded.reason, "
        "granted_by = excluded.granted_by, granted_at = excluded.granted_at");
    stmt.bind(1, e.assignmentId)
        .bind(2, e.studentId)
        .bind(3, e.revisedDeadline)
        .bind(4, e.reason)
        .bind(5, e.grantedBy)
        .bind(6, e.grantedAt);
    stmt.run();
}

std::optional<ClassExtension> SqliteExtensionRepository::findClassExtension(int assignmentId) {
    Statement stmt = db_.prepare(
        "SELECT class_extension_id, assignment_id, revised_deadline, reason, granted_by, granted_at "
        "FROM class_extensions WHERE assignment_id = ?");
    stmt.bind(1, assignmentId);
    if (!stmt.step()) return std::nullopt;
    ClassExtension e;
    e.id = stmt.getInt(0);
    e.assignmentId = stmt.getInt(1);
    e.revisedDeadline = stmt.getText(2);
    e.reason = stmt.getText(3);
    e.grantedBy = stmt.getInt(4);
    e.grantedAt = stmt.getText(5);
    return e;
}

void SqliteExtensionRepository::saveClassExtension(const ClassExtension& e) {
    Statement stmt = db_.prepare(
        "INSERT INTO class_extensions (assignment_id, revised_deadline, reason, granted_by, granted_at) "
        "VALUES (?,?,?,?,?) ON CONFLICT(assignment_id) DO UPDATE SET revised_deadline = excluded.revised_deadline, "
        "reason = excluded.reason, granted_by = excluded.granted_by, granted_at = excluded.granted_at");
    stmt.bind(1, e.assignmentId).bind(2, e.revisedDeadline).bind(3, e.reason).bind(4, e.grantedBy).bind(5, e.grantedAt);
    stmt.run();
}

// ---------------------------------------------------------------- marks

namespace {

const char* const kMarkColumns =
    "m.mark_id, m.submission_id, m.raw_mark, m.penalty, m.final_mark, m.feedback, m.status, m.marked_by, "
    "m.marked_at";

Mark readMark(const Statement& s) {
    Mark m;
    m.id = s.getInt(0);
    m.submissionId = s.getInt(1);
    m.rawMark = s.getDouble(2);
    m.penalty = s.getDouble(3);
    m.finalMark = s.getDouble(4);
    m.feedback = s.getText(5);
    m.status = markStatusFromString(s.getText(6));
    m.markedBy = s.getInt(7);
    m.markedAt = s.getText(8);
    return m;
}

}  // namespace

std::optional<Mark> SqliteMarkRepository::findById(int markId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kMarkColumns + " FROM marks m WHERE m.mark_id = ?");
    stmt.bind(1, markId);
    if (!stmt.step()) return std::nullopt;
    return readMark(stmt);
}

std::optional<Mark> SqliteMarkRepository::findBySubmission(int submissionId) {
    Statement stmt =
        db_.prepare(std::string("SELECT ") + kMarkColumns + " FROM marks m WHERE m.submission_id = ?");
    stmt.bind(1, submissionId);
    if (!stmt.step()) return std::nullopt;
    return readMark(stmt);
}

std::vector<Mark> SqliteMarkRepository::listForAssignment(int assignmentId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kMarkColumns +
                                 " FROM marks m JOIN submissions s ON s.submission_id = m.submission_id "
                                 "WHERE s.assignment_id = ? ORDER BY m.mark_id");
    stmt.bind(1, assignmentId);
    std::vector<Mark> result;
    while (stmt.step()) result.push_back(readMark(stmt));
    return result;
}

int SqliteMarkRepository::insert(const Mark& m) {
    Statement stmt = db_.prepare(
        "INSERT INTO marks (submission_id, raw_mark, penalty, final_mark, feedback, status, marked_by, marked_at) "
        "VALUES (?,?,?,?,?,?,?,?)");
    stmt.bind(1, m.submissionId)
        .bind(2, m.rawMark)
        .bind(3, m.penalty)
        .bind(4, m.finalMark)
        .bind(5, m.feedback)
        .bind(6, toString(m.status))
        .bind(7, m.markedBy)
        .bind(8, m.markedAt);
    stmt.run();
    return static_cast<int>(db_.lastInsertId());
}

void SqliteMarkRepository::update(const Mark& m) {
    Statement stmt = db_.prepare(
        "UPDATE marks SET raw_mark = ?, penalty = ?, final_mark = ?, feedback = ?, status = ?, marked_by = ?, "
        "marked_at = ? WHERE mark_id = ?");
    stmt.bind(1, m.rawMark)
        .bind(2, m.penalty)
        .bind(3, m.finalMark)
        .bind(4, m.feedback)
        .bind(5, toString(m.status))
        .bind(6, m.markedBy)
        .bind(7, m.markedAt)
        .bind(8, m.id);
    stmt.run();
}

int SqliteMarkRepository::addDecision(const ModerationDecision& d) {
    Statement stmt = db_.prepare(
        "INSERT INTO moderation_decisions (mark_id, hod_user_id, decision, previous_mark, new_mark, reason, "
        "decided_at) VALUES (?,?,?,?,?,?,?)");
    stmt.bind(1, d.markId)
        .bind(2, d.hodUserId)
        .bind(3, toString(d.decision))
        .bind(4, d.previousMark)
        .bind(5, d.newMark)
        .bind(6, d.reason)
        .bind(7, d.decidedAt);
    stmt.run();
    return static_cast<int>(db_.lastInsertId());
}

std::vector<ModerationDecision> SqliteMarkRepository::decisionsForMark(int markId) {
    Statement stmt = db_.prepare(
        "SELECT decision_id, mark_id, hod_user_id, decision, previous_mark, new_mark, reason, decided_at "
        "FROM moderation_decisions WHERE mark_id = ? ORDER BY decision_id");
    stmt.bind(1, markId);
    std::vector<ModerationDecision> result;
    while (stmt.step()) {
        ModerationDecision d;
        d.id = stmt.getInt(0);
        d.markId = stmt.getInt(1);
        d.hodUserId = stmt.getInt(2);
        d.decision = moderationActionFromString(stmt.getText(3));
        d.previousMark = stmt.getOptionalDouble(4);
        d.newMark = stmt.getOptionalDouble(5);
        d.reason = stmt.getText(6);
        d.decidedAt = stmt.getText(7);
        result.push_back(d);
    }
    return result;
}

}  // namespace mlms
