#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class SetTokenCommand : public Command {
public:
    SetTokenCommand();
    SetTokenCommand(const std::string& token);
    SetTokenCommand(const std::string& token, DatabaseManager *dbManager);
    std::string getToken() const;
    void setToken(const std::string& token);
    void setDatabaseManager(DatabaseManager *dbManager);
    bool executeCommand();

private:
    std::string token;
    DatabaseManager *dbManager;
};