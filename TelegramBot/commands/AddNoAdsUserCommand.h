#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class AddNoAdsUserCommand : public Command {
public:
    AddNoAdsUserCommand();
    AddNoAdsUserCommand(const std::string& user_id);
    AddNoAdsUserCommand(const std::string& user_id, DatabaseManager *dbManager);
    std::string getUserId() const;
    void setUserId(const std::string& user_id);
    void setDatabaseManager(DatabaseManager *dbManager);
    bool executeCommand();

private:
    std::string user_id;
    DatabaseManager *dbManager;
};