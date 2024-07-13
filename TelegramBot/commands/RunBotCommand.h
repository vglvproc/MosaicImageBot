#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../workflow/BotWorkflow.h"

class RunBotCommand : public Command {
public:
    RunBotCommand();
    RunBotCommand(DatabaseManager *dbManager);
    void setDatabaseManager(DatabaseManager *dbManager);
    bool executeCommand();
private:
    DatabaseManager *dbManager;
};