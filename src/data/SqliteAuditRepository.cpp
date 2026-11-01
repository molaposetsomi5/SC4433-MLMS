#include "data/SqliteRepositories.h"

namespace mlms {

namespace {

const char* const kAuditColumns =
    "audit_id, actor_user_id, event_type, entity_type, entity_id, timestamp, old_value, new_value, reason";

AuditRecord readAudit(const Statement& s) {
    AuditRecord r;
    r.id = s.getInt(0);
    r.actorUserId = s.getInt(1);
    r.eventType = s.getText(2);
    r.entityType = s.getText(3);
    r.entityId = s.getInt(4);
    r.timestamp = s.getText(5);
    r.oldValue = s.getOptionalText(6);
    r.newValue = s.getOptionalText(7);
    r.reason = s.getText(8);
    return r;
}

}  // namespace

int SqliteAuditRepository::append(const AuditRecord& r) {
    Statement stmt = db_.prepare(
        "INSERT INTO audit_logs (actor_user_id, event_type, entity_type, entity_id, timestamp, old_value, "
        "new_value, reason) VALUES (?,?,?,?,?,?,?,?)");
    stmt.bind(1, r.actorUserId)
        .bind(2, r.eventType)
        .bind(3, r.entityType)
        .bind(4, r.entityId)
        .bind(5, r.timestamp)
        .bind(6, r.oldValue)
        .bind(7, r.newValue)
        .bind(8, r.reason);
    stmt.run();
    return static_cast<int>(db_.lastInsertId());
}

std::vector<AuditRecord> SqliteAuditRepository::listAll() {
    Statement stmt = db_.prepare(std::string("SELECT ") + kAuditColumns + " FROM audit_logs ORDER BY audit_id");
    std::vector<AuditRecord> result;
    while (stmt.step()) result.push_back(readAudit(stmt));
    return result;
}

std::vector<AuditRecord> SqliteAuditRepository::listForEntity(const std::string& entityType, int entityId) {
    Statement stmt = db_.prepare(std::string("SELECT ") + kAuditColumns +
                                 " FROM audit_logs WHERE entity_type = ? AND entity_id = ? ORDER BY audit_id");
    stmt.bind(1, entityType).bind(2, entityId);
    std::vector<AuditRecord> result;
    while (stmt.step()) result.push_back(readAudit(stmt));
    return result;
}

}  // namespace mlms
