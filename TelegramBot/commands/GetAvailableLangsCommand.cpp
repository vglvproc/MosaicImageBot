#include <iostream>
#include "GetAvailableLangsCommand.h"

GetAvailableLangsCommand::GetAvailableLangsCommand() {}

GetAvailableLangsCommand::GetAvailableLangsCommand(DatabaseManager *dbManager)
    : dbManager(dbManager) {}

void GetAvailableLangsCommand::setDatabaseManager(DatabaseManager *dbManager) {
    this->dbManager = dbManager;
}

bool GetAvailableLangsCommand::executeCommand() {
    SqliteTable langsTable = getLanguagesTable();
    std::string sqlCommand = langsTable.generateSelectAllSQL();
    auto results = dbManager->executeSelectSQL(sqlCommand);
    if (!results.empty()) {
        std::cout << "These languages are available now:" << std::endl;
        std::cout << "ID\t\tLanguage" << std::endl;
        for (const auto& row : results) {
            std::cout << std::get<int>(row[0].value) << "\t\t" << std::get<std::string>(row[1].value) << std::endl;
        }
    } else {
        std::cout << "Languages list is empty." << std::endl;
    }

    return true;
}
