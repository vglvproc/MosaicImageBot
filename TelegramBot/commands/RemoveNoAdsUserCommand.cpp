#include "RemoveNoAdsUserCommand.h"

RemoveNoAdsUserCommand::RemoveNoAdsUserCommand() {}

RemoveNoAdsUserCommand::RemoveNoAdsUserCommand(const std::string& user_id)
    : user_id(user_id) {}

RemoveNoAdsUserCommand::RemoveNoAdsUserCommand(const std::string& user_id, DatabaseManager *dbManager)
    : user_id(user_id), dbManager(dbManager) {}

std::string RemoveNoAdsUserCommand::getUserId() const {
    return user_id;
}

void RemoveNoAdsUserCommand::setUserId(const std::string& user_id) {
    this->user_id = user_id;
}

void RemoveNoAdsUserCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool RemoveNoAdsUserCommand::executeCommand() {
    SqliteTable noAdsUsersTable = getNoAdsUsersTable();
    auto row = noAdsUsersTable.getEmptyRow();
    row[0].value = user_id;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[0]);

    std::string sqlDeleteCommand = noAdsUsersTable.generateDeleteSQL(whereRow);
    return dbManager->executeSQL(sqlDeleteCommand);
}