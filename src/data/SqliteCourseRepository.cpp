#include "data/SqliteRepositories.h"

namespace mlms {

int SqliteCourseRepository::addDepartment(const std::string& name) {
    Statement stmt = db_.prepare("INSERT INTO departments (name) VALUES (?)");
    stmt.bind(1, name);
    stmt.run();
    return static_cast<int>(db_.lastInsertId());
}

std::vector<Department> SqliteCourseRepository::listDepartments() {
    Statement stmt = db_.prepare("SELECT department_id, name FROM departments ORDER BY name");
    std::vector<Department> result;
    while (stmt.step()) result.push_back({stmt.getInt(0), stmt.getText(1)});
    return result;
}

std::optional<Department> SqliteCourseRepository::findDepartment(int departmentId) {
    Statement stmt = db_.prepare("SELECT department_id, name FROM departments WHERE department_id = ?");
    stmt.bind(1, departmentId);
    if (!stmt.step()) return std::nullopt;
    return Department{stmt.getInt(0), stmt.getText(1)};
}

std::optional<Department> SqliteCourseRepository::findDepartmentByName(const std::string& name) {
    Statement stmt = db_.prepare("SELECT department_id, name FROM departments WHERE name = ?");
    stmt.bind(1, name);
    if (!stmt.step()) return std::nullopt;
    return Department{stmt.getInt(0), stmt.getText(1)};
}

int SqliteCourseRepository::addCourse(const Course& course) {
    Statement stmt = db_.prepare("INSERT INTO courses (department_id, code, name) VALUES (?,?,?)");
    stmt.bind(1, course.departmentId).bind(2, course.code).bind(3, course.name);
    stmt.run();
    return static_cast<int>(db_.lastInsertId());
}

std::vector<Course> SqliteCourseRepository::queryCourses(Statement& stmt) {
    std::vector<Course> result;
    while (stmt.step()) {
        result.push_back({stmt.getInt(0), stmt.getInt(1), stmt.getText(2), stmt.getText(3)});
    }
    return result;
}

std::vector<Course> SqliteCourseRepository::listCourses() {
    Statement stmt = db_.prepare("SELECT course_id, department_id, code, name FROM courses ORDER BY code");
    return queryCourses(stmt);
}

std::optional<Course> SqliteCourseRepository::findCourse(int courseId) {
    Statement stmt = db_.prepare("SELECT course_id, department_id, code, name FROM courses WHERE course_id = ?");
    stmt.bind(1, courseId);
    auto courses = queryCourses(stmt);
    if (courses.empty()) return std::nullopt;
    return courses.front();
}

std::optional<Course> SqliteCourseRepository::findCourseByCode(const std::string& code) {
    Statement stmt = db_.prepare("SELECT course_id, department_id, code, name FROM courses WHERE code = ?");
    stmt.bind(1, code);
    auto courses = queryCourses(stmt);
    if (courses.empty()) return std::nullopt;
    return courses.front();
}

std::vector<Course> SqliteCourseRepository::coursesForLecturer(int lecturerId) {
    Statement stmt = db_.prepare(
        "SELECT c.course_id, c.department_id, c.code, c.name FROM courses c "
        "JOIN lecturer_courses lc ON lc.course_id = c.course_id WHERE lc.lecturer_id = ? ORDER BY c.code");
    stmt.bind(1, lecturerId);
    return queryCourses(stmt);
}

std::vector<Course> SqliteCourseRepository::coursesForStudent(int studentId) {
    Statement stmt = db_.prepare(
        "SELECT c.course_id, c.department_id, c.code, c.name FROM courses c "
        "JOIN enrolments e ON e.course_id = c.course_id WHERE e.student_id = ? ORDER BY c.code");
    stmt.bind(1, studentId);
    return queryCourses(stmt);
}

std::vector<Course> SqliteCourseRepository::coursesForDepartment(int departmentId) {
    Statement stmt = db_.prepare(
        "SELECT course_id, department_id, code, name FROM courses WHERE department_id = ? ORDER BY code");
    stmt.bind(1, departmentId);
    return queryCourses(stmt);
}

void SqliteCourseRepository::assignLecturer(int lecturerId, int courseId) {
    Statement stmt = db_.prepare("INSERT OR IGNORE INTO lecturer_courses (lecturer_id, course_id) VALUES (?,?)");
    stmt.bind(1, lecturerId).bind(2, courseId);
    stmt.run();
}

bool SqliteCourseRepository::isLecturerOf(int lecturerId, int courseId) {
    Statement stmt = db_.prepare("SELECT 1 FROM lecturer_courses WHERE lecturer_id = ? AND course_id = ?");
    stmt.bind(1, lecturerId).bind(2, courseId);
    return stmt.step();
}

void SqliteCourseRepository::enrolStudent(int studentId, int courseId, const std::string& enrolledAt) {
    Statement stmt =
        db_.prepare("INSERT OR IGNORE INTO enrolments (student_id, course_id, enrolled_at) VALUES (?,?,?)");
    stmt.bind(1, studentId).bind(2, courseId).bind(3, enrolledAt);
    stmt.run();
}

bool SqliteCourseRepository::isEnrolled(int studentId, int courseId) {
    Statement stmt = db_.prepare("SELECT 1 FROM enrolments WHERE student_id = ? AND course_id = ?");
    stmt.bind(1, studentId).bind(2, courseId);
    return stmt.step();
}

std::vector<int> SqliteCourseRepository::studentsInCourse(int courseId) {
    Statement stmt = db_.prepare("SELECT student_id FROM enrolments WHERE course_id = ? ORDER BY student_id");
    stmt.bind(1, courseId);
    std::vector<int> ids;
    while (stmt.step()) ids.push_back(stmt.getInt(0));
    return ids;
}

}  // namespace mlms
