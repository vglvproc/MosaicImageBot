CXX := g++
CXXFLAGS := -g -Wall -std=c++20

# Include path for the TgBot and other headers
INCLUDE_PATH := -I/usr/local/include

# Path for libraries
LIBRARY_PATH := -L/usr/lib -L/usr/local/lib

# Libraries to link
LIBRARIES := -lTgBot -lssl -lcrypto -lsqlite3 -ldocopt

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
           commands/GetAvailableLangsCommand.cpp \
           commands/RemoveCategoryCommand.cpp \
           commands/RunBotCommand.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) $(LIBRARY_PATH) $^ $(LIBRARIES) -o $@

clean:
	rm -f $(TARGET)
