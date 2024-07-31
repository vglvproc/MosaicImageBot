#include "SetTokenCommand.h"

SetTokenCommand::SetTokenCommand() {}

SetTokenCommand::SetTokenCommand(const std::string& token)
    : token(token) {}

SetTokenCommand::SetTokenCommand(const std::string& token, DatabaseManager *dbManager)
    : token(token), dbManager(dbManager) {}

std::string SetTokenCommand::getToken() const {
    return token;
}

void SetTokenCommand::setToken(const std::string& token) {
    this->token = token;
}

void SetTokenCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool SetTokenCommand::executeCommand() {
    SqliteTable tokensTable = getTokensTable();
    auto row = tokensTable.getEmptyRow();
    long long current_timestamp = getCurrentTimestamp();
    std::string tokenId = getRandomHexValue(32);
    row[0].value = tokenId;
    row[1].value = token;
    row[2].value = std::to_string(current_timestamp);
    row[3].value = getFormatTimestampWithMilliseconds(current_timestamp);

    std::string sqlCommand = tokensTable.generateInsertSQL(row, true);
    dbManager->executeSelectSQL(sqlCommand);
    return true;
}