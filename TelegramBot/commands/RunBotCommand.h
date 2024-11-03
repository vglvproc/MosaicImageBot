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
    enum class PhotoProcessingStatus {
        STATUS_OK,
        STATUS_PHOTO_NOT_UPLOADED,
        STATUS_SESSION_NOT_FOUND,
        STATUS_CANNOT_CREATE_DIRECTORY,
        STATUS_CANNOT_DOWNLOAD_FILE,
        STATUS_CANNOT_DELETE_FILE,
        STATUS_CANNOT_ADD_REQUEST,
        STATUS_CANNOT_PROCESS_FILE,
        STATUS_UNEXPECTED_ERROR
    };

    RunBotCommand();
    RunBotCommand(DatabaseManager *dbManager);
    RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser);
    RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser, const std::string& userIdToDuplicate);
    RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser, const std::string& userIdToDuplicate, bool doSetCaption);
    RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser, const std::string& userIdToDuplicate, bool doSetCaption, const std::string& caption);
    void setDatabaseManager(DatabaseManager *dbManager);
    void setDuplicateDataToUser(bool value);
    void setUserIdToDuplicate(const std::string& value);
    bool getDoAddCaption();
    void setDoAddCaption(bool value);
    void setCaption(const std::string& value);
    bool executeCommand();
private:
    std::string getToken();
    void handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain); // TODO: убрать аргумент dbMain, так как функция сейчас является частью класса с доступом к соответствующему полю
    void handleButtonClicked(TgBot::Bot& bot, TgBot::CallbackQuery::Ptr query, DatabaseManager* dbMain);
    PhotoProcessingStatus handlePhotoUpload(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain);
    DatabaseManager *dbManager;
    bool duplicateDataToUser = false;
    bool doAddCaption = false;
    std::string userIdToDuplicate;
    std::string caption;
};