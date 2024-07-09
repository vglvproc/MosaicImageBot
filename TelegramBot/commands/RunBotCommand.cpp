#include <iostream>
#include <tgbot/tgbot.h>
#include "RunBotCommand.h"

void handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager* dbMain) {
    bot.getApi().sendMessage(message->chat->id, "Hello!", false, 0, nullptr, "Markdown");
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
