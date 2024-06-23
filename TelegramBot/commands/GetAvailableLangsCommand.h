#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"

class GetAvailableLangsCommand : public Command {
public:
    GetAvailableLangsCommand();
    GetAvailableLangsCommand(DatabaseManager *dbManager);
    bool executeCommand();
    void setDatabaseManager(DatabaseManager *dbManager);
private:
    DatabaseManager *dbManager;
};