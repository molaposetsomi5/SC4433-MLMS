#include "data/SqliteRepositories.h"

namespace mlms {

namespace {

const char* const kUserColumns =
    "user_id, username, password_hash, role, display_name, department_id, student_number, "
    "is_active, failed_login_count, locked_until, created_at";

std::shared_ptr<User> readUser(const Statement& stmt) {
    UserRecord r;
    r.id = stmt.getInt(0);
    r.username = stmt.getText(1);
    r.passwordHash = stmt.getText(2);
    r.role = roleFromString(stmt.getText(3));
    r.displayName = stmt.getText(4);
    r.departmentId = stmt.getOptionalInt(5);
    r.studentNumber = stmt.getOptionalText(6);
    r.active = stmt.getInt(7) == 1;
    r.failedLoginCount = stmt.getInt(8);
    r.lockedUntil = stmt.getOptionalText(9);
    r.createdAt = stmt.getText(10);
    return UserFactory::create(r);
}

}  // namespace

std::shared_ptr<User> SqliteUserRepository::findById(int userId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kUserColumns + " FROM users WHERE user_id = ?");
    stmt.bind(1, userId);
    return stmt.step() ? readUser(stmt) : nullptr;
}

std::shared_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kUserColumns + " FROM users WHERE username = ?");
    stmt.bind(1, username);
    return stmt.step() ? readUser(stmt) : nullptr;
}

std::vector<std::shared_ptr<User>> SqliteUserRepository::listAll() {
    Statement stmt = db_.prepare(std::string("SELECT ") + kUserColumns + " FROM users ORDER BY role, username");
    std::vector<std::shared_ptr<User>> users;
    while (stmt.step()) users.push_back(readUser(stmt));
    return users;
}

int SqliteUserRepository::count() {
    Statement stmt = db_.prepare("SELECT COUNT(*) FROM users");
    return stmt.step() ? stmt.getInt(0) : 0;
}

bool SqliteUserRepository::studentNumberExists(const std::string& studentNumber) {
    Statement stmt = db_.prepare("SELECT 1 FROM users WHERE student_number = ?");
    stmt.bind(1, studentNumber);
    return stmt.step();
}

int SqliteUserRepository::insert(const UserRecord& r) {
    Statement stmt = db_.prepare(
        "INSERT INTO users (username, password_hash, role, display_name, department_id, student_number, "
        "is_active, failed_login_count, locked_until, created_at) VALUES (?,?,?,?,?,?,?,?,?,?)");
    stmt.bind(1, r.username)
        .bind(2, r.passwordHash)
        .bind(3, toString(r.role))
        .bind(4, r.displayName)
        .bind(5, r.departmentId)
        .bind(6, r.studentNumber)
        .bind(7, r.active ? 1 : 0)
        .bind(8, r.failedLoginCount)
        .bind(9, r.lockedUntil)
        .bind(10, r.createdAt);
    stmt.run();
    return static_cast<int>(db_.lastInsertId());
}

void SqliteUserRepository::update(const UserRecord& r) {
    Statement stmt = db_.prepare(
        "UPDATE users SET username = ?, password_hash = ?, role = ?, display_name = ?, department_id = ?, "
        "student_number = ?, is_active = ?, failed_login_count = ?, locked_until = ? WHERE user_id = ?");
    stmt.bind(1, r.username)
        .bind(2, r.passwordHash)
        .bind(3, toString(r.role))
        .bind(4, r.displayName)
        .bind(5, r.departmentId)
        .bind(6, r.studentNumber)
        .bind(7, r.active ? 1 : 0)
        .bind(8, r.failedLoginCount)
        .bind(9, r.lockedUntil)
        .bind(10, r.id);
    stmt.run();
}

}  // namespace mlms
