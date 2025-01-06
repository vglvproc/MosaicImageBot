#include <iostream>
#include "RequestsManager.h"

RequestsManager::RequestsManager() {}

RequestsManager::RequestsManager(DatabaseManager *dbManager)
    : dbManager(dbManager) {}

void RequestsManager::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

void RequestsManager::process() {
    SqliteTable requestsTable = getRequestsTable();
    std::vector<SqliteTable::FieldValue> row = requestsTable.getEmptyRow();
    row[4].value = (int)BotWorkflow::RequestStep::REQUEST_STEP_WAITING;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[4]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::vector<SqliteTable::FieldValue> orderRow;
    orderRow.push_back(row[7]);
    std::string selectSql = requestsTable.generateSelectSQL(emptyRow, whereRow, orderRow, 1);
    auto results = dbManager->executeSelectSQL(selectSql);
    if (!results.empty()) {
        auto res_row = results[0];
        std::string command = std::get<std::string>(res_row[2].value);
        std::string imagePath = std::get<std::string>(res_row[3].value);
        notifyListeners(res_row);
    }

    std::string selectErrorSql = "SELECT * FROM requests WHERE request_step BETWEEN 2 AND 5 ORDER BY last_access_timestamp LIMIT 1;";

    auto error_results = dbManager->executeSelectSQL(selectErrorSql);
    if (!error_results.empty()) {
        auto res_row = results[0];
        std::string command = std::get<std::string>(res_row[2].value);
        std::string imagePath = std::get<std::string>(res_row[3].value);
        std::cout << "Error result found! " << command << " " << imagePath << std::endl;
    }
}