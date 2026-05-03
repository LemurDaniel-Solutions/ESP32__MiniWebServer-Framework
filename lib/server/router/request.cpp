// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <router/request.h>

namespace ESP32WebServer
{

    /*-------------------------------------------------------------------------------------------------
     *
     * RequestBody
     *
     **/

    void RequestBody::clean()
    {
        if (!filePath.empty())
        {
            LittleFS.remove(filePath.c_str());
            Serial.print("Removed file: ");
            Serial.println(filePath.c_str());
        }
    }

    std::string RequestBody::text()
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

    JsonDocument RequestBody::json()
    {
        if (bodyJson.size() == 0)
        {
            this->text();
            deserializeJson(bodyJson, bodyText.c_str());
        }

        return bodyJson;
    }

    std::string RequestBody::file(const std::string &name)
    {
        if (!filePath.empty())
        {
            return filePath;
        }

        filePath = getTempFolder() + "/" + name;
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
            written += n;
        }

        tmpFile.close();
        Serial.printf("Body written to temp file: %s (%zu bytes)\n", filePath.c_str(), written);

        return filePath;
    }

    size_t RequestBody::chunks(uint8_t *chunk, size_t chunkSize)
    {
        if (readSize >= contentLength)
            return 0;

        size_t toRead = std::min(chunkSize, contentLength - readSize);
        size_t n = socket.read(chunk, toRead);
        readSize += n;

        return n;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Request — header extraction
     *
     **/

    std::vector<std::string> Request::extractHeader()
    {
        std::vector<std::string> headerRaw;

        char chunk[256];

        std::string header;
        header.reserve(512);

        size_t headerEnd = std::string::npos;

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

        // Phase 2: bytes after headers prepend for next read
        const uint8_t *bodyStart = reinterpret_cast<const uint8_t *>(header.data()) + headerEnd + 4;
        size_t bodyPrefixLen = header.size() - (headerEnd + 4);
        socket.prepend(bodyStart, bodyPrefixLen);

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

    /*-------------------------------------------------------------------------------------------------
     *
     * Request — destructor and parse
     *
     **/

    Request::~Request()
    {
        body.clean();
    }

    Request Request::parse(int clientSocket)
    {
        Request request;
        request.socket.clientSocket = clientSocket;

        Serial.println("Extracing Raw Header");
        std::vector<std::string> headerRaw = request.extractHeader();

        // Move socket (with any buffered body-prefix bytes) into body
        request.body.socket = std::move(request.socket);
        if (headerRaw.empty())
            return request;

        // Split: "GET /path HTTP/1.1"
        std::vector<std::string> firstLine = split(headerRaw[0], " ");
        request.method = firstLine[0];
        request.path = firstLine[1];

        // Reading URL Parameters
        if (request.path.find("?") != std::string::npos)
        {
            const std::vector<std::string> &segments = split(request.path, "?");
            request.path = segments[0];

            const std::vector<std::string> &query = split(segments[1], "&");
            for (const std::string &param : query)
            {
                const std::vector<std::string> &pair = split(param, "=");
                request.query.insert({pair[0], pair[1]});
            }
        }

        // Reading headers
        for (size_t i = 1; i < headerRaw.size(); i++)
        {
            std::vector<std::string> line = split(headerRaw[i], ":");
            if (line.size() >= 2)
                request.headers[line[0]] = line[1];
        }

        // Parsing Cookie Header
        if (request.headers.find("Cookie") != request.headers.end())
        {
            std::vector<std::string> cookieHeader = split(request.headers["Cookie"], ";");
            for (size_t i = 0; i < cookieHeader.size(); i++)
            {
                std::vector<std::string> line = split(cookieHeader[i], "=");
                if (line.size() >= 2)
                    request.cookies[line[0]] = line[1];
            }
        }

        if (request.headers.find("Content-Type") != request.headers.end())
        {
            request.body.contentType = request.headers["Content-Type"];
        }

        if (request.headers.find("Content-Length") != request.headers.end())
        {
            request.body.contentLength = std::stoul(request.headers["Content-Length"]);
        }

        return request;
    }

}
