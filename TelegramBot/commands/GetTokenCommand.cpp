#include "GetTokenCommand.h"

GetTokenCommand::GetTokenCommand() {}

GetTokenCommand::GetTokenCommand(DatabaseManager *dbManager)
    : dbManager(dbManager) {}

std::string GetTokenCommand::getToken() const {
    return token;
}

void GetTokenCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

std::string GetTokenCommand::executeCommand(bool* result) {
    *result = true;
    std::string tokenFromDB("");
    this->token = tokenFromDB;
    SqliteTable tokensTable = getTokensTable();

    std::string sqlCommand = tokensTable.generateSelectAllSQL() + " ORDER BY adding_timestamp ASC";
    std::vector<std::vector<SqliteTable::FieldValue>> results = dbManager->executeSelectSQL(sqlCommand);
    if (!results.empty()) {
        auto row = results[results.size() - 1];
        tokenFromDB = std::get<std::string>(row[1].value);
        this->token = tokenFromDB;
    }
    return tokenFromDB;
}