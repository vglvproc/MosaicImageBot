#include <docopt/docopt.h>
#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <tgbot/tgbot.h>
#include "utils/Utils.h"
#include "commands/RunBotCommand.h"
#include "commands/AddCategoryCommand.h"
#include "commands/RemoveCategoryCommand.h"
#include "commands/AddImagesToCategoryCommand.h"
#include "db/SqliteTable.h"
#include "db/InitialEntities.h"
#include "db/DatabaseManager.h"

static const char USAGE[] =
R"(MosaicImageBot.

Usage:
  MosaicImageBot
  MosaicImageBot add-category <category_name>
  MosaicImageBot remove-category <category_name>
  MosaicImageBot add-images-to-category <category_name> <path_to_images>
  MosaicImageBot (-h | --help)
  MosaicImageBot --version

Options:
  -h --help          Show this screen.
  --version          Show version.
  <category_name>    The name of the category to add or remove.
  <path_to_images>   The path to the directory containing images to add to the category.
)";

std::unique_ptr<Command> parseCommandLine(int argc, const char** argv) {
    try {
        std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
                                                                   {argv + 1, argv + argc},
                                                                   true, // show help if requested
                                                                   "MosaicImageBot 1.0");
        if (args["add-category"].asBool()) {
            std::string category_name = args["<category_name>"].asString();
            return std::make_unique<AddCategoryCommand>(category_name);
        } else if (args["remove-category"].asBool()) {
            std::string category_name = args["<category_name>"].asString();
            return std::make_unique<RemoveCategoryCommand>(category_name);
        } else if (args["add-images-to-category"].asBool()) {
            std::string category_name = args["<category_name>"].asString();
            std::string path_to_images = args["<path_to_images>"].asString();
            return std::make_unique<AddImagesToCategoryCommand>(category_name, path_to_images);
        } else {
            return std::make_unique<RunBotCommand>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
    }

    return nullptr;
}

void handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message, DatabaseManager& dbMain) {
    bot.getApi().sendMessage(message->chat->id, "Hello!", false, 0, nullptr, "Markdown");
}

int main(int argc, const char** argv) {
    std::unique_ptr<Command> command = parseCommandLine(argc, argv);

    if (!command) {
        std::cerr << "Invalid command or arguments." << std::endl;
        std::cout << USAGE << std::endl;
        return 1;
    }

    DatabaseManager dbMain("./main.db");
    std::vector<SqliteTable> tables = getInitialTables();
    for (auto& table : tables) {
        dbMain.checkAndCreateTable(table);
    }

    if (dynamic_cast<AddCategoryCommand*>(command.get())) {
        std::cout << "This is a AddCategoryCommand." << std::endl;
        AddCategoryCommand* cmd = dynamic_cast<AddCategoryCommand*>(command.get());
        std::cout << "category_name: " << cmd->getCategoryName() << std::endl;
        return 0;
    } else if (dynamic_cast<RemoveCategoryCommand*>(command.get())) {
        std::cout << "This is a RemoveCategoryCommand." << std::endl;
        RemoveCategoryCommand* cmd = dynamic_cast<RemoveCategoryCommand*>(command.get());
        std::cout << "category_name: " << cmd->getCategoryName() << std::endl;
        return 0;
    } else if (dynamic_cast<AddImagesToCategoryCommand*>(command.get())) {
        std::cout << "This is a AddImagesToCategoryCommand." << std::endl;
        AddImagesToCategoryCommand* cmd = dynamic_cast<AddImagesToCategoryCommand*>(command.get());
        std::cout << "category_name: " << cmd->getCategoryName() <<  "; path_to_images: " << cmd->getPathToImages() << std::endl;
        return 0;
    } else {
        std::cout << "Unknown command." << std::endl;
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
