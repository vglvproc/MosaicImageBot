#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <tgbot/tgbot.h>
#include "utils/Utils.h"
#include "db/SqliteTable.h"
#include "db/InitialEntities.h"
#include "db/DatabaseManager.h"

void handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager& dbMain) {
    bot.getApi().sendMessage(message->chat->id, "Hello!", false, 0, nullptr, "Markdown");
}

int main(int argc, char* argv[]) {
    DatabaseManager dbMain("./main.db");
    std::vector<SqliteTable> tables = getInitialTables();
    for (auto& table : tables) {
        dbMain.checkAndCreateTable(table);
    }

    TgBot::Bot bot("7347157371:AAEG1fu97mwYKd5W_lFInr3301L0weoiaKw");
    bot.getEvents().onCommand("start", [&bot, &dbMain](TgBot::Message::Ptr message) {
        handleStartCommand(bot, message, dbMain);
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

    return 0;
}
