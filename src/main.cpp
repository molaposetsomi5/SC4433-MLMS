// Entry point. Usage:
//   mlms [--db FILE] [--seed-demo]
//   --db FILE     database file to use (default: mlms.db in the current folder)
//   --seed-demo   load sample data into an EMPTY database, then start
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "app/AppContext.h"
#include "app/DemoData.h"
#include "domain/Errors.h"
#include "ui/ConsoleUI.h"

// cppcheck-suppress constParameter ; standard main() signature
int main(int argc, char* argv[]) {
    std::string dbPath = "mlms.db";
    bool seedDemo = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--db" && i + 1 < argc) {
            dbPath = argv[++i];
        } else if (arg == "--seed-demo") {
            seedDemo = true;
        } else {
            std::cerr << "Usage: mlms [--db FILE] [--seed-demo]\n";
            return 2;
        }
    }

    try {
        mlms::AppContext app(dbPath, std::make_unique<mlms::SystemClock>(),
                             std::make_unique<mlms::SodiumPasswordHasher>());
        if (seedDemo) {
            mlms::seedDemoData(app);
            std::cout << "Demo data loaded. All demo passwords are: " << mlms::kDemoPassword << "\n";
        }
        mlms::ConsoleUI ui(app, std::cin, std::cout);
        ui.run();
    } catch (const mlms::MlmsError& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
