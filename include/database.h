#pragma once

#include <sqlite3.h>
#include <string>

class Database
{
public:
    explicit Database(const std::string& databasePath);
    ~Database();

    bool initialize();

    bool insertUrl(const std::string& code,
                   const std::string& originalUrl);

    bool codeExists(const std::string& code);

    bool getOriginalUrl(const std::string& code,
                        std::string& originalUrl);

    bool incrementAccessCount(const std::string& code);

    bool getStats(const std::string& code,
                  std::string& originalUrl,
                  int& accessCount);

private:
    sqlite3* db_;
    std::string databasePath_;
};