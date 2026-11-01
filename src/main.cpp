#include <iostream>
#include "sqlite3.h"

using namespace std;

// Callback for SELECT - MUST return int
int callback(void* data, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << endl;
    }
    cout << endl;
    return 0;
}

int main()
{
    sqlite3* database;
    int result = sqlite3_open("mlms.db", &database);

    if (result != SQLITE_OK)
    {
        cout << "Database connection failed!" << endl;
        cout << sqlite3_errmsg(database) << endl;
        sqlite3_close(database);
        return 1;
    }
    cout << "Database connected successfully!" << endl;

    const char* sql =
        "CREATE TABLE IF NOT EXISTS TestUsers ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL);"
        "INSERT INTO TestUsers (name) VALUES ('Setso');"
        "SELECT * FROM TestUsers;";

    char* errorMessage = nullptr;

    result = sqlite3_exec(database, sql, callback, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        cout << "SQL failed!" << endl;
        cout << errorMessage << endl;
        sqlite3_free(errorMessage);
        sqlite3_close(database);
        return 1;
    }

    cout << "Database test completed!" << endl;
    sqlite3_close(database);
    return 0;
}