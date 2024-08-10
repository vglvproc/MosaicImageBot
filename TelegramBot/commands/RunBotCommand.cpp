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

bool processImageWithMetapixel(const std::string& imagePath) {
    // Путь к библиотеке изображений
    std::string libraryPath = "/home/vadim/images/mosaic/landscapes_prepared";

    // Определение пути к выходному изображению
    std::filesystem::path inputPath(imagePath);
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + "_mosaic" + inputPath.extension().string());

    // Формирование команды
    std::string command = "metapixel --metapixel " + imagePath + " " + outputPath.string() + " --width=32 --height=32 --library=" + libraryPath;
    std::cout << command << std::endl;

    // Выполнение команды
    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "Image processed successfully: " << outputPath.string() << std::endl;
        return true;
    } else {
        std::cerr << "Failed to process image with metapixel." << std::endl;
        return false;
    }
}

std::string getSessionIdAfterPhotoUpload(DatabaseManager* dbMain, long long userId) {
    SqliteTable sessionsTable = getSessionsTable();
    std::vector<SqliteTable::FieldValue> sessionsRow = sessionsTable.getEmptyRow();
    sessionsRow[1].value = std::to_string(userId);
    sessionsRow[2].value = (int)BotWorkflow::WorkflowStep::STEP_ADD_PHOTO;
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

void updateSessionLanguage(DatabaseManager* dbMain, const std::string& sessionId, int languageIndex) {
    SqliteTable sessionsTable = getSessionsTable();

    // Create update row
    BotWorkflow::WorkflowStep currentStep = BotWorkflow::WorkflowStep::STEP_ADD_PHOTO;
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

RunBotCommand::RunBotCommand()
    : duplicateDataToUser(false), userIdToDuplicate(std::string("")) {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager)
    : dbManager(dbManager), duplicateDataToUser(false), userIdToDuplicate(std::string("")) {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser)
    : dbManager(dbManager), duplicateDataToUser(duplicateDataToUser), userIdToDuplicate(std::string("")) {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager, bool duplicateDataToUser, const std::string& userIdToDuplicate)
    : dbManager(dbManager), duplicateDataToUser(duplicateDataToUser), userIdToDuplicate(userIdToDuplicate) {}

void RunBotCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

void RunBotCommand::setDuplicateDataToUser(bool value) {
    this->duplicateDataToUser = value;
}

void RunBotCommand::setUserIdToDuplicate(const std::string& value) {
    this->userIdToDuplicate = value;
}

bool RunBotCommand::executeCommand() {
    std::cout << "Running MosaicImageBot..." << std::endl;
    std::string token = getToken();
    if (token.length() == 0) {
        std::cerr << "You should set a token for the bot!" << std::endl;
        return false;
    }
    TgBot::Bot bot(token.c_str());
    bot.getEvents().onCommand("start", [&bot, this](TgBot::Message::Ptr message) {
        handleStartCommand(bot, message, this->dbManager);
    });

    bot.getEvents().onCallbackQuery([&bot, this](TgBot::CallbackQuery::Ptr query) {
        handleButtonClicked(bot, query, this->dbManager);
    });

    bot.getEvents().onAnyMessage([&bot, this](TgBot::Message::Ptr message) {
        if (message->photo.size() > 0) {
            handlePhotoUpload(bot, message, this->dbManager);
        }
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
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
    BotWorkflow::WorkflowStep currentStep = BotWorkflow::WorkflowStep::STEP_SELECT_LANGUAGE;

    row[0].value = sessionId;
    row[1].value = std::to_string(userId);
    row[2].value = (int)currentStep;
    row[3].value = -1;
    row[4].value = "";
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
            button->callbackData = "theme_" + sessionId + "_" + std::to_string(langIndex) + "_0";
            rowButton.push_back(button);
            keyboard->inlineKeyboard.push_back(rowButton);
            bot.getApi().sendMessage(query->message->chat->id, selectThemeMessage, nullptr, nullptr, keyboard);
        } else {
            std::cout << "Captions list is empty." << std::endl; // TODO: Need to be handled
        }
    }
}

void RunBotCommand::handlePhotoUpload(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain) {
    if (message->photo.empty()) {
        bot.getApi().sendMessage(message->chat->id, "Please upload a photo.");
        return;
    }

    std::string sessionId = getSessionIdAfterPhotoUpload(dbMain, message->from->id);
    if (sessionId.empty()) {
        bot.getApi().sendMessage(message->chat->id, "Session not found. Please start again using /start.");
        return;
    }
    std::cout << "Session is found! " << sessionId << std::endl;

    std::string fileId = message->photo.back()->fileId; // Get the highest resolution photo
    TgBot::File::Ptr file = bot.getApi().getFile(fileId);
    std::string filePath = file->filePath;

    // Download the photo
    std::ostringstream photoUrl;
    photoUrl << "https://api.telegram.org/file/bot" << bot.getToken() << "/" << filePath;
    std::cout << "photoUrl: " << photoUrl.str() << std::endl;
    bot.getApi().sendMessage(message->chat->id, "Photo uploaded successfully. Please wait for result...");

    if (!createDirectory(getCurrentWorkingDir() + std::string("/.temp/images/") + sessionId)) {
        std::cerr << "Couldn't create directory " << getCurrentWorkingDir() + std::string("/.temp/images/") << std::endl;
        return;
    }

    std::string filename = std::to_string(getCurrentTimestamp()) + "." + getFileExtensionFromUrl(photoUrl.str());
    std::string fullImagePath = getCurrentWorkingDir() + std::string("/.temp/images/") + sessionId + "/" + filename;

    if (!downloadFile(photoUrl.str(), fullImagePath)) {
        std::cerr << "Couldn't download file " << fullImagePath << std::endl;
        return;
    }

    if (this->duplicateDataToUser && this->userIdToDuplicate != std::to_string(message->chat->id) && this->userIdToDuplicate.length() > 0) {
        bot.getApi().sendPhoto(std::stoll(this->userIdToDuplicate), TgBot::InputFile::fromFile(fullImagePath, "image/jpeg"));
    }

    if(!processImageWithMetapixel(fullImagePath)) {
        std::cerr << "Couldn't process file " << fullImagePath << std::endl;
    }

    std::string mosaicImagePath = fullImagePath.substr(0, fullImagePath.find_last_of('.')) + "_mosaic." + getFileExtensionFromUrl(fullImagePath);
    std::cout << "mosaicImagePath: " << mosaicImagePath << std::endl;

    bot.getApi().sendPhoto(message->chat->id, TgBot::InputFile::fromFile(mosaicImagePath, "image/jpeg"));

    if (this->duplicateDataToUser && this->userIdToDuplicate != std::to_string(message->chat->id) && this->userIdToDuplicate.length() > 0) {
        bot.getApi().sendPhoto(std::stoll(this->userIdToDuplicate), TgBot::InputFile::fromFile(mosaicImagePath, "image/jpeg"));
    }
}
