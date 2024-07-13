#include "InitialEntities.h"
#include "SqliteTable.h"
#include "../utils/Utils.h"
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

    SqliteTable categories(
        "categories",
        {
            {"category_name", DT::TEXT},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "category_name"
    );

    SqliteTable sessions(
        "sessions",
        {
            {"session_id", DT::TEXT},
            {"user_id", DT::TEXT},
            {"current_step", DT::INTEGER},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "session_id"
    );

    tables.push_back(users);
    tables.push_back(languages);
    tables.push_back(categories);
    tables.push_back(sessions);

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

SqliteTable getSessionsTable() {
    using DT = SqliteTable::DataType;

    SqliteTable sessions(
        "sessions",
        {
            {"session_id", DT::TEXT},
            {"user_id", DT::TEXT},
            {"current_step", DT::INTEGER},
            {"adding_timestamp", DT::TEXT},
            {"adding_datetime", DT::TEXT}
        },
        "session_id"
    );
    return sessions;
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

    return true;
}
