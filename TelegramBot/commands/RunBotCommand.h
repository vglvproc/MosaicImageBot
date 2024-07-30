#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../workflow/BotWorkflow.h"
#include <tgbot/tgbot.h>

class RunBotCommand : public Command {
public:
    RunBotCommand();
    RunBotCommand(DatabaseManager *dbManager);
    RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser);
    RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser, const std::string& userIdToDuplicate);
    void setDatabaseManager(DatabaseManager *dbManager);
    void setDuplicateDataToUser(bool value);
    void setUserIdToDuplicate(const std::string& value);
    bool executeCommand();
private:
    void handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain);
    void handleLanguageSelection(TgBot::Bot& bot, TgBot::CallbackQuery::Ptr query, DatabaseManager* dbMain);
    void handlePhotoUpload(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain);
    DatabaseManager *dbManager;
    bool duplicateDataToUser;
    std::string userIdToDuplicate;
};