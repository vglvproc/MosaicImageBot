#include <iostream>
#include <filesystem>
#include "RunBotCommand.h"
#include "../utils/Utils.h"
#include "../workflow/BotWorkflow.h"

std::string getMessageByTypeAndLang(DatabaseManager* dbMain, BotWorkflow::WorkflowMessage msg_type, int lang_id, bool* msg_found) {
    SqliteTable messagesTable = getMessagesTable();
    std::vector<SqliteTable::FieldValue> row = messagesTable.getEmptyRow();
    row[1].value = (int)msg_type;
    row[2].value = lang_id;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[1]);
    whereRow.push_back(row[2]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::string selectSql = messagesTable.generateSelectSQL(emptyRow, whereRow);
    auto results = dbMain->executeSelectSQL(selectSql);
    if (!results.empty()) {
        *msg_found = true;
        auto row = results[0];
        return std::get<std::string>(row[3].value);
    } else {
        *msg_found = false;
        return std::string("");
    }
}

std::pair<std::string, std::string> getIdAndCaptionByCategoryAndLang(DatabaseManager* dbMain, const std::string& category_name, int lang_id, bool* caption_found) {
    SqliteTable captionsTable = getCaptionsTable();
    std::vector<SqliteTable::FieldValue> row = captionsTable.getEmptyRow();
    row[1].value = category_name;
    row[2].value = lang_id;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[1]);
    whereRow.push_back(row[2]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::string selectSql = captionsTable.generateSelectSQL(emptyRow, whereRow);
    auto results = dbMain->executeSelectSQL(selectSql);
    if (!results.empty()) {
        *caption_found = true;
        auto row = results[0];
        return std::make_pair(std::get<std::string>(row[0].value), std::get<std::string>(row[3].value));
    } else {
        *caption_found = false;
        return std::make_pair(std::string(""), std::string(""));
    }
}

bool checkNoAdsForUser(DatabaseManager* dbMain, const std::string& userId) {
    SqliteTable noAdsUsersTable = getNoAdsUsersTable();
    std::vector<SqliteTable::FieldValue> row = noAdsUsersTable.getEmptyRow();
    row[0].value = userId;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[0]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::string selectSql = noAdsUsersTable.generateSelectSQL(emptyRow, whereRow);
    std::vector<std::vector<SqliteTable::FieldValue>> results = dbMain->executeSelectSQL(selectSql);
    if (!results.empty()) {
        return true;
    }

    return false;
}

bool checkUnlimitedAccessForUser(DatabaseManager* dbMain, const std::string& userId) {
    SqliteTable unlimitedAccessUsersTable = getUnlimitedAccessUsersTable();
    std::vector<SqliteTable::FieldValue> row = unlimitedAccessUsersTable.getEmptyRow();
    row[0].value = userId;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(row[0]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::string selectSql = unlimitedAccessUsersTable.generateSelectSQL(emptyRow, whereRow);
    std::vector<std::vector<SqliteTable::FieldValue>> results = dbMain->executeSelectSQL(selectSql);
    if (!results.empty()) {
        return true;
    }

    return false;
}

std::string generateCommandForMetapixel(const std::string& imagePath, const std::string libraryPath, int size, bool antimosaic, const std::string antimosaicPath) {
    std::filesystem::path inputPath(imagePath);
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + "_mosaic" + inputPath.extension().string());

    std::string command = "metapixel --metapixel " + imagePath + " " + outputPath.string() + " --width=" + std::to_string(size)
                        + " --height=" + std::to_string(size);  + " --library=" + libraryPath;
    if (antimosaic) {
        if (antimosaicPath != "") {
            command = command + " --antimosaic=\"" + antimosaicPath + "\"";
        } else {
            command = command + " --antimosaic=\"" + imagePath + "\"";
        }
    } else {
        command = command + + " --library=\"" + libraryPath + "\"";
    }

    return command;
}

bool addRequestToQueue(DatabaseManager* dbMain, const std::string& session_id, const std::string& imagePath, const std::string& command) {
    std::filesystem::path inputPath(imagePath);
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + "_mosaic" + inputPath.extension().string());

    SqliteTable requestsTable = getRequestsTable();
    std::string request_id = getRandomHexValue(32);
    auto row = requestsTable.getEmptyRow();
    long long current_timestamp = getCurrentTimestamp();
    row[0].value = request_id;
    row[1].value = session_id;
    row[2].value = command;
    row[3].value = imagePath;
    row[4].value = (int)BotWorkflow::RequestStep::REQUEST_STEP_WAITING;
    row[5].value = std::to_string(current_timestamp);
    row[6].value = getFormatTimestampWithMilliseconds(current_timestamp);
    row[7].value = std::to_string(current_timestamp);
    row[8].value = getFormatTimestampWithMilliseconds(current_timestamp);

    std::string sqlCommand = requestsTable.generateInsertSQL(row, true);
    std::cout << sqlCommand << std::endl;
    return dbMain->executeSQL(sqlCommand);
}

bool processImageWithMetapixel(const std::string& imagePath, const std::string& command) {
    std::filesystem::path inputPath(imagePath);
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + "_mosaic" + inputPath.extension().string());

    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "Image processed successfully: " << outputPath.string() << std::endl;
        return true;
    } else {
        std::cerr << "Failed to process image with metapixel." << std::endl;
        return false;
    }
}

bool addCaption(const std::string& imagePath, const std::string& caption) {
    std::filesystem::path inputPath(imagePath);
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + "_caption" + inputPath.extension().string());

    std::string command = "ffmpeg -i " + imagePath + " -vf \"drawtext=text='" + caption
                        + "':fontcolor=black:fontsize=24:box=1:boxcolor=white@0.5:boxborderw=5:x=10:y=h-th-10\" "
                        + outputPath.string();

    std::cout << "Executing command: " << command << std::endl;

    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "Caption added successfully: " << outputPath.string() << std::endl;
        return true;
    } else {
        std::cerr << "Failed to add caption to the image." << std::endl;
        return false;
    }
}

long long getUserIdBySessionId(DatabaseManager* dbMain, const std::string& sessionId) {
    SqliteTable sessionsTable = getSessionsTable();
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[0].value = sessionId;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(sessionsRow[0]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::string selectSql = sessionsTable.generateSelectSQL(emptyRow, whereRow);
    std::vector<std::vector<SqliteTable::FieldValue>> results = dbMain->executeSelectSQL(selectSql);
    if (!results.empty()) {
        auto row = results[0];
        std::string userId = std::get<std::string>(row[1].value);
        long long res = 0;
        try {
            res = std::stoll(userId);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return -1;
        } catch (const std::out_of_range& e) {
            std::cerr << "Out of range: " << e.what() << std::endl;
            return -1;
        }
        return res;
    }

    return -1;
}

std::string getSessionIdAfterPhotoUpload(DatabaseManager* dbMain, long long userId) {
    SqliteTable sessionsTable = getSessionsTable();
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[1].value = std::to_string(userId);
    sessionsRow[2].value = (int)BotWorkflow::WorkflowMessage::STEP_ADD_PHOTO_MESSAGE;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(sessionsRow[1]);
    whereRow.push_back(sessionsRow[2]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::string selectSql = sessionsTable.generateSelectSQL(emptyRow, whereRow);
    std::vector<std::vector<SqliteTable::FieldValue>> results = dbMain->executeSelectSQL(selectSql);
    if (!results.empty()) {
        auto row = results[results.size() - 1];
        return std::get<std::string>(row[0].value);
    }

    return "";
}

std::string getSessionIdAfterAntimosaicPhotoUpload(DatabaseManager* dbMain, long long userId) {
    SqliteTable sessionsTable = getSessionsTable();
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[1].value = std::to_string(userId);
    sessionsRow[2].value = (int)BotWorkflow::WorkflowMessage::STEP_ADD_ANTIMOSAIC_PHOTO;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(sessionsRow[1]);
    whereRow.push_back(sessionsRow[2]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::string selectSql = sessionsTable.generateSelectSQL(emptyRow, whereRow);
    std::vector<std::vector<SqliteTable::FieldValue>> results = dbMain->executeSelectSQL(selectSql);
    if (!results.empty()) {
        auto row = results[results.size() - 1];
        return std::get<std::string>(row[0].value);
    }

    return "";
}

int getCurrentLangIndexByStepAndUserId(DatabaseManager* dbMain, BotWorkflow::WorkflowMessage step, long long userId) {
    SqliteTable sessionsTable = getSessionsTable();
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[1].value = std::to_string(userId);
    sessionsRow[2].value = (int)step;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(sessionsRow[1]);
    whereRow.push_back(sessionsRow[2]);
    std::vector<SqliteTable::FieldValue> emptyRow;
    std::string selectSql = sessionsTable.generateSelectSQL(emptyRow, whereRow);
    std::vector<std::vector<SqliteTable::FieldValue>> results = dbMain->executeSelectSQL(selectSql);
    if (!results.empty()) {
        auto row = results[results.size() - 1];
        return std::get<int>(row[3].value);
    }

    return -1;
}

void updateSessionStep(DatabaseManager* dbMain, const std::string& sessionId, BotWorkflow::WorkflowMessage step) {
    SqliteTable sessionsTable = getSessionsTable();

    // Create update row
    BotWorkflow::WorkflowMessage currentStep = step;
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[2].value = (int)currentStep;
    std::vector<SqliteTable::FieldValue> updateRow;
    updateRow.push_back(sessionsRow[2]);

    // Create where clause row
    sessionsRow[0].value = sessionId;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(sessionsRow[0]);

    std::string updateSql = sessionsTable.generateUpdateSQL(updateRow, whereRow);
    std::cout << "UPDATE SQL: " << updateSql << std::endl;
    if (dbMain->executeSQL(updateSql)) {
        std::cout << "Successfully updated sessionId: " << sessionId << std::endl;
    } else {
        std::cerr << "Failed to update session: " << sessionId << std::endl;
    }
}

void updateSessionLanguage(DatabaseManager* dbMain, const std::string& sessionId, int languageIndex) {
    SqliteTable sessionsTable = getSessionsTable();

    // Create update row
    BotWorkflow::WorkflowMessage currentStep = BotWorkflow::WorkflowMessage::STEP_SELECT_THEME_MESSAGE;
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[2].value = (int)currentStep;
    sessionsRow[3].value = languageIndex;
    std::vector<SqliteTable::FieldValue> updateRow;
    updateRow.push_back(sessionsRow[2]);
    updateRow.push_back(sessionsRow[3]);

    // Create where clause row
    sessionsRow[0].value = sessionId;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(sessionsRow[0]);

    std::string updateSql = sessionsTable.generateUpdateSQL(updateRow, whereRow);
    std::cout << "UPDATE SQL: " << updateSql << std::endl;
    if (dbMain->executeSQL(updateSql)) {
        std::cout << "Successfully updated sessionId: " << sessionId << std::endl;
    } else {
        std::cerr << "Failed to update session: " << sessionId << std::endl;
    }
}

void updateSessionTheme(DatabaseManager* dbMain, const std::string& sessionId, int themeIndex) {
    SqliteTable sessionsTable = getSessionsTable();

    BotWorkflow::WorkflowMessage currentStep = BotWorkflow::WorkflowMessage::STEP_SELECT_SIZE;
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[2].value = (int)currentStep;
    sessionsRow[4].value = themeIndex;
    std::vector<SqliteTable::FieldValue> updateRow;
    updateRow.push_back(sessionsRow[2]);
    updateRow.push_back(sessionsRow[4]);

    // Create where clause row
    sessionsRow[0].value = sessionId;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(sessionsRow[0]);

    std::string updateSql = sessionsTable.generateUpdateSQL(updateRow, whereRow);
    std::cout << "UPDATE SQL: " << updateSql << std::endl;
    if (dbMain->executeSQL(updateSql)) {
        std::cout << "Successfully updated sessionId: " << sessionId << std::endl;
    } else {
        std::cerr << "Failed to update session: " << sessionId << std::endl;
    }
}

void updateSessionSize(DatabaseManager* dbMain, const std::string& sessionId, int size) {
    SqliteTable sessionsTable = getSessionsTable();

    BotWorkflow::WorkflowMessage currentStep = BotWorkflow::WorkflowMessage::STEP_ADD_PHOTO_MESSAGE;
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[2].value = (int)currentStep;
    sessionsRow[5].value = size;
    std::vector<SqliteTable::FieldValue> updateRow;
    updateRow.push_back(sessionsRow[2]);
    updateRow.push_back(sessionsRow[5]);

    // Create where clause row
    sessionsRow[0].value = sessionId;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(sessionsRow[0]);

    std::string updateSql = sessionsTable.generateUpdateSQL(updateRow, whereRow);
    std::cout << "UPDATE SQL: " << updateSql << std::endl;
    if (dbMain->executeSQL(updateSql)) {
        std::cout << "Successfully updated sessionId: " << sessionId << std::endl;
    } else {
        std::cerr << "Failed to update session: " << sessionId << std::endl;
    }
}

RunBotCommand::RunBotCommand()
    : duplicateDataToUser(false), userIdToDuplicate(std::string("")) {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager)
    : dbManager(dbManager), duplicateDataToUser(false), userIdToDuplicate(std::string("")) {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser)
    : dbManager(dbManager), duplicateDataToUser(duplicateDataToUser), userIdToDuplicate(std::string("")) {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser, const std::string& userIdToDuplicate)
    : dbManager(dbManager), duplicateDataToUser(duplicateDataToUser), userIdToDuplicate(userIdToDuplicate) {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser, const std::string& userIdToDuplicate, bool doAddCaption)
    : dbManager(dbManager), duplicateDataToUser(duplicateDataToUser), userIdToDuplicate(userIdToDuplicate), doAddCaption(doAddCaption) {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser, const std::string& userIdToDuplicate, bool doAddCaption, const std::string& caption)
    : dbManager(dbManager), duplicateDataToUser(duplicateDataToUser), userIdToDuplicate(userIdToDuplicate), doAddCaption(doAddCaption), caption(caption) {}

void RunBotCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

void RunBotCommand::setDuplicateDataToUser(bool value) {
    this->duplicateDataToUser = value;
}

void RunBotCommand::setUserIdToDuplicate(const std::string& value) {
    this->userIdToDuplicate = value;
}

bool RunBotCommand::getDoAddCaption() {
    return this->doAddCaption;
}

void RunBotCommand::setDoAddCaption(bool value) {
    this->doAddCaption = value;
}

void RunBotCommand::setCaption(const std::string& value) {
    this->caption = value;
}

bool RunBotCommand::executeCommand() {
    std::cout << "Running MosaicImageBot..." << std::endl;
    std::string token = getToken();

    if (token.length() == 0) {
        std::cerr << "You should set a token for the bot!" << std::endl;
        return false;
    }

    bot = std::make_unique<TgBot::Bot>(token.c_str());

    bot->getEvents().onCommand("start", [this](TgBot::Message::Ptr message) {
        handleStartCommand(*bot, message, this->dbManager);
    });

    bot->getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr query) {
        handleButtonClicked(*bot, query, this->dbManager);
    });

    bot->getEvents().onAnyMessage([this](TgBot::Message::Ptr message) {
        if (message->photo.size() > 0) {
            PhotoProcessingStatus retStatus = handlePhotoUpload(*bot, message, this->dbManager);
            if (retStatus == PhotoProcessingStatus::STATUS_SESSION_NOT_FOUND) {
                bot->getApi().sendMessage(message->chat->id, "Session not found. Please start again using /start.");
            } else if (retStatus == PhotoProcessingStatus::STATUS_PHOTO_NOT_UPLOADED) {
                int langIndex = getCurrentLangIndexByStepAndUserId(dbManager, BotWorkflow::WorkflowMessage::STEP_ADD_PHOTO_MESSAGE, message->from->id);
                std::string errorMessage = "Photo is empty. Please uploaded a photo.";
                if (langIndex != -1) {
                    bool getMessage = false;
                    std::string tempMessage = getMessageByTypeAndLang(dbManager, BotWorkflow::WorkflowMessage::ERROR_PHOTO_NOT_UPLOADED, langIndex, &getMessage);
                    if (getMessage) {
                        errorMessage = tempMessage;
                    }
                }
                bot->getApi().sendMessage(message->chat->id, errorMessage);
            } else if (retStatus == PhotoProcessingStatus::STATUS_CANNOT_CREATE_DIRECTORY || retStatus == PhotoProcessingStatus::STATUS_CANNOT_DOWNLOAD_FILE || retStatus == PhotoProcessingStatus::STATUS_CANNOT_PROCESS_FILE) {
                int langIndex = getCurrentLangIndexByStepAndUserId(dbManager, BotWorkflow::WorkflowMessage::STEP_ADD_PHOTO_MESSAGE, message->from->id);
                std::string errorMessage = "An error occured while trying process the photo. Please try again later...";
                if (langIndex != -1) {
                    bool getMessage = false;
                    std::string tempMessage = getMessageByTypeAndLang(dbManager, BotWorkflow::WorkflowMessage::ERROR_PHOTO_PROCESSING_FAILED, langIndex, &getMessage);
                    if (getMessage) {
                        errorMessage = tempMessage;
                    }
                }
                bot->getApi().sendMessage(message->chat->id, errorMessage);
            }
        }
    });

    try {
        printf("Bot username: %s\n", bot->getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(*bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return true;
}

std::string RunBotCommand::getToken() {
    std::string token("");
    SqliteTable tokensTable = getTokensTable();
    std::string sqlCommand = tokensTable.generateSelectAllSQL() + " ORDER BY adding_timestamp ASC";
    std::vector<std::vector<SqliteTable::FieldValue>> results = dbManager->executeSelectSQL(sqlCommand);
    if (!results.empty()) {
        auto row = results[results.size() - 1];
        token = std::get<std::string>(row[1].value);
    }

    return token;
}

void RunBotCommand::handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain) {
    // Create session
    // TODO: Need to wrap into while cycle, in the case (which should occurs extremely rarely) if sessionId already exists in the table
    // Or find the another way to guarantee uniqueness
    SqliteTable sessionsTable = getSessionsTable();
    std::vector<SqliteTable::FieldValue> row = sessionsTable.getEmptyRow();
    std::string sessionId = getRandomHexValue(32);
    long long userId = message->from->id;
    long long timestamp = getCurrentTimestamp();
    BotWorkflow::WorkflowMessage currentStep = BotWorkflow::WorkflowMessage::STEP_SELECT_LANGUAGE;

    row[0].value = sessionId;
    row[1].value = std::to_string(userId);
    row[2].value = (int)currentStep;
    row[3].value = -1;
    row[4].value = -1;
    row[5].value = -1;
    row[6].value = std::to_string(timestamp);
    row[7].value = getFormatTimestampWithMilliseconds(timestamp);

    std::string insertSql = sessionsTable.generateInsertSQL(row, true);
    printf("INSERT SQL: %s\n", insertSql.c_str());
    if (dbMain->executeSQL(insertSql)) {
        std::cout << "Successfully inserted sessionId: " << sessionId << std::endl;
    } else {
        std::cerr << "Failed to insert session: " << sessionId << std::endl;
    }

    // Show the languages buttons
    SqliteTable langsTable = getLanguagesTable();
    std::string sqlCommand = langsTable.generateSelectAllSQL();
    auto results = dbMain->executeSelectSQL(sqlCommand);
    if (!results.empty()) {
        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup());
        int index = 0;
        for (const auto& row : results) {
            std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton;
            TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton());
            int langIndex = std::get<int>(row[0].value);
            button->text = std::get<std::string>(row[1].value) + " " + std::get<std::string>(row[2].value);
            button->callbackData = "lang_" + sessionId + "_" + std::to_string(langIndex);
            rowButton.push_back(button);
            keyboard->inlineKeyboard.push_back(rowButton);
        }
        bot.getApi().sendMessage(message->chat->id, "Please select the language:", nullptr, nullptr, keyboard);
    } else {
        std::cout << "Languages list is empty." << std::endl; // TODO: Need to be handled
    }
}

void RunBotCommand::handleButtonClicked(TgBot::Bot& bot, TgBot::CallbackQuery::Ptr query, DatabaseManager* dbMain) {
    std::string data = query->data;
    if (data.rfind("lang_", 0) == 0) {
        size_t firstUnderscore = data.find('_');
        size_t secondUnderscore = data.find('_', firstUnderscore + 1);

        std::string sessionId = data.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
        int langIndex = std::stoi(data.substr(secondUnderscore + 1));

        updateSessionLanguage(dbMain, sessionId, langIndex);

        bool getMessage = false;
        std::string selectThemeMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::STEP_SELECT_THEME_MESSAGE, langIndex, &getMessage);
        if (!getMessage) {
            selectThemeMessage = "Please select the theme of the images that will be used to create the mosaic";
        }

        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup());
        SqliteTable categoriesTable = getCategoriesTable();
        std::string sqlCommand = categoriesTable.generateSelectAllSQL();
        auto results = dbMain->executeSelectSQL(sqlCommand);
        if (!results.empty()) {
            int index = 1;
            for (const auto& row : results) {
                std::string category = std::get<std::string>(row[0].value);
                std::string caption_id("");
                std::string caption("");
                bool caption_found = false;
                std::pair<std::string, std::string> result = getIdAndCaptionByCategoryAndLang(dbMain, category, langIndex, &caption_found);
                if (!caption_found) {
                    caption_id = std::to_string(index);
                    caption = category;
                } else {
                    caption_id = result.first;
                    caption = result.second;
                }
                std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton;
                TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton());
                button->text = caption;
                button->callbackData = "theme_" + sessionId + "_" + std::to_string(langIndex) + "_" + std::to_string(index);
                rowButton.push_back(button);
                keyboard->inlineKeyboard.push_back(rowButton);
                index++;
            }
            std::string antimosaic_caption = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::CAPTION_ANTI_MOSAIC, langIndex, &getMessage);
            if (!getMessage) {
                antimosaic_caption = "Anti-mosaic";
            }
            std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton;
            TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton());
            button->text = antimosaic_caption;
            button->callbackData = "antimosaic_" + sessionId + "_" + std::to_string(langIndex) + "_0";
            rowButton.push_back(button);
            keyboard->inlineKeyboard.push_back(rowButton);
            bot.getApi().sendMessage(query->message->chat->id, selectThemeMessage, nullptr, nullptr, keyboard);
        } else {
            std::cout << "Captions list is empty." << std::endl; // TODO: Need to be handled
        }
    } else if (data.rfind("antimosaic_", 0) == 0) {
        size_t firstUnderscore = data.find('_');
        size_t secondUnderscore = data.find('_', firstUnderscore + 1);
        size_t thirdUnderscore = data.find('_', secondUnderscore + 1);

        std::string sessionId = data.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
        int langIndex = std::stoi(data.substr(secondUnderscore + 1, thirdUnderscore - secondUnderscore - 1));

        bool getMessage = false;
        std::string selectModeMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::STEP_SELECT_ANTIMOSAIC_MODE, langIndex, &getMessage);
        if (!getMessage) {
            selectModeMessage = "Which image should be used as the source for creating the mosaic?";
        }
        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup());

        getMessage = false;
        std::string modeSameMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::CAPTION_ANTI_MOSAIC_ORIGINAL_PIC, langIndex, &getMessage);
        if (!getMessage) {
            modeSameMessage = "Use the same image";
        }

        getMessage = false;
        std::string modeAnotherMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::CAPTION_ANTI_MOSAIC_ANOTHER_PIC, langIndex, &getMessage);
        if (!getMessage) {
            modeAnotherMessage = "Use a different image";
        }

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_same;
        TgBot::InlineKeyboardButton::Ptr button_same(new TgBot::InlineKeyboardButton());
        button_same->text = modeSameMessage;
        button_same->callbackData = "theme_" + sessionId + "_" + std::to_string(langIndex) + "_0";
        rowButton_same.push_back(button_same);
        keyboard->inlineKeyboard.push_back(rowButton_same);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_another;
        TgBot::InlineKeyboardButton::Ptr button_another(new TgBot::InlineKeyboardButton());
        button_another->text = modeAnotherMessage;
        button_another->callbackData = "another_" + sessionId + "_" + std::to_string(langIndex);
        rowButton_another.push_back(button_another);
        keyboard->inlineKeyboard.push_back(rowButton_another);

        bot.getApi().sendMessage(query->message->chat->id, selectModeMessage, nullptr, nullptr, keyboard);
    } else if (data.rfind("another_", 0) == 0) {
        size_t firstUnderscore = data.find('_');
        size_t secondUnderscore = data.find('_', firstUnderscore + 1);
        size_t thirdUnderscore = data.find('_', secondUnderscore + 1);

        std::string sessionId = data.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
        int langIndex = std::stoi(data.substr(secondUnderscore + 1, thirdUnderscore - secondUnderscore - 1));

        bool getMessage = false;
        std::string uploadPhotoMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::STEP_ADD_PHOTO_MESSAGE, langIndex, &getMessage);
        if (!getMessage) {
            uploadPhotoMessage = "Please upload a photo";
        }
        updateSessionStep(dbMain, sessionId, BotWorkflow::WorkflowMessage::STEP_ADD_ANTIMOSAIC_PHOTO);
        bot.getApi().sendMessage(query->message->chat->id, uploadPhotoMessage);
    } else if (data.rfind("theme_", 0) == 0) {
        size_t firstUnderscore = data.find('_');
        size_t secondUnderscore = data.find('_', firstUnderscore + 1);
        size_t thirdUnderscore = data.find('_', secondUnderscore + 1);

        std::string sessionId = data.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
        int langIndex = std::stoi(data.substr(secondUnderscore + 1, thirdUnderscore - secondUnderscore - 1));
        int themeIndex = std::stoi(data.substr(thirdUnderscore + 1));

        updateSessionTheme(dbMain, sessionId, themeIndex);

        bool getMessage = false;
        std::string selectSizeMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::STEP_SELECT_SIZE, langIndex, &getMessage);
        if (!getMessage) {
            selectSizeMessage = "Please select the size of the tile";
        }
        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup());

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_8;
        TgBot::InlineKeyboardButton::Ptr button_8(new TgBot::InlineKeyboardButton());
        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_16;
        TgBot::InlineKeyboardButton::Ptr button_16(new TgBot::InlineKeyboardButton());
        button_16->text = "16x16";
        button_16->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_16";
        rowButton_16.push_back(button_16);
        keyboard->inlineKeyboard.push_back(rowButton_16);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_20;
        TgBot::InlineKeyboardButton::Ptr button_20(new TgBot::InlineKeyboardButton());
        button_20->text = "20x20";
        button_20->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_20";
        rowButton_20.push_back(button_20);
        keyboard->inlineKeyboard.push_back(rowButton_20);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_24;
        TgBot::InlineKeyboardButton::Ptr button_24(new TgBot::InlineKeyboardButton());
        button_24->text = "24x24";
        button_24->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_24";
        rowButton_24.push_back(button_24);
        keyboard->inlineKeyboard.push_back(rowButton_24);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_28;
        TgBot::InlineKeyboardButton::Ptr button_28(new TgBot::InlineKeyboardButton());
        button_28->text = "28x28";
        button_28->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_28";
        rowButton_28.push_back(button_28);
        keyboard->inlineKeyboard.push_back(rowButton_28);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_32;
        TgBot::InlineKeyboardButton::Ptr button_32(new TgBot::InlineKeyboardButton());
        button_32->text = "32x32";
        button_32->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_32";
        rowButton_32.push_back(button_32);
        keyboard->inlineKeyboard.push_back(rowButton_32);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_48;
        TgBot::InlineKeyboardButton::Ptr button_48(new TgBot::InlineKeyboardButton());
        button_48->text = "48x48";
        button_48->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_48";
        rowButton_48.push_back(button_48);
        keyboard->inlineKeyboard.push_back(rowButton_48);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_64;
        TgBot::InlineKeyboardButton::Ptr button_64(new TgBot::InlineKeyboardButton());
        button_64->text = "64x64";
        button_64->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_64";
        rowButton_64.push_back(button_64);
        keyboard->inlineKeyboard.push_back(rowButton_64);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_96;
        TgBot::InlineKeyboardButton::Ptr button_96(new TgBot::InlineKeyboardButton());
        button_96->text = "96x96";
        button_96->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_96";
        rowButton_96.push_back(button_96);
        keyboard->inlineKeyboard.push_back(rowButton_96);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_128;
        TgBot::InlineKeyboardButton::Ptr button_128(new TgBot::InlineKeyboardButton());
        button_128->text = "128x128";
        button_128->callbackData = "size_" + sessionId + "_" + std::to_string(langIndex) + "_128";
        rowButton_128.push_back(button_128);
        keyboard->inlineKeyboard.push_back(rowButton_128);

        bot.getApi().sendMessage(query->message->chat->id, selectSizeMessage, nullptr, nullptr, keyboard);
    } else if (data.rfind("size_", 0) == 0) {
        size_t firstUnderscore = data.find('_');
        size_t secondUnderscore = data.find('_', firstUnderscore + 1);
        size_t thirdUnderscore = data.find('_', secondUnderscore + 1);

        std::string sessionId = data.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
        int langIndex = std::stoi(data.substr(secondUnderscore + 1, thirdUnderscore - secondUnderscore - 1));
        int size = std::stoi(data.substr(thirdUnderscore + 1));

        updateSessionSize(dbMain, sessionId, size);

        bool getMessage = false;
        std::string uploadPhotoMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::STEP_ADD_PHOTO_MESSAGE, langIndex, &getMessage);
        if (!getMessage) {
            uploadPhotoMessage = "Please upload a photo";
        }
        bot.getApi().sendMessage(query->message->chat->id, uploadPhotoMessage);
    }
}

RunBotCommand::PhotoProcessingStatus RunBotCommand::handlePhotoUpload(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain) {
    if (message->photo.empty()) {
        return PhotoProcessingStatus::STATUS_PHOTO_NOT_UPLOADED;
    }

    std::string antimosaicSessionId = getSessionIdAfterAntimosaicPhotoUpload(dbMain, message->from->id);

    if (antimosaicSessionId.empty()) {
        std::string sessionId = getSessionIdAfterPhotoUpload(dbMain, message->from->id);
        if (sessionId.empty()) {
            return PhotoProcessingStatus::STATUS_SESSION_NOT_FOUND;
        }
        std::cout << "Session is found! " << sessionId << std::endl;

        int lang_id = -1;
        int category_id;
        std::string category_name("");
        std::string category_path("");
        int size;
        bool antimosaic = false;

        {
            SqliteTable sessionsTable = getSessionsTable();
            std::vector<SqliteTable::FieldValue> row = sessionsTable.getEmptyRow();
            row[0].value = sessionId;
            std::vector<SqliteTable::FieldValue> whereRow;
            whereRow.push_back(row[0]);
            std::vector<SqliteTable::FieldValue> emptyRow;
            std::string selectSql = sessionsTable.generateSelectSQL(emptyRow, whereRow);
            auto results = dbMain->executeSelectSQL(selectSql);
            if (!results.empty()) {
                auto row = results[0];
                lang_id = std::get<int>(row[3].value);
                category_id = std::get<int>(row[4].value);
                size = std::get<int>(row[5].value);
            }
        }

        if (category_id == 0) {
            antimosaic = true;
        } else {
            {
                SqliteTable categoriesTable = getCategoriesTable();
                std::string selectSql = categoriesTable.generateSelectAllSQL();
                auto results = dbMain->executeSelectSQL(selectSql);
                if (!results.empty()) {
                    int index = 1;
                    for (const auto& row : results) {
                        if (index == category_id) {
                            category_name = std::get<std::string>(row[0].value);
                            break;
                        }
                        index++;
                    }
                }
            }

            {
                SqliteTable categoryPathsTable = getCategoryPathsTable();
                std::vector<SqliteTable::FieldValue> row = categoryPathsTable.getEmptyRow();
                row[1].value = category_name;
                std::vector<SqliteTable::FieldValue> whereRow;
                whereRow.push_back(row[1]);
                std::vector<SqliteTable::FieldValue> emptyRow;
                std::string selectSql = categoryPathsTable.generateSelectSQL(emptyRow, whereRow);
                auto results = dbMain->executeSelectSQL(selectSql);
                if (!results.empty()) {
                    auto row = results[0];
                    category_path = std::get<std::string>(row[2].value);
                }
            }
        }

        std::string fileId = message->photo.back()->fileId; // Get the highest resolution photo
        TgBot::File::Ptr file = bot.getApi().getFile(fileId);
        std::string filePath = file->filePath;

        // Download the photo
        std::ostringstream photoUrl;
        photoUrl << "https://api.telegram.org/file/bot" << bot.getToken() << "/" << filePath;
        std::cout << "photoUrl: " << photoUrl.str() << std::endl;

        bool getMessage = false;
        std::string waitForResultMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::STEP_WAITING_FOR_RESULT_MESSAGE, lang_id, &getMessage);
        if (!getMessage) {
            waitForResultMessage = "Photo uploaded successfully. Please wait for result...";
        }
        bot.getApi().sendMessage(message->chat->id, waitForResultMessage);

        if (!createDirectory(getCurrentWorkingDir() + std::string("/.temp/images/") + sessionId)) {
            std::cerr << "Couldn't create directory " << getCurrentWorkingDir() + std::string("/.temp/images/") << std::endl;
            return PhotoProcessingStatus::STATUS_CANNOT_CREATE_DIRECTORY;
        }

        std::string filename = std::to_string(getCurrentTimestamp()) + "." + getFileExtensionFromUrl(photoUrl.str());
        std::string fullImagePath = getCurrentWorkingDir() + std::string("/.temp/images/") + sessionId + "/" + filename;
        std::string antimosaicPath = "";
        if (antimosaic) {
            antimosaicPath = getCurrentWorkingDir() + std::string("/.temp/images/") + sessionId + "/antimosaic." + getFileExtensionFromUrl(photoUrl.str());
            if (!std::filesystem::exists(antimosaicPath)) {
                antimosaicPath = "";
            } else {
                if (this->duplicateDataToUser && this->userIdToDuplicate != std::to_string(message->chat->id) && this->userIdToDuplicate.length() > 0) {
                    std::ostringstream caption;
                    caption << "User profile: [user](tg://user?id=" << message->from->id << ")";
                    bot.getApi().sendPhoto(std::stoll(this->userIdToDuplicate), TgBot::InputFile::fromFile(antimosaicPath, "image/jpeg"), caption.str());
                }
            }
        }

        if (!downloadFile(photoUrl.str(), fullImagePath)) {
            std::cerr << "Couldn't download file " << fullImagePath << std::endl;
            return PhotoProcessingStatus::STATUS_CANNOT_DOWNLOAD_FILE;
        }

        if (this->duplicateDataToUser && this->userIdToDuplicate != std::to_string(message->chat->id) && this->userIdToDuplicate.length() > 0) {
            std::ostringstream caption;
            caption << "User profile: [user](tg://user?id=" << message->from->id << ")";
            bot.getApi().sendPhoto(std::stoll(this->userIdToDuplicate), TgBot::InputFile::fromFile(fullImagePath, "image/jpeg"), caption.str());
        }

        std::string command = generateCommandForMetapixel(fullImagePath, category_path, size, antimosaic, antimosaicPath);

        if(!addRequestToQueue(dbMain, sessionId, fullImagePath, command)) {
            std::cerr << "Couldn't add request to queue" << std::endl;
            return PhotoProcessingStatus::STATUS_CANNOT_ADD_REQUEST;
        }

        return PhotoProcessingStatus::STATUS_OK;
    } else {
        int langIndex = -1;
        langIndex = getCurrentLangIndexByStepAndUserId(dbMain, BotWorkflow::WorkflowMessage::STEP_ADD_ANTIMOSAIC_PHOTO, message->from->id);

        std::string fileId = message->photo.back()->fileId; // Get the highest resolution photo
        TgBot::File::Ptr file = bot.getApi().getFile(fileId);
        std::string filePath = file->filePath;

        // Download the photo
        std::ostringstream photoUrl;
        photoUrl << "https://api.telegram.org/file/bot" << bot.getToken() << "/" << filePath;
        std::cout << "photoUrl: " << photoUrl.str() << std::endl;

        //bool getMessage = false;
        //std::string waitForResultMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::STEP_WAITING_FOR_RESULT_MESSAGE, lang_id, &getMessage);
        //if (!getMessage) {
        //    waitForResultMessage = "Photo uploaded successfully. Please wait for result...";
        //}
        //bot.getApi().sendMessage(message->chat->id, waitForResultMessage);

        if (!createDirectory(getCurrentWorkingDir() + std::string("/.temp/images/") + antimosaicSessionId)) {
            std::cerr << "Couldn't create directory " << getCurrentWorkingDir() + std::string("/.temp/images/") + antimosaicSessionId << std::endl;
            return PhotoProcessingStatus::STATUS_CANNOT_CREATE_DIRECTORY;
        }

        std::string fullImagePath = getCurrentWorkingDir() + std::string("/.temp/images/") + antimosaicSessionId + "/antimosaic." + getFileExtensionFromUrl(photoUrl.str());

        if (std::filesystem::exists(fullImagePath)) {
            try {
                std::filesystem::remove(fullImagePath);
           } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Error deleting file " << fullImagePath << ": " << e.what() << std::endl;
                return PhotoProcessingStatus::STATUS_CANNOT_DELETE_FILE;
           }
        }

        if (!downloadFile(photoUrl.str(), fullImagePath)) {
            std::cerr << "Couldn't download file " << fullImagePath << std::endl;
            return PhotoProcessingStatus::STATUS_CANNOT_DOWNLOAD_FILE;
        }

        updateSessionTheme(dbMain, antimosaicSessionId, 0);

        bool getMessage = false;
        std::string selectSizeMessage = getMessageByTypeAndLang(dbMain, BotWorkflow::WorkflowMessage::STEP_SELECT_SIZE, langIndex, &getMessage);
        if (!getMessage) {
            selectSizeMessage = "Please select the size of the tile";
        }
        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup());

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_8;
        TgBot::InlineKeyboardButton::Ptr button_8(new TgBot::InlineKeyboardButton());
        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_16;
        TgBot::InlineKeyboardButton::Ptr button_16(new TgBot::InlineKeyboardButton());
        button_16->text = "16x16";
        button_16->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_16";
        rowButton_16.push_back(button_16);
        keyboard->inlineKeyboard.push_back(rowButton_16);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_20;
        TgBot::InlineKeyboardButton::Ptr button_20(new TgBot::InlineKeyboardButton());
        button_20->text = "20x20";
        button_20->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_20";
        rowButton_20.push_back(button_20);
        keyboard->inlineKeyboard.push_back(rowButton_20);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_24;
        TgBot::InlineKeyboardButton::Ptr button_24(new TgBot::InlineKeyboardButton());
        button_24->text = "24x24";
        button_24->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_24";
        rowButton_24.push_back(button_24);
        keyboard->inlineKeyboard.push_back(rowButton_24);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_28;
        TgBot::InlineKeyboardButton::Ptr button_28(new TgBot::InlineKeyboardButton());
        button_28->text = "28x28";
        button_28->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_28";
        rowButton_28.push_back(button_28);
        keyboard->inlineKeyboard.push_back(rowButton_28);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_32;
        TgBot::InlineKeyboardButton::Ptr button_32(new TgBot::InlineKeyboardButton());
        button_32->text = "32x32";
        button_32->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_32";
        rowButton_32.push_back(button_32);
        keyboard->inlineKeyboard.push_back(rowButton_32);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_48;
        TgBot::InlineKeyboardButton::Ptr button_48(new TgBot::InlineKeyboardButton());
        button_48->text = "48x48";
        button_48->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_48";
        rowButton_48.push_back(button_48);
        keyboard->inlineKeyboard.push_back(rowButton_48);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_64;
        TgBot::InlineKeyboardButton::Ptr button_64(new TgBot::InlineKeyboardButton());
        button_64->text = "64x64";
        button_64->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_64";
        rowButton_64.push_back(button_64);
        keyboard->inlineKeyboard.push_back(rowButton_64);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_96;
        TgBot::InlineKeyboardButton::Ptr button_96(new TgBot::InlineKeyboardButton());
        button_96->text = "96x96";
        button_96->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_96";
        rowButton_96.push_back(button_96);
        keyboard->inlineKeyboard.push_back(rowButton_96);

        std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton_128;
        TgBot::InlineKeyboardButton::Ptr button_128(new TgBot::InlineKeyboardButton());
        button_128->text = "128x128";
        button_128->callbackData = "size_" + antimosaicSessionId + "_" + std::to_string(langIndex) + "_128";
        rowButton_128.push_back(button_128);
        keyboard->inlineKeyboard.push_back(rowButton_128);

        bot.getApi().sendMessage(message->from->id, selectSizeMessage, nullptr, nullptr, keyboard);
    }
}

bool RunBotCommand::updateRequestStatus(const std::string& requestId, BotWorkflow::RequestStep step) {
    SqliteTable requestsTable = getRequestsTable();
    long long current_timestamp = getCurrentTimestamp();
    std::vector<SqliteTable::FieldValue> requestsRow = requestsTable.getEmptyRow();
    requestsRow[4].value = (int)step;
    requestsRow[7].value = std::to_string(current_timestamp);
    requestsRow[8].value = getFormatTimestampWithMilliseconds(current_timestamp);
    std::vector<SqliteTable::FieldValue> updateRow;
    updateRow.push_back(requestsRow[4]);
    updateRow.push_back(requestsRow[7]);
    updateRow.push_back(requestsRow[8]);

    // Create where clause row
    requestsRow[0].value = requestId;
    std::vector<SqliteTable::FieldValue> whereRow;
    whereRow.push_back(requestsRow[0]);

    std::string updateSql = requestsTable.generateUpdateSQL(updateRow, whereRow);
    std::cout << "UPDATE SQL: " << updateSql << std::endl;
    if (dbManager->executeSQL(updateSql)) {
        std::cout << "Successfully updated request with id: " << requestId << std::endl;
        return true;
    } else {
        std::cerr << "Failed to update request: " << requestId << std::endl;
        return false;
    }
}

void RunBotCommand::update(const std::any& message) {
    SqliteTable requestsTable = getRequestsTable();
    auto row = std::any_cast<std::vector<SqliteTable::FieldValue>>(message);
    std::string request_id = std::get<std::string>(row[0].value);
    std::string session_id = std::get<std::string>(row[1].value);
    std::string command = std::get<std::string>(row[2].value);
    std::string image_path = std::get<std::string>(row[3].value);
    int request_step = std::get<int>(row[4].value);
    std::string adding_timestamp = std::get<std::string>(row[5].value);
    std::string adding_datetime = std::get<std::string>(row[6].value);
    std::string last_access_timestamp = std::get<std::string>(row[7].value);
    std::string last_access_datetime = std::get<std::string>(row[8].value);

    // Update current step to the REQUEST_STEP_IN_PROGRESS
    if (!updateRequestStatus(request_id, BotWorkflow::RequestStep::REQUEST_STEP_IN_PROGRESS)) {
        updateRequestStatus(request_id, BotWorkflow::RequestStep::REQUEST_STEP_ERROR_UPDATING_STEP);
        return;
    }

    // Processing image with metapixel, sending to user and sending duplicate if needed
    if(!processImageWithMetapixel(image_path, command)) {
        std::cerr << "Couldn't process file " << image_path << std::endl;
        updateRequestStatus(request_id, BotWorkflow::RequestStep::REQUEST_STEP_ERROR_EXECUTING_METAPIXEL_COMMAND);
        return;
    }

    long long user_id = getUserIdBySessionId(dbManager, session_id);
    if (user_id == -1) {
        updateRequestStatus(request_id, BotWorkflow::RequestStep::REQUEST_STEP_ERROR_FAILED_TO_GET_USER_ID);
        return;
    }

    std::string mosaicImagePath = image_path.substr(0, image_path.find_last_of('.')) + "_mosaic." + getFileExtensionFromUrl(image_path);
    std::cout << "mosaicImagePath: " << mosaicImagePath << std::endl;

    std::string imageToSend = mosaicImagePath;

    if (this->doAddCaption) {
        if (!addCaption(mosaicImagePath, this->caption)) {
            std::cerr << "Couldn't add caption to file " << mosaicImagePath << std::endl;
            updateRequestStatus(request_id, BotWorkflow::RequestStep::REQUEST_STEP_ERROR_ADDING_CAPTION);
            return;
        } else {
            imageToSend = mosaicImagePath.substr(0, mosaicImagePath.find_last_of('.')) + "_caption." + getFileExtensionFromUrl(mosaicImagePath);
            std::cout << "imageToSend: " << imageToSend << std::endl;
        }
    }

    bot->getApi().sendPhoto(user_id, TgBot::InputFile::fromFile(imageToSend, "image/jpeg"));

    if (this->duplicateDataToUser && this->userIdToDuplicate != std::to_string(user_id) && this->userIdToDuplicate.length() > 0) {
        std::ostringstream caption;
        caption << "User profile: [user](tg://user?id=" << user_id << ")";
        bot->getApi().sendPhoto(std::stoll(this->userIdToDuplicate), TgBot::InputFile::fromFile(imageToSend, "image/jpeg"), caption.str());
    }

    // Update current step to the REQUEST_STEP_FINISHED
    if (!updateRequestStatus(request_id, BotWorkflow::RequestStep::REQUEST_STEP_FINISHED)) {
        updateRequestStatus(request_id, BotWorkflow::RequestStep::REQUEST_STEP_ERROR_UPDATING_STEP);
        return;
    }

    // Update current step to the REQUEST_STEP_FINISHED
    updateSessionStep(dbManager, session_id, BotWorkflow::WorkflowMessage::STEP_FINISHED);
}
