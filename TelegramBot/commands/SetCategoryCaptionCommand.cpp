#include <iostream>
#include "SetCategoryCaptionCommand.h"
#include "../utils/Utils.h"

SetCategoryCaptionCommand::SetCategoryCaptionCommand() {}

SetCategoryCaptionCommand::SetCategoryCaptionCommand(DatabaseManager *dbManager)
    : dbManager(dbManager) {}

SetCategoryCaptionCommand::SetCategoryCaptionCommand(const std::string& category_name)
    : category_name(category_name) {}

SetCategoryCaptionCommand::SetCategoryCaptionCommand(const std::string& category_name, DatabaseManager *dbManagers)
    : category_name(category_name), dbManager(dbManager) {}

SetCategoryCaptionCommand::SetCategoryCaptionCommand(const std::string& category_name, const std::string& caption)
    : category_name(category_name), caption(caption) {}

SetCategoryCaptionCommand::SetCategoryCaptionCommand(const std::string& category_name, const std::string& caption, DatabaseManager *dbManager)
    : category_name(category_name), caption(caption), dbManager(dbManager) {}

SetCategoryCaptionCommand::SetCategoryCaptionCommand(const std::string& category_name, const std::string& caption, int lang_id)
    : category_name(category_name), caption(caption), lang_id(lang_id) {}

SetCategoryCaptionCommand::SetCategoryCaptionCommand(const std::string& category_name, const std::string& caption, int lang_id, DatabaseManager *dbManager)
    : category_name(category_name), caption(caption), lang_id(lang_id), dbManager(dbManager) {}

void SetCategoryCaptionCommand::setCategoryName(const std::string& category_name) {
    this->category_name = category_name;
}

std::string SetCategoryCaptionCommand::getCategoryName() const {
    return this->category_name;
}

void SetCategoryCaptionCommand::setCaption(const std::string& caption) {
    this->caption = caption;
}

std::string SetCategoryCaptionCommand::getCaption() const {
    return this->caption;
}

void SetCategoryCaptionCommand::setLangId(int lang_id) {
    this->lang_id = lang_id;
}

int SetCategoryCaptionCommand::getLangId() const {
    return this->lang_id;
}

void SetCategoryCaptionCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool SetCategoryCaptionCommand::executeCommand() {
    bool doCategoryExists = false;
    bool doLangIdExists = false;
    bool doCaptionExists = false;

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

    SqliteTable langsTable = getLanguagesTable();
    sqlCommand = langsTable.generateSelectAllSQL();
    auto results_langs = dbManager->executeSelectSQL(sqlCommand);
    if (!results_langs.empty()) {
        for (const auto& row : results_langs) {
            int current_id = std::get<int>(row[0].value);
            if (current_id == this->lang_id) {
                doLangIdExists = true;
                break;
            }
        }
    }

    if (!doCategoryExists) {
        std::cerr << "Category \"" << this->category_name << "\" does not exist in the database" << std::endl;
    }

    if (!doLangIdExists) {
        std::cerr << "Language with id " << this->lang_id << " does not exist in the database" << std::endl;
    }

    if (!doCategoryExists || ! doLangIdExists) {
        return false;
    }

    std::vector<SqliteTable::FieldValue> whereRow;

    SqliteTable captionsTable = getCaptionsTable();
    sqlCommand = captionsTable.generateSelectAllSQL();
    auto results_captions = dbManager->executeSelectSQL(sqlCommand);
    if (!results_captions.empty()) {
        for (const auto& row : results_captions) {
            std::string current_caption_id = std::get<std::string>(row[0].value);
            std::string current_category_name = std::get<std::string>(row[1].value);
            int current_lang_id = std::get<int>(row[2].value);
            if (current_category_name == this->category_name && current_lang_id == this->lang_id) {
                doCaptionExists = true;
                whereRow.push_back(row[0]);
                whereRow.push_back(row[1]);
                whereRow.push_back(row[2]);
                break;
            }
        }
    }

    if (doCaptionExists) {
        std::vector<SqliteTable::FieldValue> captionsRow = captionsTable.getEmptyRow();
        captionsRow[3].value = this->caption;
        std::vector<SqliteTable::FieldValue> updateRow;
        updateRow.push_back(captionsRow[3]);

        std::string updateSql = captionsTable.generateUpdateSQL(updateRow, whereRow);
        if (dbManager->executeSQL(updateSql)) {
            std::cout << "Successfully updated caption: \"" << this->caption << "\" for category \"" << this->category_name
                      << "\" and language with ID " << this->lang_id << std::endl;
        } else {
            std::cout << "Failed to update caption: \"" << this->caption << "\" for category \"" << this->category_name
                      << "\" and language with ID " << this->lang_id << std::endl;
            return false;
        }
    } else {
        auto row = captionsTable.getEmptyRow();
        std::string caption_id = getRandomHexValue(32);
        long long current_timestamp = getCurrentTimestamp();
        row[0].value = caption_id;
        row[1].value = category_name;
        row[2].value = this->lang_id;
        row[3].value = this->caption;
        row[4].value = std::to_string(current_timestamp);
        row[5].value = getFormatTimestampWithMilliseconds(current_timestamp);
        std::string sqlCommand = captionsTable.generateInsertSQL(row, true);
        return dbManager->executeSQL(sqlCommand);
    }

    return true;
}