#include "AddNoAdsUserCommand.h"

AddNoAdsUserCommand::AddNoAdsUserCommand() {}

AddNoAdsUserCommand::AddNoAdsUserCommand(const std::string& user_id)
    : user_id(user_id) {}

AddNoAdsUserCommand::AddNoAdsUserCommand(const std::string& user_id, DatabaseManager *dbManager)
    : user_id(user_id), dbManager(dbManager) {}

std::string AddNoAdsUserCommand::getUserId() const {
    return user_id;
}

void AddNoAdsUserCommand::setUserId(const std::string& user_id) {
    this->user_id = user_id;
}

void AddNoAdsUserCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool AddNoAdsUserCommand::executeCommand() {
    SqliteTable noAdsUsersTable = getNoAdsUsersTable();
    auto row = noAdsUsersTable.getEmptyRow();
    long long current_timestamp = getCurrentTimestamp();
    row[0].value = user_id;
    row[1].value = std::to_string(current_timestamp);
    row[2].value = getFormatTimestampWithMilliseconds(current_timestamp);

    std::string sqlCommand = noAdsUsersTable.generateInsertSQL(row, true);
    return dbManager->executeSQL(sqlCommand);
}