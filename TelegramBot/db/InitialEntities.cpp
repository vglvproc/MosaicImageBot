#include "InitialEntities.h"
#include "SqliteTable.h"
#include <iostream>

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

    tables.push_back(users);

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
