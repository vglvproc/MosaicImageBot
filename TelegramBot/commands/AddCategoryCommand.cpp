#include "AddCategoryCommand.h"

AddCategoryCommand::AddCategoryCommand(const std::string& category_name)
    : category_name(category_name) {}

std::string AddCategoryCommand::getCategoryName() const {
    return category_name;
}

void AddCategoryCommand::setCategoryName(const std::string& category_name) {
    this->category_name = category_name;
}