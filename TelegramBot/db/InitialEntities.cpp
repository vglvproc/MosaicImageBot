#include "InitialEntities.h"
#include "SqliteTable.h"
#include "../utils/Utils.h"
#include "../workflow/BotWorkflow.h"
#include <iostream>
#include <string>
#include <vector>
#include <tuple>

std::vector<SqliteTable> getInitialTables() {
    using DT = SqliteTable::DataType;
    std::vector<SqliteTable> tables;

    SqliteTable users(
        "users",
        {
            {"user_id", DT::TEXT},
            {"username", DT::TEXT},
            {"first_name", DT::TEXT},
            {"last_name", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "user_id"
    );

    SqliteTable languages(
        "languages",
        {
            {"language_id", DT::INTEGER},
            {"language_name", DT::TEXT},
            {"language_flag_code", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "language_id"
    );

    SqliteTable categories( // TODO: добавить category_id (INTEGER)
        "categories",
        {
            {"category_name", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "category_name"
    );

    SqliteTable captions(
        "captions",
        {
            {"caption_id", DT::TEXT},
            {"category_name", DT::TEXT},
            {"language_id", DT::INTEGER},
            {"caption", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "caption_id"
    );

    SqliteTable category_paths(
        "category_paths",
        {
            {"id", DT::TEXT},
            {"category_name", DT::TEXT},
            {"category_path", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "id"
    );

    SqliteTable sessions(
        "sessions",
        {
            {"session_id", DT::TEXT},
            {"user_id", DT::TEXT},
            {"current_step", DT::INTEGER},
            {"selected_language_id", DT::INTEGER},
            {"selected_category_id", DT::INTEGER},
            {"selected_size", DT::INTEGER},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "session_id"
    );

    SqliteTable messages(
        "messages",
        {
            {"message_id", DT::INTEGER},
            {"message_type", DT::INTEGER},
            {"language_id", DT::INTEGER},
            {"message", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "message_id"
    );

    SqliteTable no_ads_users(
        "no_ads_users",
        {
            {"user_id", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "user_id"
    );

    SqliteTable unlimited_access_users(
        "unlimited_access_users",
        {
            {"user_id", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "user_id"
    );

    SqliteTable tokens(
        "token",
        {
            {"token_id", DT::TEXT},
            {"token", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "token_id"
    );

    tables.push_back(users);
    tables.push_back(languages);
    tables.push_back(categories);
    tables.push_back(captions);
    tables.push_back(category_paths);
    tables.push_back(sessions);
    tables.push_back(messages);
    tables.push_back(no_ads_users);
    tables.push_back(unlimited_access_users);
    tables.push_back(tokens);

    return tables;
}

SqliteTable getUsersTable() {
    using DT = SqliteTable::DataType;

    SqliteTable users(
        "users",
        {
            {"user_id", DT::TEXT},
            {"username", DT::TEXT},
            {"first_name", DT::TEXT},
            {"last_name", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "user_id"
    );
    return users;
}

SqliteTable getLanguagesTable() {
    using DT = SqliteTable::DataType;

    SqliteTable languages(
        "languages",
        {
            {"language_id", DT::INTEGER},
            {"language_name", DT::TEXT},
            {"language_flag_code", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "language_id"
    );
    return languages;
}

SqliteTable getCategoriesTable() {
    using DT = SqliteTable::DataType;

    SqliteTable categories(
        "categories",
        {
            {"category_name", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "category_name"
    );
    return categories;
}

SqliteTable getCaptionsTable() {
    using DT = SqliteTable::DataType;

    SqliteTable captions(
        "captions",
        {
            {"caption_id", DT::TEXT},
            {"category_name", DT::TEXT},
            {"language_id", DT::INTEGER},
            {"caption", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "caption_id"
    );
    return captions;
}

SqliteTable getCategoryPathsTable() {
    using DT = SqliteTable::DataType;

    SqliteTable category_paths(
        "category_paths",
        {
            {"id", DT::TEXT},
            {"category_name", DT::TEXT},
            {"category_path", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "id"
    );
    return category_paths;
}

SqliteTable getSessionsTable() {
    using DT = SqliteTable::DataType;

    SqliteTable sessions(
        "sessions",
        {
            {"session_id", DT::TEXT},
            {"user_id", DT::TEXT},
            {"current_step", DT::INTEGER},
            {"selected_language_id", DT::INTEGER},
            {"selected_category_id", DT::INTEGER},
            {"selected_size", DT::INTEGER},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "session_id"
    );
    return sessions;
}

SqliteTable getMessagesTable() {
    using DT = SqliteTable::DataType;

    SqliteTable messages(
        "messages",
        {
            {"message_id", DT::INTEGER},
            {"message_type", DT::INTEGER},
            {"language_id", DT::INTEGER},
            {"message", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "message_id"
    );
    return messages;
}

SqliteTable getNoAdsUsersTable() {
    using DT = SqliteTable::DataType;

    SqliteTable no_ads_users(
        "no_ads_users",
        {
            {"user_id", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "user_id"
    );
    return no_ads_users;
}

SqliteTable getUnlimitedAccessUsersTable() {
    using DT = SqliteTable::DataType;

    SqliteTable unlimited_access_users(
        "unlimited_access_users",
        {
            {"user_id", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "user_id"
    );
    return unlimited_access_users;
}

SqliteTable getTokensTable() {
    using DT = SqliteTable::DataType;

    SqliteTable tokens(
        "token",
        {
            {"token_id", DT::TEXT},
            {"token", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "token_id"
    );
    return tokens;
}

bool initLanguagesTable(DatabaseManager& dbManager) {
    SqliteTable table = getLanguagesTable();

    std::vector<std::tuple<int, std::string, std::string>> languages = {
        std::make_tuple(1, std::string("Русский"), std::string("🇷🇺")),
        std::make_tuple(2, std::string("English"), std::string("🇺🇸")),
        std::make_tuple(3, std::string("Deutsch"), std::string("🇩🇪")),
        std::make_tuple(4, std::string("Français"), std::string("🇫🇷")),
        std::make_tuple(5, std::string("Español"), std::string("🇪🇸")),
    };

    for (const auto& [id, name, flag] : languages) {
        std::vector<SqliteTable::FieldValue> row;
        row.push_back({{"language_id", SqliteTable::DataType::INTEGER}, id});
        row.push_back({{"language_name", SqliteTable::DataType::TEXT}, name});
        row.push_back({{"language_flag_code", SqliteTable::DataType::TEXT}, flag});
        long long current_timestamp = getCurrentTimestamp();
        row.push_back({{"adding_timestamp", SqliteTable::DataType::TEXT}, std::to_string(current_timestamp)});
        row.push_back({{"adding_datetime", SqliteTable::DataType::TEXT}, getFormatTimestampWithMilliseconds(current_timestamp)});

        std::string insertSQL = table.generateInsertSQL(row, true);

        if (!dbManager.executeSQL(insertSQL)) {
            std::cerr << "Failed to insert data into languages table: " << name << std::endl;
            return false;
        }
    }

    std::vector<std::string> selectThemeMessages = {
        std::string("Пожалуйста, выберите тему картинок, из которых будет составляться мозаика"),
        std::string("Please select the theme of the images that will be used to create the mosaic"),
        std::string("Bitte wählen Sie das Thema der Bilder aus, aus denen das Mosaik erstellt wird"),
        std::string("Veuillez choisir le thème des images qui seront utilisées pour créer la mosaïque"),
        std::string("Por favor, seleccione el tema de las imágenes que se utilizarán para crear el mosaico"),
    };

    int total_index = 0;
    int index = 1;

    table = getMessagesTable();

    for (const auto& message : selectThemeMessages) {
        std::vector<SqliteTable::FieldValue> row;
        row.push_back({{"message_id", SqliteTable::DataType::INTEGER}, total_index++});
        row.push_back({{"message_type", SqliteTable::DataType::INTEGER}, (int)BotWorkflow::WorkflowMessage::STEP_SELECT_THEME_MESSAGE});
        row.push_back({{"language_id", SqliteTable::DataType::INTEGER}, index});
        row.push_back({{"message", SqliteTable::DataType::TEXT}, message});
        long long current_timestamp = getCurrentTimestamp();
        row.push_back({{"adding_timestamp", SqliteTable::DataType::TEXT}, std::to_string(current_timestamp)});
        row.push_back({{"adding_datetime", SqliteTable::DataType::TEXT}, getFormatTimestampWithMilliseconds(current_timestamp)});

        std::string insertSQL = table.generateInsertSQL(row, true);

        if (!dbManager.executeSQL(insertSQL)) {
            std::cerr << "Failed to insert data into messages table: " << message << std::endl;
            return false;
        }
        index++;
    }

    std::vector<std::string> antimosaicMessages = {
        std::string("Анти-мозаика"),
        std::string("Anti-mosaic"),
        std::string("Anti-Mosaik"),
        std::string("Anti-mosaïque"),
        std::string("Anti-mosaico"),
    };

    index = 1;

    table = getMessagesTable();

    for (const auto& message : antimosaicMessages) {
        std::vector<SqliteTable::FieldValue> row;
        row.push_back({{"message_id", SqliteTable::DataType::INTEGER}, total_index++});
        row.push_back({{"message_type", SqliteTable::DataType::INTEGER}, (int)BotWorkflow::WorkflowMessage::CAPTION_ANTI_MOSAIC});
        row.push_back({{"language_id", SqliteTable::DataType::INTEGER}, index});
        row.push_back({{"message", SqliteTable::DataType::TEXT}, message});
        long long current_timestamp = getCurrentTimestamp();
        row.push_back({{"adding_timestamp", SqliteTable::DataType::TEXT}, std::to_string(current_timestamp)});
        row.push_back({{"adding_datetime", SqliteTable::DataType::TEXT}, getFormatTimestampWithMilliseconds(current_timestamp)});

        std::string insertSQL = table.generateInsertSQL(row, true);
        std::cout << insertSQL << std::endl;

        if (!dbManager.executeSQL(insertSQL)) {
            std::cerr << "Failed to insert data into messages table: " << message << std::endl;
            return false;
        }
        index++;
    }

    std::vector<std::string> selectSizeMessages = {
        std::string("Пожалуйста, выберите размер плитки"),
        std::string("Please select the size of the tile"),
        std::string("Bitte wählen Sie die Größe der Kachel"),
        std::string("Veuillez choisir la taille de la tuile"),
        std::string("Por favor, seleccione el tamaño de la baldosa"),
    };

    index = 1;

    table = getMessagesTable();

    for (const auto& message : selectSizeMessages) {
        std::vector<SqliteTable::FieldValue> row;
        row.push_back({{"message_id", SqliteTable::DataType::INTEGER}, total_index++});
        row.push_back({{"message_type", SqliteTable::DataType::INTEGER}, (int)BotWorkflow::WorkflowMessage::STEP_SELECT_SIZE});
        row.push_back({{"language_id", SqliteTable::DataType::INTEGER}, index});
        row.push_back({{"message", SqliteTable::DataType::TEXT}, message});
        long long current_timestamp = getCurrentTimestamp();
        row.push_back({{"adding_timestamp", SqliteTable::DataType::TEXT}, std::to_string(current_timestamp)});
        row.push_back({{"adding_datetime", SqliteTable::DataType::TEXT}, getFormatTimestampWithMilliseconds(current_timestamp)});

        std::string insertSQL = table.generateInsertSQL(row, true);
        std::cout << insertSQL << std::endl;

        if (!dbManager.executeSQL(insertSQL)) {
            std::cerr << "Failed to insert data into messages table: " << message << std::endl;
            return false;
        }
        index++;
    }

    std::vector<std::string> askPhotoMessages = {
        std::string("Пожалуйста, загрузите фотографию"),
        std::string("Please upload a photo"),
        std::string("Bitte laden Sie ein Foto hoch"),
        std::string("Veuillez télécharger une photo"),
        std::string("Por favor, sube una foto"),
    };

    index = 1;

    table = getMessagesTable();

    for (const auto& message : askPhotoMessages) {
        std::vector<SqliteTable::FieldValue> row;
        row.push_back({{"message_id", SqliteTable::DataType::INTEGER}, total_index++});
        row.push_back({{"message_type", SqliteTable::DataType::INTEGER}, (int)BotWorkflow::WorkflowMessage::STEP_ADD_PHOTO_MESSAGE});
        row.push_back({{"language_id", SqliteTable::DataType::INTEGER}, index});
        row.push_back({{"message", SqliteTable::DataType::TEXT}, message});
        long long current_timestamp = getCurrentTimestamp();
        row.push_back({{"adding_timestamp", SqliteTable::DataType::TEXT}, std::to_string(current_timestamp)});
        row.push_back({{"adding_datetime", SqliteTable::DataType::TEXT}, getFormatTimestampWithMilliseconds(current_timestamp)});

        std::string insertSQL = table.generateInsertSQL(row, true);

        if (!dbManager.executeSQL(insertSQL)) {
            std::cerr << "Failed to insert data into messages table: " << message << std::endl;
            return false;
        }
        index++;
    }

    return true;
}
