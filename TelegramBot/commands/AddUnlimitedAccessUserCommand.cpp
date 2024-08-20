#include "AddUnlimitedAccessUserCommand.h"

AddUnlimitedAccessUserCommand::AddUnlimitedAccessUserCommand() {}

AddUnlimitedAccessUserCommand::AddUnlimitedAccessUserCommand(const std::string& user_id)
    : user_id(user_id) {}

AddUnlimitedAccessUserCommand::AddUnlimitedAccessUserCommand(const std::string& user_id, DatabaseManager *dbManager)
    : user_id(user_id), dbManager(dbManager) {}

std::string AddUnlimitedAccessUserCommand::getUserId() const {
    return user_id;
}

void AddUnlimitedAccessUserCommand::setUserId(const std::string& user_id) {
    this->user_id = user_id;
}

void AddUnlimitedAccessUserCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool AddUnlimitedAccessUserCommand::executeCommand() {
    SqliteTable unlimitedAccessUsersTable = getUnlimitedAccessUsersTable();
    auto row = unlimitedAccessUsersTable.getEmptyRow();
    long long current_timestamp = getCurrentTimestamp();
    row[0].value = user_id;
    row[1].value = std::to_string(current_timestamp);
    row[2].value = getFormatTimestampWithMilliseconds(current_timestamp);

    std::string sqlCommand = unlimitedAccessUsersTable.generateInsertSQL(row, true);
    return dbManager->executeSQL(sqlCommand);
}