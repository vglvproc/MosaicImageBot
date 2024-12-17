#include "SqliteTable.h"
#include <sstream>
#include <iomanip>

SqliteTable::SqliteTable(const std::string& name, const std::vector<Field>& fields, const std::string& primaryKey)
    : name_(name), fields_(fields), primaryKey_(primaryKey) {}

std::string SqliteTable::createTableSQL() const {
    std::ostringstream sql;
    sql << "CREATE TABLE " << name_ << " (";
    for (size_t i = 0; i < fields_.size(); ++i) {
        sql << fields_[i].name << " " << dataTypeToString(fields_[i].type);
        if (i < fields_.size() - 1) {
            sql << ", ";
        }
    }
    if (!primaryKey_.empty()) {
        sql << ", PRIMARY KEY (" << primaryKey_ << ")";
    }
    sql << ")";
    return sql.str();
}

std::string SqliteTable::dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INTEGER: return "INTEGER";
        case DataType::REAL: return "REAL";
        case DataType::TEXT: return "TEXT";
        case DataType::BLOB: return "BLOB";
        default: return "UNKNOWN";
    }
}

std::string SqliteTable::getName() const {
    return this->name_;
}

std::string SqliteTable::getPrimaryKey() const {
    return this->primaryKey_;
}

std::vector<SqliteTable::Field> SqliteTable::getFields() const {
    return this->fields_;
}

std::vector<SqliteTable::FieldValue> SqliteTable::getEmptyRow() const {
    std::vector<FieldValue> row;
    for (const auto& field : fields_) {
        std::variant<int, double, std::string, std::vector<unsigned char>> value;
        switch (field.type) {
            case DataType::INTEGER:
                value = int{};
                break;
            case DataType::REAL:
                value = double{};
                break;
            case DataType::TEXT:
                value = std::string{};
                break;
            case DataType::BLOB:
                value = std::vector<unsigned char>{};
                break;
        }
        row.push_back({field, value});
    }
    return row;
}

std::string SqliteTable::generateInsertSQL(const std::vector<FieldValue>& row, bool doCheckExists) const {
    std::ostringstream sql;
    sql << (doCheckExists ? "INSERT OR IGNORE INTO " : "INSERT INTO ") << name_ << " (";

    for (size_t i = 0; i < row.size(); ++i) {
        sql << row[i].field.name;
        if (i < row.size() - 1) {
            sql << ", ";
        }
    }

    sql << ") VALUES (";

    for (size_t i = 0; i < row.size(); ++i) {
        const auto& value = row[i].value;
        switch (row[i].field.type) {
            case DataType::INTEGER:
                sql << std::get<int>(value);
                break;
            case DataType::REAL:
                sql << std::get<double>(value);
                break;
            case DataType::TEXT:
                sql << "'" << SqliteTable::escapeString(std::get<std::string>(value)) << "'";
                break;
            case DataType::BLOB: {
                const auto& blob = std::get<std::vector<unsigned char>>(value);
                sql << "x'";
                for (const auto& byte : blob) {
                    sql << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
                }
                sql << "'";
                break;
            }
        }
        if (i < row.size() - 1) {
            sql << ", ";
        }
    }

    sql << ")";
    return sql.str();
}

std::string SqliteTable::generateUpdateSQL(const std::vector<FieldValue>& updateRow, const std::vector<FieldValue>& whereRow) const {
    std::ostringstream sql;
    sql << "UPDATE " << name_ << " SET ";

    for (size_t i = 0; i < updateRow.size(); ++i) {
        sql << updateRow[i].field.name << " = ";
        const auto& value = updateRow[i].value;
        switch (updateRow[i].field.type) {
            case DataType::INTEGER:
                sql << std::get<int>(value);
                break;
            case DataType::REAL:
                sql << std::get<double>(value);
                break;
            case DataType::TEXT:
                sql << "'" << SqliteTable::escapeString(std::get<std::string>(value)) << "'";
                break;
            case DataType::BLOB: {
                const auto& blob = std::get<std::vector<unsigned char>>(value);
                sql << "x'";
                for (const auto& byte : blob) {
                    sql << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
                }
                sql << "'";
                break;
            }
        }
        if (i < updateRow.size() - 1) {
            sql << ", ";
        }
    }

    if (!whereRow.empty()) {
        sql << " WHERE ";
        for (size_t i = 0; i < whereRow.size(); ++i) {
            sql << whereRow[i].field.name << " = ";
            const auto& value = whereRow[i].value;
            switch (whereRow[i].field.type) {
                case DataType::INTEGER:
                    sql << std::get<int>(value);
                    break;
                case DataType::REAL:
                    sql << std::get<double>(value);
                    break;
                case DataType::TEXT:
                    sql << "'" << SqliteTable::escapeString(std::get<std::string>(value)) << "'";
                    break;
                case DataType::BLOB: {
                    const auto& blob = std::get<std::vector<unsigned char>>(value);
                    sql << "x'";
                    for (const auto& byte : blob) {
                        sql << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
                    }
                    sql << "'";
                    break;
                }
            }
            if (i < whereRow.size() - 1) {
                sql << " AND ";
            }
        }
    }

    return sql.str();
}

std::string SqliteTable::generateSelectAllSQL() const {
    std::ostringstream sql;
    sql << "SELECT * FROM " << this->name_;
    return sql.str();
}

std::string SqliteTable::generateSelectSQL(const std::vector<FieldValue>& selectRow, const std::vector<FieldValue>& whereRow, const std::vector<FieldValue>& orderByRow, int top) const {
std::ostringstream sql;
    sql << "SELECT ";

    if (top != 0) {
        sql << "LIMIT " << top << " ";
    }

    if (selectRow.empty()) {
        sql << "*";
    } else {
        for (size_t i = 0; i < selectRow.size(); ++i) {
            sql << selectRow[i].field.name;
            if (i < selectRow.size() - 1) {
                sql << ", ";
            }
        }
    }

    sql << " FROM " << name_;

    if (!whereRow.empty()) {
        sql << " WHERE ";
        for (size_t i = 0; i < whereRow.size(); ++i) {
            sql << whereRow[i].field.name << " = ";
            const auto& value = whereRow[i].value;
            switch (whereRow[i].field.type) {
                case DataType::INTEGER:
                    sql << std::get<int>(value);
                    break;
                case DataType::REAL:
                    sql << std::get<double>(value);
                    break;
                case DataType::TEXT:
                    sql << "'" << SqliteTable::escapeString(std::get<std::string>(value)) << "'";
                    break;
                case DataType::BLOB: {
                    const auto& blob = std::get<std::vector<unsigned char>>(value);
                    sql << "x'";
                    for (const auto& byte : blob) {
                        sql << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
                    }
                    sql << "'";
                    break;
                }
            }
            if (i < whereRow.size() - 1) {
                sql << " AND ";
            }
        }
    }

    if (!orderByRow.empty()) {
        sql << " ORDER BY ";
        for (size_t i = 0; i < orderByRow.size(); ++i) {
            sql << orderByRow[i].field.name;
            if (i < orderByRow.size() - 1) {
                sql << ", ";
            }
        }
    }

    return sql.str();
}

std::string SqliteTable::generateDeleteSQL(const std::vector<FieldValue>& whereRow) const {
    std::ostringstream sql;
    sql << "DELETE FROM " << name_;

    if (!whereRow.empty()) {
        sql << " WHERE ";
        for (size_t i = 0; i < whereRow.size(); ++i) {
            sql << whereRow[i].field.name << " = ";
            const auto& value = whereRow[i].value;
            switch (whereRow[i].field.type) {
                case DataType::INTEGER:
                    sql << std::get<int>(value);
                    break;
                case DataType::REAL:
                    sql << std::get<double>(value);
                    break;
                case DataType::TEXT:
                    sql << "'" << SqliteTable::escapeString(std::get<std::string>(value)) << "'";
                    break;
                case DataType::BLOB: {
                    const auto& blob = std::get<std::vector<unsigned char>>(value);
                    sql << "x'";
                    for (const auto& byte : blob) {
                        sql << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
                    }
                    sql << "'";
                    break;
                }
            }
            if (i < whereRow.size() - 1) {
                sql << " AND ";
            }
        }
    }

    return sql.str();
}

std::string SqliteTable::escapeString(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size() * 2);
    for (char c : value) {
        if (c == '\'') {
            escaped += "''";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

std::string SqliteTable::generatePragmaTableInfoSQL() const {
    std::ostringstream sql;
    sql << "PRAGMA table_info(\"" << this->name_ << "\");";
    return sql.str();
}

std::string SqliteTable::generateDropTableSQL() const {
    std::ostringstream sql;
    sql << "DROP TABLE IF EXISTS " << this->name_ << ";";
    return sql.str();
}