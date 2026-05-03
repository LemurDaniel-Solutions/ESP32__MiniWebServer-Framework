// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>
#include <LittleFS.h>

#include <string>
#include <vector>
#include <map>

#include <utils/utility.file.h>

namespace ESP32WebServer
{

    const size_t BODY_SIZE_TRESHOLD = 8192;

    /*-------------------------------------------------------------------------------------------------
     *
     * SocketReader
     *
     **/

    struct SocketReader
    {
        int clientSocket = -1;
        std::vector<uint8_t> bytes;

        void prepend(std::vector<uint8_t> bytes)
        {
            this->bytes = std::move(bytes);
        }
        int read(void *buf, size_t len)
        {
            if (!bytes.empty())
            {
                size_t n = std::min(len, bytes.size());
                memcpy(buf, bytes.data(), n);
                bytes.erase(bytes.begin(), bytes.begin() + n);
                return (int)n;
            }
            return ::read(clientSocket, buf, len);
        }
    };

    /*-------------------------------------------------------------------------------------------------
     *
     * RequestBody
     *
     **/

    class RequestBody
    {
        friend class Request;

    private:
        SocketReader socket;
        std::string bodyText;
        std::string filePath;
        JsonDocument bodyJson;

        size_t readSize = 0;

    public:
        RequestBody() = default;

        size_t contentLength = 0;
        std::string contentType = "application/text";

        void clean();
        std::string text();
        JsonDocument json();
        std::string file();
        size_t chunks(uint8_t *chunk, size_t chunkSize);
    };

    /*-------------------------------------------------------------------------------------------------
     *
     * Request — private static helpers
     *
     **/

    class Request
    {
    private:
        std::vector<std::string> extractHeader();

        SocketReader socket;

        /*-------------------------------------------------------------------------------------------------
         *
         * Request — destructor and parse
         *
         **/
    public:
        Request() = default;

        Request(Request &&other) = default;
        Request &operator=(Request &&other) = default;

        Request(const Request &) = delete;
        Request &operator=(const Request &) = delete;

        ~Request();

        int rejected = false;
        std::string error;

        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> cookies;

        RequestBody body;

        static Request parse(int clientSocket);
    };
}
