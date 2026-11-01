#pragma once
// Repository interfaces (design section 4.4, pattern 2). Services depend on
// these abstractions only; SQLite details live in the Sqlite* classes.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "domain/Types.h"
#include "domain/User.h"

namespace mlms {

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual std::shared_ptr<User> findById(int userId) = 0;  // nullptr if not found
    virtual std::shared_ptr<User> findByUsername(const std::string& username) = 0;
    virtual std::vector<std::shared_ptr<User>> listAll() = 0;
    virtual int count() = 0;
    virtual bool studentNumberExists(const std::string& studentNumber) = 0;
    virtual int insert(const UserRecord& record) = 0;
    virtual void update(const UserRecord& record) = 0;
};

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    virtual int addDepartment(const std::string& name) = 0;
    virtual std::vector<Department> listDepartments() = 0;
    virtual std::optional<Department> findDepartment(int departmentId) = 0;
    virtual std::optional<Department> findDepartmentByName(const std::string& name) = 0;

    virtual int addCourse(const Course& course) = 0;
    virtual std::vector<Course> listCourses() = 0;
    virtual std::optional<Course> findCourse(int courseId) = 0;
    virtual std::optional<Course> findCourseByCode(const std::string& code) = 0;
    virtual std::vector<Course> coursesForLecturer(int lecturerId) = 0;
    virtual std::vector<Course> coursesForStudent(int studentId) = 0;
    virtual std::vector<Course> coursesForDepartment(int departmentId) = 0;

    virtual void assignLecturer(int lecturerId, int courseId) = 0;
    virtual bool isLecturerOf(int lecturerId, int courseId) = 0;
    virtual void enrolStudent(int studentId, int courseId, const std::string& enrolledAt) = 0;
    virtual bool isEnrolled(int studentId, int courseId) = 0;
    virtual std::vector<int> studentsInCourse(int courseId) = 0;
};

class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual int insert(const Assignment& assignment) = 0;
    virtual std::optional<Assignment> findById(int assignmentId) = 0;
    virtual std::vector<Assignment> listForCourse(int courseId) = 0;
};

class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    virtual std::optional<Submission> findById(int submissionId) = 0;
    virtual std::optional<Submission> find(int assignmentId, int studentId) = 0;
    virtual std::vector<Submission> listForAssignment(int assignmentId) = 0;
    virtual int create(const Submission& submission) = 0;
    virtual void updateStatus(int submissionId, SubmissionStatus status) = 0;
    // Adds a new version and makes it the only "latest valid" one. Returns the version number.
    virtual int addVersion(int submissionId, const std::string& submittedAt, const std::string& content) = 0;
    virtual std::optional<SubmissionVersion> latestVersion(int submissionId) = 0;
    virtual std::vector<SubmissionVersion> versions(int submissionId) = 0;
};

class IExtensionRepository {
public:
    virtual ~IExtensionRepository() = default;
    virtual std::optional<Extension> findById(int extensionId) = 0;
    virtual std::optional<Extension> find(int assignmentId, int studentId) = 0;
    virtual void save(const Extension& extension) = 0;  // insert or replace the single extension
    // v1.1 (CR1)
    virtual std::optional<ClassExtension> findClassExtension(int assignmentId) = 0;
    virtual void saveClassExtension(const ClassExtension& extension) = 0;  // insert or replace
};

class IMarkRepository {
public:
    virtual ~IMarkRepository() = default;
    virtual std::optional<Mark> findById(int markId) = 0;
    virtual std::optional<Mark> findBySubmission(int submissionId) = 0;
    virtual std::vector<Mark> listForAssignment(int assignmentId) = 0;
    virtual int insert(const Mark& mark) = 0;
    virtual void update(const Mark& mark) = 0;
    virtual int addDecision(const ModerationDecision& decision) = 0;
    virtual std::vector<ModerationDecision> decisionsForMark(int markId) = 0;
};

class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual int append(const AuditRecord& record) = 0;
    virtual std::vector<AuditRecord> listAll() = 0;
    virtual std::vector<AuditRecord> listForEntity(const std::string& entityType, int entityId) = 0;
};

}  // namespace mlms
