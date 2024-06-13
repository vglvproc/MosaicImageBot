#include "RemoveCategoryCommand.h"

RemoveCategoryCommand::RemoveCategoryCommand(const std::string& category_name)
    : category_name(category_name) {}

std::string RemoveCategoryCommand::getCategoryName() const {
    return category_name;
}

void RemoveCategoryCommand::setCategoryName(const std::string& category_name) {
    this->category_name = category_name;
}