#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class RemoveCategoryCommand : public Command {
public:
    RemoveCategoryCommand(const std::string& category_name);
    std::string getCategoryName() const;
    void setDatabaseManager(DatabaseManager *dbManager);
    void setCategoryName(const std::string& category_name);
    bool executeCommand();

private:
    std::string category_name;
    DatabaseManager *dbManager;
};