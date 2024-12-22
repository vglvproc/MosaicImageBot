CXX := g++
CXXFLAGS := -g -Wall -std=c++20

# Include path for the TgBot and other headers
INCLUDE_PATH := -I/usr/local/include

# Path for libraries
LIBRARY_PATH := -L/usr/lib -L/usr/local/lib

# Libraries to link
LIBRARIES := -lTgBot -lssl -lcrypto -lsqlite3 -ldocopt -lcurl

# The build target executable:
TARGET := mosaic_image_bot

# Source files
SOURCES := main.cpp \
           utils/Utils.cpp \
           db/SqliteTable.cpp \
           db/InitialEntities.cpp \
           db/DatabaseManager.cpp \
           commands/AddCategoryCommand.cpp \
           commands/AddImagesToCategoryCommand.cpp \
           commands/AddNoAdsUserCommand.cpp \
           commands/AddUnlimitedAccessUserCommand.cpp \
           commands/GetAvailableLangsCommand.cpp \
           commands/GetTokenCommand.cpp \
           commands/RemoveCategoryCommand.cpp \
           commands/RemoveNoAdsUserCommand.cpp \
           commands/RemoveUnlimitedAccessUserCommand.cpp \
           commands/RunBotCommand.cpp \
           commands/SetCategoryCaptionCommand.cpp \
           commands/SetTokenCommand.cpp \
           core/Subject.cpp \
           requests/RequestsManager.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) $(LIBRARY_PATH) $^ $(LIBRARIES) -o $@

clean:
	rm -f $(TARGET)
