#include "crow.h"
#include "database.h"

#include <iostream>
#include <random>
#include <string>

std::string generateCode(std::size_t length)
{
    static const std::string characters =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    static std::random_device randomDevice;
    static std::mt19937 generator(randomDevice());

    static std::uniform_int_distribution<std::size_t> distribution(
        0,
        characters.size() - 1
    );

    std::string code;
    code.reserve(length);

    for (std::size_t i = 0; i < length; ++i)
    {
        code += characters[distribution(generator)];
    }

    return code;
}

bool isValidUrl(const std::string& url)
{
    return url.starts_with("http://") ||
           url.starts_with("https://");
}

int main()
{
    Database database("data/urls.db");

    if (!database.initialize())
    {
        std::cerr << "Database initialization failed."
                  << std::endl;

        return 1;
    }

    crow::SimpleApp app;

    // Health check
    CROW_ROUTE(app, "/")
    ([] {
        return "bombo";
    });

    // Create shortened URL
    CROW_ROUTE(app, "/shorten")
        .methods(crow::HTTPMethod::POST)
    ([&database](const crow::request& request)
    {
        auto body = crow::json::load(request.body);

        if (!body)
        {
            crow::json::wvalue response;
            response["error"] = "Invalid JSON";

            return crow::response(400, response);
        }

        if (!body.has("url") ||
            body["url"].t() != crow::json::type::String)
        {
            crow::json::wvalue response;
            response["error"] =
                "Request must contain a 'url' string";

            return crow::response(400, response);
        }

        std::string originalUrl = body["url"].s();

        if (!isValidUrl(originalUrl))
        {
            crow::json::wvalue response;
            response["error"] =
                "URL must start with http:// or https://";

            return crow::response(400, response);
        }

        std::string code;

        // Generate a new code until an unused one is found.
        do
        {
            code = generateCode(6);
        }
        while (database.codeExists(code));

        if (!database.insertUrl(code, originalUrl))
        {
            crow::json::wvalue response;
            response["error"] =
                "Failed to save shortened URL";

            return crow::response(500, response);
        }

        crow::json::wvalue response;

        response["code"] = code;

        response["short_url"] =
            "http://localhost:8080/" + code;

        response["original_url"] = originalUrl;

        return crow::response(201, response);
    });

    // URL statistics
    CROW_ROUTE(app, "/<string>/stats")
    ([&database](std::string code)
    {
        std::string originalUrl;
        int accessCount = 0;

        if (!database.getStats(
                code,
                originalUrl,
                accessCount))
        {
            crow::json::wvalue error;
            error["error"] = "Short URL not found";

            return crow::response(404, error);
        }

        crow::json::wvalue response;

        response["code"] = code;
        response["original_url"] = originalUrl;
        response["access_count"] = accessCount;

        return crow::response(200, response);
    });

    // Redirect short URL
    CROW_ROUTE(app, "/<string>")
    ([&database](crow::response& response,
                 std::string code)
    {
        std::string originalUrl;

        if (!database.getOriginalUrl(
                code,
                originalUrl))
        {
            crow::json::wvalue error;
            error["error"] = "Short URL not found";

            response.code = 404;
            response.write(error.dump());
            response.end();

            return;
        }

        if (!database.incrementAccessCount(code))
        {
            response.code = 500;
            response.write(
                "Failed to update access count"
            );
            response.end();

            return;
        }

        response.code = 302;

        response.set_header(
            "Location",
            originalUrl
        );

        response.end();
    });

    std::cout
        << "Server running on http://localhost:8080"
        << std::endl;

    app.port(8080)
       .multithreaded()
       .run();
}