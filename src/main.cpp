#include <iostream>
#include <fstream>
#include <sstream>
#include "sqlite3.h"

using namespace std;

int main()
{
    sqlite3* database = nullptr;

    // Open the database
    int result = sqlite3_open("mlms.db", &database);

    if (result != SQLITE_OK)
    {
        cout << "Database connection failed: "
            << sqlite3_errmsg(database) << endl;

        sqlite3_close(database);
        return 1;
    }

    cout << "Database connected successfully!" << endl;

    // Enable foreign keys
    char* errorMessage = nullptr;

    result = sqlite3_exec(
        database,
        "PRAGMA foreign_keys = ON;",
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        cout << "Foreign key setup failed: "
            << errorMessage << endl;

        sqlite3_free(errorMessage);
        sqlite3_close(database);
        return 1;
    }

    // Open the schema file
    ifstream schemaFile("schema.sql");

    if (!schemaFile.is_open())
    {
        cout << "Could not open schema.sql!" << endl;
        sqlite3_close(database);
        return 1;
    }

    // Read the complete SQL file
    stringstream buffer;
    buffer << schemaFile.rdbuf();

    string schema = buffer.str();

    schemaFile.close();

    // Execute the schema
    result = sqlite3_exec(
        database,
        schema.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        cout << "Schema execution failed: "
            << errorMessage << endl;

        sqlite3_free(errorMessage);
        sqlite3_close(database);
        return 1;
    }

    cout << "Database schema executed successfully!" << endl;

    // Display created tables
    const char* sql =
        "SELECT name FROM sqlite_master "
        "WHERE type = 'table' "
        "AND name NOT LIKE 'sqlite_%' "
        "AND name <> 'TestUsers' "
        "ORDER BY name;";

    sqlite3_stmt* statement = nullptr;

    result = sqlite3_prepare_v2(
        database,
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        cout << "Could not read database tables: "
            << sqlite3_errmsg(database) << endl;

        sqlite3_close(database);
        return 1;
    }

    cout << "\nTables created:\n";

    int tableCount = 0;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        const unsigned char* tableName =
            sqlite3_column_text(statement, 0);

        cout << ++tableCount << ". " << tableName << endl;
    }

    sqlite3_finalize(statement);

    cout << "\nTotal tables: " << tableCount << endl;

    sqlite3_close(database);

    cout << "\nDatabase setup completed!" << endl;

    return 0;
}