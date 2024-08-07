#pragma once
#include "Command.h"
#include <string>
#include "../db/DatabaseManager.h"
#include "../db/SqliteTable.h"
#include "../db/InitialEntities.h"
#include "../utils/Utils.h"

class SetCategoryCaptionCommand : public Command {
public:
    SetCategoryCaptionCommand();
    SetCategoryCaptionCommand(DatabaseManager *dbManager);
    SetCategoryCaptionCommand(const std::string& category_name);
    SetCategoryCaptionCommand(const std::string& category_name, DatabaseManager *dbManagers);
    SetCategoryCaptionCommand(const std::string& category_name, const std::string& caption);
    SetCategoryCaptionCommand(const std::string& category_name, const std::string& caption, DatabaseManager *dbManager);
    SetCategoryCaptionCommand(const std::string& category_name, const std::string& caption, int lang_id);
    SetCategoryCaptionCommand(const std::string& category_name, const std::string& caption, int lang_id, DatabaseManager *dbManager);
    void setCategoryName(const std::string& category_name);
    std::string getCategoryName() const;
    void setCaption(const std::string& caption);
    std::string getCaption() const;
    void setLangId(int lang_id);
    int getLangId() const;
    void setDatabaseManager(DatabaseManager *dbManager);
    bool executeCommand();

private:
    std::string category_name;
    std::string caption;
    int lang_id;
    DatabaseManager *dbManager;
};