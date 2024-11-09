#pragma once
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class RequestsManager {
public:
    RequestsManager();
    RequestsManager(DatabaseManager *dbManager);
    void setDatabaseManager(DatabaseManager *dbManager);
private:
    DatabaseManager *dbManager;
};