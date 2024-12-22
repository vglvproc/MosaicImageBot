#pragma once
#include "../core/Subject.h"
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"
#include "../workflow/BotWorkflow.h"

class RequestsManager : public Subject {
public:
    RequestsManager();
    RequestsManager(DatabaseManager *dbManager);
    void setDatabaseManager(DatabaseManager *dbManager);
    void process();
private:
    DatabaseManager *dbManager;
};