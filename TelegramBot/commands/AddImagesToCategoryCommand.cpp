#include <iostream>
#include "AddImagesToCategoryCommand.h"

AddImagesToCategoryCommand::AddImagesToCategoryCommand() {}

AddImagesToCategoryCommand::AddImagesToCategoryCommand(DatabaseManager *dbManager)
    : dbManager(dbManager) {}

AddImagesToCategoryCommand::AddImagesToCategoryCommand(const std::string& category_name)
    : category_name(category_name) {}

AddImagesToCategoryCommand::AddImagesToCategoryCommand(const std::string& category_name, DatabaseManager *dbManager)
    : category_name(category_name), dbManager(dbManager) {}

AddImagesToCategoryCommand::AddImagesToCategoryCommand(const std::string& category_name, const std::string& path_to_images)
    : category_name(category_name), path_to_images(path_to_images) {}

AddImagesToCategoryCommand::AddImagesToCategoryCommand(const std::string& category_name, const std::string& path_to_images, DatabaseManager *dbManager)
    : category_name(category_name), path_to_images(path_to_images), dbManager(dbManager) {}

std::string AddImagesToCategoryCommand::getCategoryName() const {
    return category_name;
}

std::string AddImagesToCategoryCommand::getPathToImages() const {
    return path_to_images;
}

void AddImagesToCategoryCommand::setCategoryName(const std::string& category_name) {
    this->category_name = category_name;
}

void AddImagesToCategoryCommand::setPathToImages(const std::string& path_to_images) {
    this->path_to_images = path_to_images;
}

void AddImagesToCategoryCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool AddImagesToCategoryCommand::executeCommand() {
    bool doCategoryExists = false;
    bool doPathExists = false;

    SqliteTable categoriesTable = getCategoriesTable();
    std::string sqlCommand = categoriesTable.generateSelectAllSQL();
    auto results_categories = dbManager->executeSelectSQL(sqlCommand);
    if (!results_categories.empty()) {
        for (const auto& row : results_categories) {
            std::string current_category = std::get<std::string>(row[0].value);
            if (current_category == this->category_name) {
                doCategoryExists = true;
                break;
            }
        }
    }

    if (!doCategoryExists) {
        std::cerr << "Category \"" << this->category_name << "\" does not exist in the database" << std::endl;
        return false;
    }

    std::vector<SqliteTable::FieldValue> whereRow;

    SqliteTable categoryPathsTable = getCategoryPathsTable();
    sqlCommand = categoryPathsTable.generateSelectAllSQL();
    auto results_paths = dbManager->executeSelectSQL(sqlCommand);
    if (!results_paths.empty()) {
        for (const auto& row : results_paths) {
            std::string current_path_id = std::get<std::string>(row[0].value);
            std::string current_category_name = std::get<std::string>(row[1].value);
            if (current_category_name == this->category_name) {
                doPathExists = true;
                whereRow.push_back(row[0]);
                whereRow.push_back(row[1]);
                break;
            }
        }
    }

    if (doPathExists) {
        std::vector<SqliteTable::FieldValue> categoryPathsRow = categoryPathsTable.getEmptyRow();
        categoryPathsRow[2].value = this->path_to_images;
        std::vector<SqliteTable::FieldValue> updateRow;
        updateRow.push_back(categoryPathsRow[2]);

        std::string updateSql = categoryPathsTable.generateUpdateSQL(updateRow, whereRow);
        if (dbManager->executeSQL(updateSql)) {
            std::cout << "Successfully updated path: \"" << this->path_to_images << "\" for category \"" << this->category_name
                      << "\"" << std::endl;
        } else {
            std::cout << "Failed to update path: \"" << this->path_to_images << "\" for category \"" << this->category_name
                      << "\"" << std::endl;
            return false;
        }
    } else {
        auto row = categoryPathsTable.getEmptyRow();
        std::string id = getRandomHexValue(32);
        long long current_timestamp = getCurrentTimestamp();
        row[0].value = id;
        row[1].value = category_name;
        row[2].value = this->path_to_images;
        row[3].value = std::to_string(current_timestamp);
        row[4].value = getFormatTimestampWithMilliseconds(current_timestamp);
        std::string sqlCommand = categoryPathsTable.generateInsertSQL(row, true);
        dbManager->executeSelectSQL(sqlCommand);
    }

    return true;
}