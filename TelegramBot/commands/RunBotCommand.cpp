#include <iostream>
#include <tgbot/tgbot.h>
#include "RunBotCommand.h"
#include "../utils/Utils.h"

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
    row[3].value = std::to_string(timestamp);
    row[4].value = getFormatTimestampWithMilliseconds(timestamp);

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
        for (const auto& row : results) {
            std::vector<TgBot::InlineKeyboardButton::Ptr> rowButton;
            TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton());
            button->text = std::get<std::string>(row[1].value) + " " + std::get<std::string>(row[2].value);
            button->callbackData = "lang_" + std::to_string(std::get<int>(row[0].value));
            rowButton.push_back(button);
            keyboard->inlineKeyboard.push_back(rowButton);
        }
        bot.getApi().sendMessage(message->chat->id, "Please select the language:", false, 0, keyboard);
    } else {
        std::cout << "Languages list is empty." << std::endl; // TODO: Need to be handled
    }
    // TODO: Need to expand session table with selected language id.
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
