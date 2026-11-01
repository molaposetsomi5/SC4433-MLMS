#include "data/Database.h"

#include "domain/Errors.h"

namespace mlms {

// ---------------------------------------------------------------- Statement

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db) {
    const int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr);
    if (rc != SQLITE_OK) {
        throw DatabaseError(std::string("Could not prepare statement: ") + sqlite3_errmsg(db_));
    }
}

Statement::~Statement() {
    if (stmt_ != nullptr) sqlite3_finalize(stmt_);
}

Statement::Statement(Statement&& other) noexcept : db_(other.db_), stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

void Statement::check(int resultCode) const {
    if (resultCode != SQLITE_OK) throw DatabaseError(std::string("Database error: ") + sqlite3_errmsg(db_));
}

Statement& Statement::bind(int index, int value) {
    check(sqlite3_bind_int(stmt_, index, value));
    return *this;
}

Statement& Statement::bind(int index, std::int64_t value) {
    check(sqlite3_bind_int64(stmt_, index, value));
    return *this;
}

Statement& Statement::bind(int index, double value) {
    check(sqlite3_bind_double(stmt_, index, value));
    return *this;
}

Statement& Statement::bind(int index, const std::string& value) {
    check(sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT));
    return *this;
}

Statement& Statement::bind(int index, const char* value) { return bind(index, std::string(value)); }

Statement& Statement::bind(int index, const std::optional<int>& value) {
    return value ? bind(index, *value) : bindNull(index);
}

Statement& Statement::bind(int index, const std::optional<double>& value) {
    return value ? bind(index, *value) : bindNull(index);
}

Statement& Statement::bind(int index, const std::optional<std::string>& value) {
    return value ? bind(index, *value) : bindNull(index);
}

Statement& Statement::bindNull(int index) {
    check(sqlite3_bind_null(stmt_, index));
    return *this;
}

bool Statement::step() {
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw DatabaseError(std::string("Database error: ") + sqlite3_errmsg(db_));
}

void Statement::run() {
    while (step()) {
    }
}

int Statement::getInt(int column) const { return sqlite3_column_int(stmt_, column); }

std::int64_t Statement::getInt64(int column) const { return sqlite3_column_int64(stmt_, column); }

double Statement::getDouble(int column) const { return sqlite3_column_double(stmt_, column); }

std::string Statement::getText(int column) const {
    const unsigned char* text = sqlite3_column_text(stmt_, column);
    return text == nullptr ? std::string() : std::string(reinterpret_cast<const char*>(text));
}

bool Statement::isNull(int column) const { return sqlite3_column_type(stmt_, column) == SQLITE_NULL; }

std::optional<int> Statement::getOptionalInt(int column) const {
    if (isNull(column)) return std::nullopt;
    return getInt(column);
}

std::optional<double> Statement::getOptionalDouble(int column) const {
    if (isNull(column)) return std::nullopt;
    return getDouble(column);
}

std::optional<std::string> Statement::getOptionalText(int column) const {
    if (isNull(column)) return std::nullopt;
    return getText(column);
}

// ---------------------------------------------------------------- Database

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string message = db_ != nullptr ? sqlite3_errmsg(db_) : "out of memory";
        if (db_ != nullptr) sqlite3_close(db_);
        db_ = nullptr;
        throw DatabaseError("Could not open database '" + path + "': " + message);
    }
    sqlite3_busy_timeout(db_, 3000);  // wait instead of failing if the file is briefly locked
    exec("PRAGMA foreign_keys = ON;");
}

Database::~Database() {
    if (db_ != nullptr) sqlite3_close(db_);
}

Statement Database::prepare(const std::string& sql) { return Statement(db_, sql); }

void Database::exec(const std::string& sql) {
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error != nullptr ? error : "unknown error";
        sqlite3_free(error);
        throw DatabaseError("Database error: " + message);
    }
}

std::int64_t Database::lastInsertId() const { return sqlite3_last_insert_rowid(db_); }

bool Database::inTransaction() const { return sqlite3_get_autocommit(db_) == 0; }

// ---------------------------------------------------------------- TransactionManager

void TransactionManager::begin() {
    if (depth_ == 0) db_.exec("BEGIN IMMEDIATE;");
    ++depth_;
}

void TransactionManager::commit() {
    --depth_;
    if (depth_ == 0) db_.exec("COMMIT;");
}

void TransactionManager::rollback() {
    --depth_;
    // SQLite may already have rolled back by itself after some errors
    if (depth_ == 0 && db_.inTransaction()) db_.exec("ROLLBACK;");
}

}  // namespace mlms
