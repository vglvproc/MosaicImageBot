#pragma once
#include "SqliteTable.h"
#include "DatabaseManager.h"
#include <vector>

std::vector<SqliteTable> getInitialTables();

SqliteTable getUsersTable();

SqliteTable getLanguagesTable();

SqliteTable getCategoriesTable();

SqliteTable getCaptionsTable();

SqliteTable getCategoryPathsTable();

SqliteTable getSessionsTable();

SqliteTable getMessagesTable();

SqliteTable getNoAdsUsersTable();

SqliteTable getUnlimitedAccessUsersTable();

SqliteTable getTokensTable();

SqliteTable getRequestsTable();

bool initLanguagesTable(DatabaseManager& dbManager);