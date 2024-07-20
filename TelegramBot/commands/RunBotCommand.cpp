#include <iostream>
#include <tgbot/tgbot.h>
#include "RunBotCommand.h"
#include "../utils/Utils.h"

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

void handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain) {
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
    row[4].value = std::to_string(timestamp);
    row[5].value = getFormatTimestampWithMilliseconds(timestamp);

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
            button->text = std::get<std::string>(row[1].value) + " " + std::get<std::string>(row[2].value);
            button->callbackData = "lang_" + sessionId + "_" + std::to_string(index++);
            rowButton.push_back(button);
            keyboard->inlineKeyboard.push_back(rowButton);
        }
        bot.getApi().sendMessage(message->chat->id, "Please select the language:", false, 0, keyboard);
    } else {
        std::cout << "Languages list is empty." << std::endl; // TODO: Need to be handled
    }
}

void handleLanguageSelection(TgBot::Bot& bot, TgBot::CallbackQuery::Ptr query, DatabaseManager* dbMain) {
    std::string data = query->data;
    if (data.rfind("lang_", 0) == 0) {
        size_t firstUnderscore = data.find('_');
        size_t secondUnderscore = data.find('_', firstUnderscore + 1);

        std::string sessionId = data.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
        int langIndex = std::stoi(data.substr(secondUnderscore + 1));

        updateSessionLanguage(dbMain, sessionId, langIndex);
        bot.getApi().sendMessage(query->message->chat->id, "Language selected. Please add a photo.");
    }
}

RunBotCommand::RunBotCommand() {}

RunBotCommand::RunBotCommand(DatabaseManager *dbManager)
    : dbManager(dbManager) {}

void RunBotCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool RunBotCommand::executeCommand() {
    std::cout << "Running MosaicImageBot..." << std::endl;
    TgBot::Bot bot("7347157371:AAEG1fu97mwYKd5W_lFInr3301L0weoiaKw");
    bot.getEvents().onCommand("start", [&bot, this](TgBot::Message::Ptr message) {
        handleStartCommand(bot, message, this->dbManager);
    });

    bot.getEvents().onCallbackQuery([&bot, this](TgBot::CallbackQuery::Ptr query) {
        handleLanguageSelection(bot, query, this->dbManager);
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
