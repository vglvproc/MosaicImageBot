#pragma once
#include "SqliteTable.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <mutex>

class DatabaseManager {
public:
    enum class TableCreationResult {
        NOT_EXISTED_AND_CREATED,
        EXISTED_AND_RECREATED,
        ALREADY_EXISTS,
        ERROR
    };

    DatabaseManager(const std::string& dbPath, bool pragmaTimeout = true, int timeout_ms = 60000);
    ~DatabaseManager();

    TableCreationResult checkAndCreateTable(const SqliteTable& table); // Checks and creates a table if it does not exist, or recreates it if the structure of the existing table differs from the expected

    bool executeSQL(const std::string& sql); // Executes an SQL command

    std::vector<std::vector<SqliteTable::FieldValue>> executeSelectSQL(const std::string& sql);
    void printResults(const std::vector<std::vector<SqliteTable::FieldValue>>& results);

    // Other potentially useful functions
    bool tableExists(const std::string& tableName, bool& errorOccured); // Checks if a table exists

    bool checkTableStructure(const SqliteTable& expectedTable, bool& errorOccured); // Checks table structure

private:
    std::string dbPath_; // Path to the database file
    sqlite3* db_; // Pointer to the SQLite database object
    static std::mutex dbMutex_;
    static std::mutex dbCheckAndCreateTableMutex_;
    int busyTimeout_;

    bool initializeDatabase(bool walMode, bool busyTimeout); // Initializes the database, creates it if it does not exist
};