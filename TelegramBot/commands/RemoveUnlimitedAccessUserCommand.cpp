#include "RemoveUnlimitedAccessUserCommand.h"

RemoveUnlimitedAccessUserCommand::RemoveUnlimitedAccessUserCommand() {}

RemoveUnlimitedAccessUserCommand::RemoveUnlimitedAccessUserCommand(const std::string& user_id)
    : user_id(user_id) {}

RemoveUnlimitedAccessUserCommand::RemoveUnlimitedAccessUserCommand(const std::string& user_id, DatabaseManager *dbManager)
    : user_id(user_id), dbManager(dbManager) {}

std::string RemoveUnlimitedAccessUserCommand::getUserId() const {
    return user_id;
}

void RemoveUnlimitedAccessUserCommand::setUserId(const std::string& user_id) {
    this->user_id = user_id;
}

void RemoveUnlimitedAccessUserCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool RemoveUnlimitedAccessUserCommand::executeCommand() {
    SqliteTable unlimitedAccessUsersTable = getUnlimitedAccessUsersTable();
    auto row = unlimitedAccessUsersTable.getEmptyRow();
    row[0].value = user_id;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[0]);

    std::string sqlDeleteCommand = unlimitedAccessUsersTable.generateDeleteSQL(whereRow);
    return dbManager->executeSQL(sqlDeleteCommand);
}