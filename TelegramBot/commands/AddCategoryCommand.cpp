#include "AddCategoryCommand.h"

AddCategoryCommand::AddCategoryCommand(const std::string& category_name)
    : category_name(category_name) {}

AddCategoryCommand::AddCategoryCommand(const std::string& category_name, DatabaseManager *dbManager)
    : category_name(category_name), dbManager(dbManager) {}

std::string AddCategoryCommand::getCategoryName() const {
    return category_name;
}

void AddCategoryCommand::setCategoryName(const std::string& category_name) {
    this->category_name = category_name;
}

void AddCategoryCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool AddCategoryCommand::executeCommand() {
    SqliteTable categoriesTable = getCategoriesTable();
    auto row = categoriesTable.getEmptyRow();
    long long current_timestamp = getCurrentTimestamp();
    row[0].value = category_name;
    row[1].value = std::to_string(current_timestamp);
    row[2].value = getFormatTimestampWithMilliseconds(current_timestamp);

    std::string sqlCommand = categoriesTable.generateInsertSQL(row, true);
    return dbManager->executeSQL(sqlCommand);
}