#include <docopt/docopt.h>
#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <thread>
#include <chrono>
#include "utils/Utils.h"
#include "commands/RunBotCommand.h"
#include "commands/AddCategoryCommand.h"
#include "commands/RemoveCategoryCommand.h"
#include "commands/AddImagesToCategoryCommand.h"
#include "commands/AddNoAdsUserCommand.h"
#include "commands/RemoveNoAdsUserCommand.h"
#include "commands/AddUnlimitedAccessUserCommand.h"
#include "commands/RemoveUnlimitedAccessUserCommand.h"
#include "commands/GetAvailableLangsCommand.h"
#include "commands/SetTokenCommand.h"
#include "commands/GetTokenCommand.h"
#include "commands/SetCategoryCaptionCommand.h"
#include "db/SqliteTable.h"
#include "db/InitialEntities.h"
#include "db/DatabaseManager.h"
#include "requests/RequestsManager.h"

static const char USAGE[] =
R"(MosaicImageBot.

Usage:
  MosaicImageBot
  MosaicImageBot run [--duplicate-data=<user_id>] [--add-caption=<caption>]
  MosaicImageBot set-token <token>
  MosaicImageBot get-token
  MosaicImageBot add-category <category_name>
  MosaicImageBot remove-category <category_name>
  MosaicImageBot add-images-to-category <category_name> <path_to_images>
  MosaicImageBot set-category-caption <category_name> --caption=<caption> --lang=<lang_id>
  MosaicImageBot add-no-ads-user <user_id>
  MosaicImageBot remove-no-ads-user <user_id>
  MosaicImageBot add-unlimited-access-user <user_id>
  MosaicImageBot remove-unlimited-access-user <user_id>
  MosaicImageBot get-available-langs
  MosaicImageBot (-h | --help)
  MosaicImageBot --version

Options:
  -h --help          Show this screen.
  --version          Show version.
  <category_name>    The name of the category to add or remove.
  <path_to_images>   The path to the directory containing images to add to the category.
)";

bool checkMetapixelAvailability() {
    std::array<char, 512> buffer;
    std::string result;
    FILE* pipe = popen("metapixel --version", "r");
    if (!pipe) throw std::runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    int returnCode = pclose(pipe);

    return (returnCode == 0);
}

bool checkFfmpegAvailability() {
    std::array<char, 512> buffer;
    std::string result;
    FILE* pipe = popen("ffmpeg -version", "r");
    if (!pipe) throw std::runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    int returnCode = pclose(pipe);

    return (returnCode == 0 && result.find("version") != std::string::npos);
}

std::unique_ptr<Command> parseCommandLine(int argc, const char** argv) {
    try {
        std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
                                                                   {argv + 1, argv + argc},
                                                                   true, // show help if requested
                                                                   "MosaicImageBot 1.0");
        if (args["set-token"].asBool()) {
            std::string token = args["<token>"].asString();
            return std::make_unique<SetTokenCommand>(token);
        } else if (args["get-token"].asBool()) {
            return std::make_unique<GetTokenCommand>();
        } else if (args["add-category"].asBool()) {
            std::string category_name = args["<category_name>"].asString();
            return std::make_unique<AddCategoryCommand>(category_name);
        } else if (args["remove-category"].asBool()) {
            std::string category_name = args["<category_name>"].asString();
            return std::make_unique<RemoveCategoryCommand>(category_name);
        } else if (args["add-images-to-category"].asBool()) {
            std::string category_name = args["<category_name>"].asString();
            std::string path_to_images = args["<path_to_images>"].asString();
            if (!doesPathExist(path_to_images)) {
                std::cerr << "Path \"" << path_to_images << "\" does not exist in the system!" << std::endl;
                exit(0);
            }
            return std::make_unique<AddImagesToCategoryCommand>(category_name, path_to_images);
        } else if (args["set-category-caption"].asBool()) {
            std::string category_name = args["<category_name>"].asString();
            std::string caption = args["--caption"].asString();
            int lang_id = args["--lang"].asLong();
            return std::make_unique<SetCategoryCaptionCommand>(category_name, caption, lang_id);
        } else if (args["add-no-ads-user"].asBool()) {
            std::string user_id = args["<user_id>"].asString();
            return std::make_unique<AddNoAdsUserCommand>(user_id);
        } else if (args["remove-no-ads-user"].asBool()) {
            std::string user_id = args["<user_id>"].asString();
            return std::make_unique<RemoveNoAdsUserCommand>(user_id);
        } else if (args["add-unlimited-access-user"].asBool()) {
            std::string user_id = args["<user_id>"].asString();
            return std::make_unique<AddUnlimitedAccessUserCommand>(user_id);
        } else if (args["remove-unlimited-access-user"].asBool()) {
            std::string user_id = args["<user_id>"].asString();
            return std::make_unique<RemoveUnlimitedAccessUserCommand>(user_id);
        } else if (args["get-available-langs"].asBool()) {
            return std::make_unique<GetAvailableLangsCommand>();
        } else if (args["run"].asBool()) {
            std::unique_ptr<RunBotCommand> runBotCommand = std::make_unique<RunBotCommand>();
            if (args["--duplicate-data"]) {
                std::string userId = args["--duplicate-data"].asString();
                runBotCommand->setDuplicateDataToUser(true);
                runBotCommand->setUserIdToDuplicate(userId);
            }
            if (args["--add-caption"]) {
                std::string caption = args["--add-caption"].asString();
                runBotCommand->setDoAddCaption(true);
                runBotCommand->setCaption(caption);
            }
            return runBotCommand;
        } else {
            std::cout << USAGE;
            exit(0);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
    }

    return nullptr;
}

void processInLoop(RequestsManager& manager) {
    while (true) {
        manager.process();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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

    if(!initLanguagesTable(dbMain)) {
        std::cerr << "Failed to initialize languages table." << std::endl;
        return 1;
    }

    RequestsManager manager(&dbMain);

    if (dynamic_cast<SetTokenCommand*>(command.get())) {
        SetTokenCommand* cmd = dynamic_cast<SetTokenCommand*>(command.get());
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully set bot token \"" << cmd->getToken() << "\"" << std::endl;
        } else {
            std::cout << "Failed to set bot token \"" << cmd->getToken() << "\"" << std::endl;
        }
        return 0;
    } else if (dynamic_cast<GetTokenCommand*>(command.get())) {
        GetTokenCommand* cmd = dynamic_cast<GetTokenCommand*>(command.get());
        cmd->setDatabaseManager(&dbMain);
        bool result = false;
        std::string token = cmd->executeCommand(&result);
        if (result) {
            std::cout << "Current bot token is: " << token << std::endl;
        } else {
            std::cout << "Failed to get bot token from database" << std::endl;
        }
        return 0;
    } else if (dynamic_cast<AddCategoryCommand*>(command.get())) {
        AddCategoryCommand* cmd = dynamic_cast<AddCategoryCommand*>(command.get());
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully added category \"" << cmd->getCategoryName() << "\" into database." << std::endl;
        } else {
            std::cout << "Failed to add category \"" << cmd->getCategoryName() << "\" into database." << std::endl;
        }
        return 0;
    } else if (dynamic_cast<RemoveCategoryCommand*>(command.get())) {
        RemoveCategoryCommand* cmd = dynamic_cast<RemoveCategoryCommand*>(command.get());
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully removed category \"" << cmd->getCategoryName() << "\" from database." << std::endl;
        } else {
            std::cout << "Failed to remove category \"" << cmd->getCategoryName() << "\" from database." << std::endl;
        }
        return 0;
    } else if (dynamic_cast<AddImagesToCategoryCommand*>(command.get())) {
        AddImagesToCategoryCommand* cmd = dynamic_cast<AddImagesToCategoryCommand*>(command.get());
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully set path \"" << cmd->getPathToImages() << "\" for category \"" << cmd->getCategoryName()
                      << "\" into database" << std::endl;
        } else {
            std::cout << "Failed to set path \"" << cmd->getPathToImages() << "\" for category \"" << cmd->getCategoryName()
                      << "\" into database" << std::endl;
        }
        return 0;
    } else if (dynamic_cast<SetCategoryCaptionCommand*>(command.get())) {
        SetCategoryCaptionCommand* cmd = dynamic_cast<SetCategoryCaptionCommand*>(command.get());
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully added caption \"" << cmd->getCaption() << "\" for category \"" << cmd->getCategoryName()
                      << "\" and language with ID " << cmd->getLangId() << " into database." << std::endl;
        } else {
            std::cout << "Failed to add caption \"" << cmd->getCaption() << "\" for category \"" << cmd->getCategoryName()
                      << "\" and language with ID " << cmd->getLangId() << " into database." << std::endl;
        }
        return 0;
    } else if (dynamic_cast<AddNoAdsUserCommand*>(command.get())) {
        AddNoAdsUserCommand* cmd = dynamic_cast<AddNoAdsUserCommand*>(command.get()); // TODO: Check user for right ID!
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully added user with id \"" << cmd->getUserId() << "\" into no ads receiving list." << std::endl;
        } else {
            std::cout << "Failed to add user with id \"" << cmd->getUserId() << "\" into no ads receiving list." << std::endl;
        }
        return 0;
    } else if (dynamic_cast<RemoveNoAdsUserCommand*>(command.get())) {
        RemoveNoAdsUserCommand* cmd = dynamic_cast<RemoveNoAdsUserCommand*>(command.get()); // TODO: Check user for right ID!
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully removed user with id \"" << cmd->getUserId() << "\" from no ads receiving list." << std::endl;
        } else {
            std::cout << "Failed to remove user with id \"" << cmd->getUserId() << "\" from no ads receiving list." << std::endl;
        }
        return 0;
    } else if (dynamic_cast<AddUnlimitedAccessUserCommand*>(command.get())) {
        AddUnlimitedAccessUserCommand* cmd = dynamic_cast<AddUnlimitedAccessUserCommand*>(command.get()); // TODO: Check user for right ID!
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully added user with id \"" << cmd->getUserId() << "\" into unlimited access list." << std::endl;
        } else {
            std::cout << "Failed to add user with id \"" << cmd->getUserId() << "\" into unlimited access list." << std::endl;
        }
        return 0;
    } else if (dynamic_cast<RemoveUnlimitedAccessUserCommand*>(command.get())) {
        RemoveUnlimitedAccessUserCommand* cmd = dynamic_cast<RemoveUnlimitedAccessUserCommand*>(command.get()); // TODO: Check user for right ID!
        cmd->setDatabaseManager(&dbMain);
        bool result = cmd->executeCommand();
        if (result) {
            std::cout << "Successfully removed user with id \"" << cmd->getUserId() << "\" from unlimited access list." << std::endl;
        } else {
            std::cout << "Failed to remove user with id \"" << cmd->getUserId() << "\" from unlimited access list." << std::endl;
        }
        return 0;
    } else if (dynamic_cast<GetAvailableLangsCommand*>(command.get())) {
        GetAvailableLangsCommand* cmd = dynamic_cast<GetAvailableLangsCommand*>(command.get());
        cmd->setDatabaseManager(&dbMain);
        cmd->executeCommand();
        return 0;
    } else if (dynamic_cast<RunBotCommand*>(command.get())) {
        RunBotCommand* cmd = dynamic_cast<RunBotCommand*>(command.get());
        if (!checkMetapixelAvailability()) {
            std::cout << "metapixel is not available in your system." << std::endl;
            return 1;
        }
        if (cmd->getDoAddCaption()) {
            if (!checkFfmpegAvailability()) {
                std::cout << "ffmpeg is not available in your system." << std::endl;
                return 1;
            }
        }
        std::thread processingThread(processInLoop, std::ref(manager));
        cmd->setDatabaseManager(&dbMain);
        bool res = cmd->executeCommand();
        return res ? 0 : 1;
    } else {
        std::cout << "Unknown command." << std::endl;
    }

    return 0;
}
