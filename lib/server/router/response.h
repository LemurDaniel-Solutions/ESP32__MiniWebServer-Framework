// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <sys/socket.h>

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
        int statusCode = 200;

        // Custom headers to include in the response
        std::map<std::string, std::string> headers;

        // For static file responses, this will be set to the file path to serve
        size_t fileSize = 0;
        std::string filePath;

        bool isFileMode = false;
        std::string body;

        int socket;
        int isSent = false;
        int isHeaderSent = false;
        int isStreaming = false;

    public:
        Response(int socket)
        {
            this->socket = socket;
        }

        /*-------------------------------------------------------------------------------------------------
         *
         * sending & streaming
         **/

        void send();
        void beginStream();
        void endStream();
        void setTimeout(int seconds);
        void sendChunk(const std::string &data);
        void sendChunk(const uint8_t *data, size_t len);

        /*-------------------------------------------------------------------------------------------------
         *
         * Finalize
         **/

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
        void sendHeaders();

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
