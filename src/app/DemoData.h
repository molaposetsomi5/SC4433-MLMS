#pragma once
// Sample data for demonstrations (viva) and for the 800-student performance test.
#include <string>

#include "app/AppContext.h"

namespace mlms {

constexpr const char* kDemoPassword = "Password123";

// Loads a small school: 2 departments, 3 courses, staff and 6 students.
// Only allowed on an empty database.
void seedDemoData(AppContext& app);

// Adds `count` extra students enrolled in every course (password = kDemoPassword).
// One hash is reused for speed - this is test data only.
void seedBulkStudents(AppContext& app, int count);

}  // namespace mlms
