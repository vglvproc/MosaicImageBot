#include "AddImagesToCategoryCommand.h"

AddImagesToCategoryCommand::AddImagesToCategoryCommand(const std::string& category_name, const std::string& path_to_images)
    : category_name(category_name), path_to_images(path_to_images) {}

std::string AddImagesToCategoryCommand::getCategoryName() const {
    return category_name;
}

std::string AddImagesToCategoryCommand::getPathToImages() const {
    return path_to_images;
}

void AddImagesToCategoryCommand::setCategoryName(const std::string& category_name) {
    this->category_name = category_name;
}

void AddImagesToCategoryCommand::setPathToImages(const std::string& path_to_images) {
    this->path_to_images = path_to_images;
}