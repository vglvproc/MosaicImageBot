#pragma once
#include "Command.h"
#include <string>

class AddCategoryCommand : public Command {
public:
    AddCategoryCommand(const std::string& category_name);

    std::string getCategoryName() const;

    void setCategoryName(const std::string& category_name);

private:
    std::string category_name;
};