#pragma once
// Thin RAII wrappers around the SQLite C API (risk R4 mitigation).
// Only the data-access layer includes this file.
#include <sqlite3.h>

#include <cstdint>
#include <optional>
#include <string>

namespace mlms {

class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&&) = delete;

    // Parameter indexes start at 1, as in SQLite.
    Statement& bind(int index, int value);
    Statement& bind(int index, std::int64_t value);
    Statement& bind(int index, double value);
    Statement& bind(int index, const std::string& value);
    Statement& bind(int index, const char* value);
    Statement& bind(int index, const std::optional<int>& value);
    Statement& bind(int index, const std::optional<double>& value);
    Statement& bind(int index, const std::optional<std::string>& value);
    Statement& bindNull(int index);

    bool step();  // true while a row is available
    void run();   // execute a statement that returns no rows

    int getInt(int column) const;
    std::int64_t getInt64(int column) const;
    double getDouble(int column) const;
    std::string getText(int column) const;
    bool isNull(int column) const;
    std::optional<int> getOptionalInt(int column) const;
    std::optional<double> getOptionalDouble(int column) const;
    std::optional<std::string> getOptionalText(int column) const;

private:
    void check(int resultCode) const;
    sqlite3* db_;
    sqlite3_stmt* stmt_ = nullptr;
};

class Database {
public:
    // Use ":memory:" for an in-memory database (tests).
    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    Statement prepare(const std::string& sql);
    void exec(const std::string& sql);
    std::int64_t lastInsertId() const;
    bool inTransaction() const;

private:
    sqlite3* db_ = nullptr;
};

// Runs a unit of work as one SQLite transaction: commit on success,
// rollback if anything throws. Nested calls join the outer transaction.
class TransactionManager {
public:
    explicit TransactionManager(Database& db) : db_(db) {}

    template <typename Work>
    void run(Work&& work) {
        begin();
        try {
            work();
        } catch (...) {
            rollback();
            throw;
        }
        commit();
    }

private:
    void begin();
    void commit();
    void rollback();

    Database& db_;
    int depth_ = 0;
};

}  // namespace mlms
