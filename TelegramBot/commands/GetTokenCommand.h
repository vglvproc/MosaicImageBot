#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class GetTokenCommand : public Command {
public:
    GetTokenCommand();
    GetTokenCommand(DatabaseManager *dbManager);
    void setDatabaseManager(DatabaseManager *dbManager);
    std::string getToken() const;
    std::string executeCommand(bool* result);

private:
    std::string token;
    DatabaseManager *dbManager;
};