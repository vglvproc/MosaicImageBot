#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class AddImagesToCategoryCommand : public Command {
public:
    AddImagesToCategoryCommand();
    AddImagesToCategoryCommand(DatabaseManager *dbManager);
    AddImagesToCategoryCommand(const std::string& category_name);
    AddImagesToCategoryCommand(const std::string& category_name, DatabaseManager *dbManager);
    AddImagesToCategoryCommand(const std::string& category_name, const std::string& path_to_images);
    AddImagesToCategoryCommand(const std::string& category_name, const std::string& path_to_images, DatabaseManager *dbManager);

    std::string getCategoryName() const;
    std::string getPathToImages() const;

    void setCategoryName(const std::string& category_name);
    void setPathToImages(const std::string& path_to_images);

    void setDatabaseManager(DatabaseManager *dbManager);
    bool executeCommand();

private:
    std::string category_name;
    std::string path_to_images;
    DatabaseManager *dbManager;
};