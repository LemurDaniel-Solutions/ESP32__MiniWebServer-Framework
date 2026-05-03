// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>

#include <LittleFS.h>
#include <string>
#include <map>

namespace ESP32WebServer
{
    class Response
    {
    public:
        static void send(int socket, Response &res);

        // Flag to indicate if the response has been finalized by some middleware or handler, preventing further modifications
        int finalized = false;

        Response &finalize();

        std::string responseMode = "body"; // "body" or "file"

        // HTTP status code for the response (e.g., 200 for OK, 404 for Not Found)
        int status_code = 200;

        // Custom headers to include in the response
        std::map<std::string, std::string> headers;

        // Default content type is text/plain, but can be set to application/json or others as needed
        std::string body;

        // For static file responses, this will be set to the file path to serve
        size_t fileSize;
        std::string filePath;

        Response &header(const std::string &key, const std::string &value);
        std::string getHeaders();

        /*-------------------------------------------------------------------------------------------------
         *
         * Common status codes
         **/
        Response &OK();
        Response &NotFound();
        Response &InternalServerError();
        Response &status(int status);

        /*-------------------------------------------------------------------------------------------------
         *
         * Response body helpers for different content types
         **/
        Response &file(const std::string &path);
        Response &binaryFile(const std::string &path);
        Response &text(const std::string &text);
        Response &json(const JsonDocument &bodyJson);
        Response &html(const std::string &htmlBody);

    private:
        void send(int socket);
    };
}
