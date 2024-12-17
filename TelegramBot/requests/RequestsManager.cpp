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
    row[3].value = (int)BotWorkflow::RequestStep::REQUEST_STEP_WAITING;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[3]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::vector<SqliteTable::FieldValue> orderRow;
    orderRow.push_back(row[6]);
    std::string selectSql = requestsTable.generateSelectSQL(emptyRow, whereRow, orderRow, 1);
    auto results = dbManager->executeSelectSQL(selectSql);
    if (!results.empty()) {
        auto res_row = results[0];
    }
}