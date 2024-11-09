#include "RequestsManager.h"

RequestsManager::RequestsManager() {}

RequestsManager::RequestsManager(DatabaseManager *dbManager)
    : dbManager(dbManager) {}

void RequestsManager::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}