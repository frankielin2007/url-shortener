#include "database.h"

#include <filesystem>
#include <iostream>

Database::Database(const std::string& databasePath)
    : db_(nullptr),
      databasePath_(databasePath)
{
}

Database::~Database()
{
    if (db_ != nullptr)
    {
        sqlite3_close(db_);
    }
}

bool Database::initialize()
{
    std::filesystem::path path(databasePath_);

    std::filesystem::create_directories(path.parent_path());

    int result = sqlite3_open(
        databasePath_.c_str(),
        &db_
    );

    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to open database: "
                  << sqlite3_errmsg(db_)
                  << std::endl;

        return false;
    }

    const char* createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS urls (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            code TEXT UNIQUE NOT NULL,
            original_url TEXT NOT NULL,
            access_count INTEGER NOT NULL DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* errorMessage = nullptr;

    result = sqlite3_exec(
        db_,
        createTableSQL,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to create table: "
                  << errorMessage
                  << std::endl;

        sqlite3_free(errorMessage);

        return false;
    }

    return true;
}

bool Database::insertUrl(const std::string& code,
                         const std::string& originalUrl)
{
    const char* sql =
        "INSERT INTO urls (code, original_url) "
        "VALUES (?, ?);";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        db_,
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare insert statement: "
                  << sqlite3_errmsg(db_)
                  << std::endl;

        return false;
    }

    sqlite3_bind_text(
        statement,
        1,
        code.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        2,
        originalUrl.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}

bool Database::codeExists(const std::string& code)
{
    const char* sql =
        "SELECT 1 "
        "FROM urls "
        "WHERE code = ? "
        "LIMIT 1;";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        db_,
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(
        statement,
        1,
        code.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_ROW;
}

bool Database::getOriginalUrl(const std::string& code,
                              std::string& originalUrl)
{
    const char* sql =
        "SELECT original_url "
        "FROM urls "
        "WHERE code = ? "
        "LIMIT 1;";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        db_,
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(
        statement,
        1,
        code.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    result = sqlite3_step(statement);

    if (result == SQLITE_ROW)
    {
        const unsigned char* value =
            sqlite3_column_text(statement, 0);

        if (value != nullptr)
        {
            originalUrl =
                reinterpret_cast<const char*>(value);
        }

        sqlite3_finalize(statement);

        return true;
    }

    sqlite3_finalize(statement);

    return false;
}

bool Database::incrementAccessCount(const std::string& code)
{
    const char* sql =
        "UPDATE urls "
        "SET access_count = access_count + 1 "
        "WHERE code = ?;";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        db_,
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(
        statement,
        1,
        code.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}

bool Database::getStats(const std::string& code,
                        std::string& originalUrl,
                        int& accessCount)
{
    const char* sql =
        "SELECT original_url, access_count "
        "FROM urls "
        "WHERE code = ? "
        "LIMIT 1;";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        db_,
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(
        statement,
        1,
        code.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    result = sqlite3_step(statement);

    if (result == SQLITE_ROW)
    {
        const unsigned char* url =
            sqlite3_column_text(statement, 0);

        if (url != nullptr)
        {
            originalUrl =
                reinterpret_cast<const char*>(url);
        }

        accessCount =
            sqlite3_column_int(statement, 1);

        sqlite3_finalize(statement);

        return true;
    }

    sqlite3_finalize(statement);

    return false;
}