#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class AddCategoryCommand : public Command {
public:
    AddCategoryCommand(const std::string& category_name);
    AddCategoryCommand(const std::string& category_name, DatabaseManager *dbManager);
    std::string getCategoryName() const;
    void setCategoryName(const std::string& category_name);
    void setDatabaseManager(DatabaseManager *dbManager);
    bool executeCommand();

private:
    std::string category_name;
    DatabaseManager *dbManager;
};