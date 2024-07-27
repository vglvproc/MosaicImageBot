#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class AddUnlimitedAccessUserCommand : public Command {
public:
    AddUnlimitedAccessUserCommand();
    AddUnlimitedAccessUserCommand(const std::string& user_id);
    AddUnlimitedAccessUserCommand(const std::string& user_id, DatabaseManager *dbManager);
    std::string getUserId() const;
    void setUserId(const std::string& user_id);
    void setDatabaseManager(DatabaseManager *dbManager);
    bool executeCommand();

private:
    std::string user_id;
    DatabaseManager *dbManager;
};