#pragma once
#include <string>
#include <vector>
#include <variant>

class SqliteTable {
public:
    enum class DataType {
        INTEGER,
        REAL,
        TEXT,
        BLOB
    };

    struct Field {
        std::string name;
        DataType type;
    };

    struct FieldValue {
        Field field;
        std::variant<int, double, std::string, std::vector<unsigned char>> value;
    };

    SqliteTable(const std::string& name, const std::vector<Field>& fields, const std::string& primaryKey);
    std::string createTableSQL() const;
    static std::string dataTypeToString(DataType type);
    std::string getName() const;
    std::string getPrimaryKey() const;
    std::vector<Field> getFields() const;

    std::vector<FieldValue> getEmptyRow() const;
    std::string generateInsertSQL(const std::vector<FieldValue>& row, bool doCheckExists) const;
    std::string generateUpdateSQL(const std::vector<FieldValue>& updateRow, const std::vector<FieldValue>& whereRow) const;
    std::string generateSelectAllSQL() const;
    std::string generateSelectSQL(const std::vector<FieldValue>& selectRow, const std::vector<FieldValue>& whereRow) const;
    std::string generatePragmaTableInfoSQL() const;
    std::string generateDropTableSQL() const;

private:
    std::string name_;
    std::vector<Field> fields_;
    std::string primaryKey_;

    static std::string escapeString(const std::string& value);
};