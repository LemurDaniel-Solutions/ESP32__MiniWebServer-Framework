// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>

#include <LittleFS.h>
#include <string>
#include <map>

namespace EspWeb
{
    class Response
    {
    private:
        // Flag to indicate if the response has been finalized by some middleware or handler, preventing further modifications
        int finalized = false;

        // HTTP status code for the response (e.g., 200 for OK, 404 for Not Found)
        int status_code = 200;

        // Custom headers to include in the response
        std::map<std::string, std::string> headers;

        // For static file responses, this will be set to the file path to serve
        size_t fileSize;
        std::string filePath;

        bool isFileMode = false;
        std::string body;

    private:
        void send(int socket);

    public:
        static void send(int socket, Response &res);

        Response &finalize();
        int isFinalized()
        {
            return finalized;
        }

        /*-------------------------------------------------------------------------------------------------
         *
         * Headers
         **/
        Response &header(const std::string &key, const std::string &value);
        std::string getHeaders();

        /*-------------------------------------------------------------------------------------------------
         *
         * Common status codes
         **/
        Response &status(int status);

        Response &OK();
        Response &Created();
        Response &BadRequest();
        Response &Unauthorized();
        Response &Forbidden();
        Response &NotFound();
        Response &InternalServerError();
        Response &Redirect(const std::string &location);

        /*-------------------------------------------------------------------------------------------------
         *
         * Response body helpers for different content types
         **/
        Response &file(const std::string &path);
        Response &text(const std::string &text);
        Response &json(const JsonDocument &bodyJson);
        Response &html(const std::string &htmlBody);

        /*-------------------------------------------------------------------------------------------------
         *
         * Content Type setters
         **/
        Response &binary();
        Response &text();
        Response &json();
        Response &html();
    };
}
