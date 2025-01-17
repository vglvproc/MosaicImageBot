#include "DatabaseManager.h"
#include "../log/Logger.h"
#include <iostream>
#include <filesystem>

std::mutex DatabaseManager::dbMutex_;
std::mutex DatabaseManager::dbCheckAndCreateTableMutex_;

DatabaseManager::DatabaseManager(const std::string& dbPath, bool pragmaTimeout, int timeout_ms)
    : dbPath_(dbPath), db_(nullptr), busyTimeout_(timeout_ms) {
    std::unique_lock<std::mutex> lock(dbMutex_);

    bool dbFileExists = std::filesystem::exists(dbPath_);

    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        Logger::log(LogSource::Database, std::cerr, std::string("Error opening database: ") + sqlite3_errmsg(db_));
        sqlite3_close(db_);
        db_ = nullptr;
        lock.unlock();
        throw std::runtime_error("Failed to open database");
    }

    if (!initializeDatabase(!dbFileExists, pragmaTimeout)) {
        lock.unlock();
        throw std::runtime_error("Failed to initialize database");
    }
}

bool DatabaseManager::initializeDatabase(bool walMode, bool busyTimeout) {
    // There is no need to check for database file existence, as sqlite3_open will create the file if it does not exist.
    // Here you might include code to set initial database parameters, such as PRAGMA statements.
    // PRAGMA commands can be used to improve performance or manage database behavior.
    if (sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        Logger::log(LogSource::Database, std::cerr, std::string("Error setting PRAGMA foreign_keys: ") + sqlite3_errmsg(db_));
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    if (busyTimeout) {
        std::string pragma_query = "PRAGMA busy_timeout = " + std::to_string(busyTimeout_) + ";";
        if (sqlite3_exec(db_, pragma_query.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
            Logger::log(LogSource::Database, std::cerr, std::string("Error setting timeout: ") + sqlite3_errmsg(db_));
            sqlite3_close(db_);
            db_ = nullptr;
            return false;
        }
    }

    if (walMode) {
        if (sqlite3_exec(db_, "PRAGMA journal_mode = WAL;", nullptr, nullptr, nullptr) != SQLITE_OK) {
            Logger::log(LogSource::Database, std::cerr, std::string("Error setting WAL mode: ") + sqlite3_errmsg(db_));
            sqlite3_close(db_);
            db_ = nullptr;
            return false;
        }
    }

    return true;
}

DatabaseManager::TableCreationResult DatabaseManager::checkAndCreateTable(const SqliteTable& table) {
    std::unique_lock<std::mutex> lock(dbCheckAndCreateTableMutex_);
    bool errorOccured = false;
    bool result = tableExists(table.getName(), errorOccured);
    if (errorOccured) {
        return DatabaseManager::TableCreationResult::ERROR;
    }
    if (!result) {
        std::string createTableSql = table.createTableSQL();
        if (executeSQL(createTableSql)) {
            Logger::log(LogSource::Database, std::cout, std::string("Table '") + table.getName() + "' has been successfully created.");
            return DatabaseManager::TableCreationResult::NOT_EXISTED_AND_CREATED;
        } else {
            Logger::log(LogSource::Database, std::cerr, std::string("Failed to create table '") + table.getName() + "'.");
            return DatabaseManager::TableCreationResult::ERROR;
        }
    } else {
        result = checkTableStructure(table, errorOccured);
        if (errorOccured) {
            return DatabaseManager::TableCreationResult::ERROR;
        }
        if (result) {
            Logger::log(LogSource::Database, std::cerr, std::string("Table '") + table.getName() + "' already exists.");
            return DatabaseManager::TableCreationResult::ALREADY_EXISTS;
        } else {
            std::string dropTableSql = table.generateDropTableSQL();
            if (executeSQL(dropTableSql)) {
                Logger::log(LogSource::Database, std::cout, std::string("Table '") + table.getName() + "' has been successfully removed.");
                std::string createTableSql = table.createTableSQL();
                if (executeSQL(createTableSql)) {
                    Logger::log(LogSource::Database, std::cout, std::string("Table '") + table.getName() + "' has been successfully created.");
                    return DatabaseManager::TableCreationResult::EXISTED_AND_RECREATED;
                } else {
                    Logger::log(LogSource::Database, std::cerr, std::string("Failed to create table '") + table.getName() + "'.");
                    return DatabaseManager::TableCreationResult::ERROR;
                }
            } else {
                Logger::log(LogSource::Database, std::cerr, std::string("Failed to remove table '") + table.getName() + "'.");
                return DatabaseManager::TableCreationResult::ERROR;
            }
        }
    }
}

bool DatabaseManager::tableExists(const std::string& tableName, bool& errorOccured) {
    std::unique_lock<std::mutex> lock(dbMutex_);
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
    char* errMsg = nullptr;
    bool exists = false;
    errorOccured = false;

    auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
        bool* exists = static_cast<bool*>(data);
        *exists = true;
        return 0;
    };

    if (sqlite3_exec(db_, sql.c_str(), callback, &exists, &errMsg) != SQLITE_OK) {
        Logger::log(LogSource::Database, std::cerr, std::string("Error checking table existence: ") + errMsg);
        sqlite3_free(errMsg);
        errorOccured = true;
        return false;
    }

    return exists;
}

bool DatabaseManager::checkTableStructure(const SqliteTable& expectedTable, bool& errorOccured) {
    std::unique_lock<std::mutex> lock(dbMutex_);
    std::string sql = expectedTable.generatePragmaTableInfoSQL();
    sqlite3_stmt* stmt;
    errorOccured = false;

    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::log(LogSource::Database, std::cerr, std::string("Error preparing statement: ") + sqlite3_errmsg(db_));
        errorOccured = true;
        return false;
    }

    std::vector<SqliteTable::Field> actualFields;
    std::string actualPrimaryKey = "";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SqliteTable::Field field;
        field.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (type == "INTEGER") {
            field.type = SqliteTable::DataType::INTEGER;
        } else if (type == "REAL") {
            field.type = SqliteTable::DataType::REAL;
        } else if (type == "TEXT") {
            field.type = SqliteTable::DataType::TEXT;
        } else if (type == "BLOB") {
            field.type = SqliteTable::DataType::BLOB;
        } else {
            field.type = SqliteTable::DataType::TEXT; // default type in case of unknown type
        }

        actualFields.push_back(field);

        int isPrimaryKey = sqlite3_column_int(stmt, 5);
        if (isPrimaryKey == 1) {
            actualPrimaryKey = field.name;
        }
    }

    sqlite3_finalize(stmt);
    lock.unlock();

    if (actualPrimaryKey != expectedTable.getPrimaryKey()) {
        Logger::log(LogSource::Database, std::cerr, std::string("Primary key mismatch. Expected: ") + expectedTable.getPrimaryKey() + ", Actual: " + actualPrimaryKey);
        return false;
    }

    const auto& expectedFields = expectedTable.getFields();
    if (actualFields.size() != expectedFields.size()) {
        Logger::log(LogSource::Database, std::cerr, std::string("Field count mismatch. Expected: ") + std::to_string(expectedFields.size()) + ", Actual: " + std::to_string(actualFields.size()));
        return false;
    }

    for (size_t i = 0; i < expectedFields.size(); ++i) {
        if (expectedFields[i].name != actualFields[i].name || expectedFields[i].type != actualFields[i].type) {
            Logger::log(LogSource::Database, std::cerr, std::string("Field mismatch at position ") + std::to_string(i) + ". Expected: " + expectedFields[i].name + " " + SqliteTable::dataTypeToString(expectedFields[i].type) + ", Actual: " + actualFields[i].name +
                        " " + SqliteTable::dataTypeToString(actualFields[i].type));
            return false;
        }
    }

    return true;
}

bool DatabaseManager::executeSQL(const std::string& sql) {
    std::unique_lock<std::mutex> lock(dbMutex_);
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        Logger::log(LogSource::Database, std::cerr, std::string("SQL error: ") + errMsg);
        Logger::log(LogSource::Database, std::cerr, sql);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

std::vector<std::vector<SqliteTable::FieldValue>> DatabaseManager::executeSelectSQL(const std::string& sql) {
    std::unique_lock<std::mutex> lock(dbMutex_);
    sqlite3_stmt* stmt;
    std::vector<std::vector<SqliteTable::FieldValue>> results;

    Logger::log(LogSource::Database, std::cout, sql);

    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::log(LogSource::Database, std::cerr, std::string("Error preparing statement: ") + sqlite3_errmsg(db_));
        return results;
    }

    int columnCount = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<SqliteTable::FieldValue> row;
        for (int i = 0; i < columnCount; ++i) {
            SqliteTable::FieldValue fieldValue;
            fieldValue.field.name = sqlite3_column_name(stmt, i);

            switch (sqlite3_column_type(stmt, i)) {
                case SQLITE_INTEGER:
                    fieldValue.field.type = SqliteTable::DataType::INTEGER;
                    fieldValue.value = sqlite3_column_int(stmt, i);
                    break;
                case SQLITE_FLOAT:
                    fieldValue.field.type = SqliteTable::DataType::REAL;
                    fieldValue.value = sqlite3_column_double(stmt, i);
                    break;
                case SQLITE_TEXT:
                    fieldValue.field.type = SqliteTable::DataType::TEXT;
                    fieldValue.value = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
                    break;
                case SQLITE_BLOB:
                    fieldValue.field.type = SqliteTable::DataType::BLOB;
                    {
                        const void* blobData = sqlite3_column_blob(stmt, i);
                        int blobSize = sqlite3_column_bytes(stmt, i);
                        std::vector<unsigned char> blobVec(static_cast<const unsigned char*>(blobData),
                                                           static_cast<const unsigned char*>(blobData) + blobSize);
                        fieldValue.value = blobVec;
                    }
                    break;
                case SQLITE_NULL:
                    fieldValue.field.type = SqliteTable::DataType::TEXT;
                    fieldValue.value = std::string("NULL");
                    break;
            }
            row.push_back(fieldValue);
        }
        results.push_back(row);
    }

    sqlite3_finalize(stmt);
    return results;
}

void DatabaseManager::printResults(const std::vector<std::vector<SqliteTable::FieldValue>>& results) {
    for (const auto& row : results) {
        for (const auto& fieldValue : row) {
            std::cout << fieldValue.field.name << ": ";
            switch (fieldValue.field.type) {
                case SqliteTable::DataType::INTEGER:
                    std::cout << std::get<int>(fieldValue.value);
                    break;
                case SqliteTable::DataType::REAL:
                    std::cout << std::get<double>(fieldValue.value);
                    break;
                case SqliteTable::DataType::TEXT:
                    std::cout << std::get<std::string>(fieldValue.value);
                    break;
                case SqliteTable::DataType::BLOB:
                    {
                        const auto& blob = std::get<std::vector<unsigned char>>(fieldValue.value);
                        std::cout << "BLOB(" << blob.size() << " bytes)";
                    }
                    break;
            }
            std::cout << " | ";
        }
        std::cout << std::endl;
    }
}

DatabaseManager::~DatabaseManager() {
    std::unique_lock<std::mutex> lock(dbMutex_);
    if (db_) {
        sqlite3_close(db_);
    }
}