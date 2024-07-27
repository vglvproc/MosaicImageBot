#pragma once
#include "SqliteTable.h"
#include "DatabaseManager.h"
#include <vector>

std::vector<SqliteTable> getInitialTables();

SqliteTable getUsersTable();

SqliteTable getLanguagesTable();

SqliteTable getCategoriesTable();

SqliteTable getSessionsTable();

SqliteTable getMessagesTable();

SqliteTable getNoAdsUsersTable();

SqliteTable getUnlimitedAccessUsersTable();

bool initLanguagesTable(DatabaseManager& dbManager);