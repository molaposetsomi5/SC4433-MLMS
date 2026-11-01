// NFR-01 performance check: builds a database with 800 students and times
// login and opening a student's assignment list.
// Usage: mlms_perf [--db FILE]   (default: perf_test.db, recreated each run)
#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "app/AppContext.h"
#include "app/DemoData.h"

namespace {

template <typename Work>
double timeMs(Work work) {
    const auto start = std::chrono::steady_clock::now();
    work();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string dbPath = "perf_test.db";
    if (argc == 3 && std::string(argv[1]) == "--db") dbPath = argv[2];
    std::remove(dbPath.c_str());

    mlms::AppContext app(dbPath, std::make_unique<mlms::SystemClock>(),
                         std::make_unique<mlms::SodiumPasswordHasher>());
    mlms::seedDemoData(app);
    mlms::seedBulkStudents(app, 794);  // 6 demo students + 794 = 800 students
    std::cout << "Users in database: " << app.userRepo.count() << " (800 students + staff)\n";

    std::shared_ptr<mlms::User> user;
    const double loginMs = timeMs([&] { user = app.auth.login("bulk400", mlms::kDemoPassword); });

    std::size_t count = 0;
    const double listMs = timeMs([&] { count = app.assignments.listForStudent(*user).size(); });

    std::cout << "Login:                 " << loginMs << " ms (target < 2000 ms)\n";
    std::cout << "Open assignment list:  " << listMs << " ms for " << count << " assignments (target < 2000 ms)\n";
    const bool pass = loginMs < 2000 && listMs < 2000;
    std::cout << (pass ? "NFR-01 PASS" : "NFR-01 FAIL") << "\n";
    std::remove(dbPath.c_str());
    return pass ? 0 : 1;
}
