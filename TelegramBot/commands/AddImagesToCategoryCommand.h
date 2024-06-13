#pragma once
#include "Command.h"
#include <string>

class AddImagesToCategoryCommand : public Command {
public:
    AddImagesToCategoryCommand(const std::string& category_name, const std::string& path_to_images);

    std::string getCategoryName() const;
    std::string getPathToImages() const;

    void setCategoryName(const std::string& category_name);
    void setPathToImages(const std::string& path_to_images);

private:
    std::string category_name;
    std::string path_to_images;
};