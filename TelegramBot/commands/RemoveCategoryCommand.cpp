#include "RemoveCategoryCommand.h"

RemoveCategoryCommand::RemoveCategoryCommand(const std::string& category_name)
    : category_name(category_name) {}

std::string RemoveCategoryCommand::getCategoryName() const {
    return category_name;
}

void RemoveCategoryCommand::setCategoryName(const std::string& category_name) {
    this->category_name = category_name;
}

void RemoveCategoryCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool RemoveCategoryCommand::executeCommand() {
    SqliteTable categoriesTable = getCategoriesTable();
    auto row = categoriesTable.getEmptyRow();
    row[0].value = category_name;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[0]);

    std::string sqlDeleteCommand = categoriesTable.generateDeleteSQL(whereRow);
    return dbManager->executeSQL(sqlDeleteCommand);
}