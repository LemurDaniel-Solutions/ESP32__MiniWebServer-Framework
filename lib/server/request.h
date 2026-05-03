// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>
#include <LittleFS.h>

#include <string>
#include <vector>
#include <map>

#include <utility.file.h>

namespace ESP32WebServer
{

    const size_t BODY_SIZE_TRESHOLD = 8192;

    struct SocketReader
    {
        int clientSocket = -1;
        std::vector<uint8_t> bytes;

        // Allows the prepend back to much read bytes
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

        void clean()
        {
            if (!filePath.empty())
            {
                LittleFS.remove(filePath.c_str());
                Serial.print("Removed file: ");
                Serial.println(filePath.c_str());
            }
        }

        std::string text()
        {
            if (!bodyText.empty())
            {
                return bodyText;
            }

            char chunk[256];
            while (bodyText.size() < contentLength)
            {
                size_t toRead = std::min((size_t)sizeof(chunk), contentLength - bodyText.size());
                int n = socket.read(chunk, toRead);
                if (n <= 0)
                    break;
                bodyText.insert(bodyText.end(), chunk, chunk + n);
            }

            return bodyText;
        }

        JsonDocument json()
        {
            if (bodyJson.size() == 0)
            {
                this->text();
                deserializeJson(bodyJson, bodyText.c_str());
            }

            return bodyJson;
        }

        std::string file()
        {
            if (!filePath.empty())
            {
                return filePath;
            }

            filePath = getTempFolder() + "/" + randomString(4);
            File tmpFile = LittleFS.open(filePath.c_str(), "w", true);
            if (!tmpFile)
            {
                Serial.println("Failed to open temp file for body");
                filePath = "";
                return filePath;
            }

            char chunk[256];
            size_t written = 0;
            while (written < contentLength)
            {
                size_t toRead = std::min((size_t)sizeof(chunk), contentLength - written);
                int n = socket.read(chunk, toRead);
                if (n <= 0)
                    break;
                tmpFile.write((uint8_t *)chunk, n);
            }

            tmpFile.close();
            Serial.printf("Body written to temp file: %s (%zu bytes)\n", filePath.c_str(), written);

            return filePath;
        }

        size_t chunks(uint8_t *chunk, size_t chunkSize)
        {
            if (readSize >= contentLength)
                return 0;

            size_t toRead = std::min(chunkSize, contentLength - readSize);
            size_t n = socket.read(chunk, toRead);
            readSize += n;

            return n;
        }
    };

    class Request
    {
    private:
        /*-------------------------------------------------------------------------------------------------
         *
         * static helper Methods
         *
         *
         **/
        static std::string trim(const std::string &s)
        {
            size_t start = s.find_first_not_of(" \t");
            if (start == std::string::npos)
                return {};
            size_t end = s.find_last_not_of(" \t");
            return s.substr(start, end - start + 1);
        }

        static size_t findBytes(const std::vector<uint8_t> &data, const std::string &pattern, size_t start = 0)
        {
            size_t pat_length = pattern.length();
            if (pat_length == 0 || data.size() < pat_length || start > data.size() - pat_length)
                return std::string::npos;

            for (size_t i = start; i <= data.size() - pat_length; ++i)
            {
                if (memcmp(data.data() + i, pattern.c_str(), pat_length) == 0)
                    return i;
            }

            return std::string::npos;
        }

        static std::string extractString(const std::vector<uint8_t> &data, size_t start, size_t end)
        {
            return std::string(reinterpret_cast<const char *>(data.data() + start), end - start);
        }

        static std::vector<std::string> split(const std::string &text, const std::string &splitter)
        {
            std::vector<std::string> elements;

            size_t pos = 0;
            while (pos < text.size())
            {
                size_t end = text.find(splitter, pos);
                if (end == std::string::npos)
                    end = text.size();

                elements.push_back(trim(text.substr(pos, end - pos)));
                pos = end + splitter.size();
            }

            return elements;
        }

        /*-------------------------------------------------------------------------------------------------
         *
         * Extract Header
         *
         *
         **/
        std::vector<std::string> extractHeader()
        {
            std::vector<std::string> headerRaw;

            char chunk[256];

            std::string header;
            header.reserve(512);

            size_t headerEnd = std::string::npos;

            // ---------------------------------------------------------------------
            // Phase 1: read until \r\n\r\n (complete headers)
            do
            {
                int n = socket.read(chunk, sizeof(chunk));
                if (n <= 0)
                {
                    Serial.println("Failed to read from socket");
                    return {};
                }
                header.append(chunk, n);
                headerEnd = header.find("\r\n\r\n");
            } while (headerEnd == std::string::npos && header.size() < 8192);

            if (headerEnd == std::string::npos)
                return headerRaw;

            // ---------------------------------------------------------------------
            // Phase 2: bytes in headers prepend for next read
            std::vector<uint8_t> body(header.begin() + headerEnd + 4, header.end());
            socket.prepend(body);

            // ---------------------------------------------------------------------
            // Phase 3: split headers into list of lines
            header.resize(headerEnd);
            size_t startOfLine = 0;
            while (startOfLine < header.size())
            {
                size_t endOfLine = header.find("\r\n", startOfLine);
                if (endOfLine == std::string::npos)
                {
                    endOfLine = header.size();
                }

                const std::string line = header.substr(startOfLine, endOfLine - startOfLine);
                headerRaw.push_back(line);
                startOfLine = endOfLine + 2;
            }

            return headerRaw;
        }

    public:
        Request() = default;

        Request(Request &&other) = default;
        Request &operator=(Request &&other) = default;

        Request(const Request &) = delete;
        Request &operator=(const Request &) = delete;

        // Delete temp file on destruktor
        ~Request()
        {
            body.clean();
        }

    private:
        SocketReader socket;

    public:
        int rejected = false;
        std::string error;

        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> cookies;

        RequestBody body;

        static Request parse(int clientSocket)
        {
            Request request;
            request.socket.clientSocket = clientSocket;

            // --- Extract header ---
            Serial.println("Extracing Raw Header");
            std::vector<std::string> headerRaw = request.extractHeader();

            // Move socket (with any buffered body-prefix bytes) into body
            request.body.socket = std::move(request.socket);
            if (headerRaw.empty())
                return request;

            // Split: "GET /path HTTP/1.1"
            std::vector<std::string> firstLine = Request::split(headerRaw[0], " ");
            request.method = firstLine[0];
            request.path = firstLine[1];

            for (size_t i = 1; i < headerRaw.size(); i++)
            {
                std::vector<std::string> line = Request::split(headerRaw[i], ":");
                if (line.size() >= 2)
                    request.headers[line[0]] = line[1];
            }

            if (request.headers.find("Cookie") != request.headers.end())
            {
                std::vector<std::string> cookieHeader = Request::split(request.headers["Cookie"], ";");
                for (size_t i = 0; i < cookieHeader.size(); i++)
                {
                    std::vector<std::string> line = Request::split(cookieHeader[i], "=");
                    if (line.size() >= 2)
                        request.cookies[line[0]] = line[1];
                }
            }

            // --- Extract body ---

            if (request.headers.find("Content-Type") != request.headers.end())
            {
                request.body.contentType = request.headers["Content-Type"];
            }

            auto clIt = request.headers.find("Content-Length");
            if (clIt != request.headers.end())
            {
                for (char c : clIt->second)
                {
                    if (c < '0' || c > '9')
                        break;
                    request.body.contentLength = request.body.contentLength * 10 + (c - '0');
                }
            }

            return request;
        }
    };
}
