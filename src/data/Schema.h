#pragma once
#include "data/Database.h"

namespace mlms {

// Creates or upgrades the schema. The version is kept in PRAGMA user_version.
void migrateSchema(Database& db);

int schemaVersion(Database& db);

}  // namespace mlms
