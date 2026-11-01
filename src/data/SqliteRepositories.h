#pragma once
// SQLite implementations of the repository interfaces.
// Every query uses bound parameters - no SQL is built from user input.
#include "data/Database.h"
#include "data/Repositories.h"

namespace mlms {

class SqliteUserRepository final : public IUserRepository {
public:
    explicit SqliteUserRepository(Database& db) : db_(db) {}
    std::shared_ptr<User> findById(int userId) override;
    std::shared_ptr<User> findByUsername(const std::string& username) override;
    std::vector<std::shared_ptr<User>> listAll() override;
    int count() override;
    bool studentNumberExists(const std::string& studentNumber) override;
    int insert(const UserRecord& record) override;
    void update(const UserRecord& record) override;

private:
    Database& db_;
};

class SqliteCourseRepository final : public ICourseRepository {
public:
    explicit SqliteCourseRepository(Database& db) : db_(db) {}
    int addDepartment(const std::string& name) override;
    std::vector<Department> listDepartments() override;
    std::optional<Department> findDepartment(int departmentId) override;
    std::optional<Department> findDepartmentByName(const std::string& name) override;
    int addCourse(const Course& course) override;
    std::vector<Course> listCourses() override;
    std::optional<Course> findCourse(int courseId) override;
    std::optional<Course> findCourseByCode(const std::string& code) override;
    std::vector<Course> coursesForLecturer(int lecturerId) override;
    std::vector<Course> coursesForStudent(int studentId) override;
    std::vector<Course> coursesForDepartment(int departmentId) override;
    void assignLecturer(int lecturerId, int courseId) override;
    bool isLecturerOf(int lecturerId, int courseId) override;
    void enrolStudent(int studentId, int courseId, const std::string& enrolledAt) override;
    bool isEnrolled(int studentId, int courseId) override;
    std::vector<int> studentsInCourse(int courseId) override;

private:
    std::vector<Course> queryCourses(Statement& stmt);
    Database& db_;
};

class SqliteAssignmentRepository final : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(Database& db) : db_(db) {}
    int insert(const Assignment& assignment) override;
    std::optional<Assignment> findById(int assignmentId) override;
    std::vector<Assignment> listForCourse(int courseId) override;

private:
    Database& db_;
};

class SqliteSubmissionRepository final : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(Database& db) : db_(db) {}
    std::optional<Submission> findById(int submissionId) override;
    std::optional<Submission> find(int assignmentId, int studentId) override;
    std::vector<Submission> listForAssignment(int assignmentId) override;
    int create(const Submission& submission) override;
    void updateStatus(int submissionId, SubmissionStatus status) override;
    int addVersion(int submissionId, const std::string& submittedAt, const std::string& content) override;
    std::optional<SubmissionVersion> latestVersion(int submissionId) override;
    std::vector<SubmissionVersion> versions(int submissionId) override;

private:
    Database& db_;
};

class SqliteExtensionRepository final : public IExtensionRepository {
public:
    explicit SqliteExtensionRepository(Database& db) : db_(db) {}
    std::optional<Extension> findById(int extensionId) override;
    std::optional<Extension> find(int assignmentId, int studentId) override;
    void save(const Extension& extension) override;
    std::optional<ClassExtension> findClassExtension(int assignmentId) override;
    void saveClassExtension(const ClassExtension& extension) override;

private:
    Database& db_;
};

class SqliteMarkRepository final : public IMarkRepository {
public:
    explicit SqliteMarkRepository(Database& db) : db_(db) {}
    std::optional<Mark> findById(int markId) override;
    std::optional<Mark> findBySubmission(int submissionId) override;
    std::vector<Mark> listForAssignment(int assignmentId) override;
    int insert(const Mark& mark) override;
    void update(const Mark& mark) override;
    int addDecision(const ModerationDecision& decision) override;
    std::vector<ModerationDecision> decisionsForMark(int markId) override;

private:
    Database& db_;
};

class SqliteAuditRepository final : public IAuditRepository {
public:
    explicit SqliteAuditRepository(Database& db) : db_(db) {}
    int append(const AuditRecord& record) override;
    std::vector<AuditRecord> listAll() override;
    std::vector<AuditRecord> listForEntity(const std::string& entityType, int entityId) override;

private:
    Database& db_;
};

}  // namespace mlms
