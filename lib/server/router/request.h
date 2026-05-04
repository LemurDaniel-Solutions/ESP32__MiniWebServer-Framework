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
        uint8_t bytes[256];
        size_t bytesLen = 0;

        void prepend(const uint8_t *data, size_t len)
        {
            bytesLen = std::min(len, sizeof(bytes));
            memcpy(bytes, data, bytesLen);
        }
        int read(void *buf, size_t len)
        {
            size_t total = 0;

            if (bytesLen > 0)
            {
                size_t n = std::min(len, bytesLen);
                memcpy(buf, bytes, n);
                memmove(bytes, bytes + n, bytesLen - n);
                bytesLen -= n;
                total += n;

                if (total >= len)
                    return (int)total;
            }

            int n = ::read(clientSocket, (uint8_t *)buf + total, len - total);
            if (n > 0)
                total += n;

            return (int)total;
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
        JsonDocument &json();
        const std::string &text();
        const std::string &file(const std::string &name);
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
        std::map<std::string, std::string> query;

        RequestBody body;

        static Request parse(int clientSocket);
    };
}
