// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <router/response.h>

namespace EspWeb
{

    Response &Response::header(const std::string &key, const std::string &value)
    {
        headers[key] = value;
        return *this;
    }

    void Response::sendHeaders()
    {
        if (isHeaderSent)
        {
            return;
        }
        finalized = true;
        isHeaderSent = true;

        std::string header;
        header.reserve(512);
        header = "HTTP/1.1 ";
        header += std::to_string(statusCode);
        header += " ";
        switch (statusCode)
        {
        case 200:
            header += "OK";
            break;
        case 201:
            header += "Created";
            break;
        case 204:
            header += "No Content";
            break;
        case 301:
            header += "Moved Permanently";
            break;
        case 302:
            header += "Found";
            break;
        case 400:
            header += "Bad Request";
            break;
        case 401:
            header += "Unauthorized";
            break;
        case 403:
            header += "Forbidden";
            break;
        case 404:
            header += "Not Found";
            break;
        case 500:
            header += "Internal Server Error";
            break;
        default:
            header += "Unknown";
            break;
        }
        header += "\r\n";
        for (const auto &kv : headers)
        {
            header += kv.first;
            header += ": ";
            header += kv.second;
            header += "\r\n";
        }
        header += "Connection: close\r\n\r\n";
        write(socket, header.c_str(), header.size());
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * streaming
     **/

    void Response::setTimeout(int seconds)
    {
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;

        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }

    void Response::endStream()
    {
        if (!isStreaming || isSent)
            return;

        isSent = true;
        write(socket, "0\r\n\r\n", 5);
    }

    void Response::beginStream()
    {
        if (isSent)
            return;

        finalized = true;
        isStreaming = true;

        int flag = 1;
        setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

        if (headers.find("Content-Type") == headers.end())
            header("Content-Type", "text/plain; charset=utf-8");

        header("Cache-Control", "no-cache");
        header("Transfer-Encoding", "chunked").sendHeaders();
    }

    void Response::sendChunk(const uint8_t *data, size_t len)
    {
        if (isSent || !isStreaming || len == 0)
            return;

        char sizeHex[16];
        int hexLen = snprintf(sizeHex, sizeof(sizeHex), "%X\r\n", len);

        std::string frame;
        frame.reserve(hexLen + len + 2);
        frame.append(sizeHex, hexLen);
        frame.append(reinterpret_cast<const char *>(data), len);
        frame.append("\r\n", 2);
        write(socket, frame.c_str(), frame.size());
    }

    void Response::sendChunk(const std::string &data)
    {
        Serial.println(data.c_str());
        sendChunk(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * sending
     **/

    void Response::send()
    {
        if (isSent || isStreaming)
        {
            return;
        }
        isSent = true;
        finalized = true;
        isStreaming = false;

        // Write Headers
        header("Content-Length", std::to_string(isFileMode ? fileSize : body.size())).sendHeaders();

        // Write Content
        if (!isFileMode)
        {
            write(socket, body.c_str(), body.size());
            return;
        }

        File file = LittleFS.open(filePath.c_str(), "r");
        if (!file)
        {
            Serial.println("❌ File disappeared before send()");
            return;
        }
        char chunk[1460];
        size_t totalSent = 0;
        while (file.available() && totalSent < fileSize)
        {
            size_t n = file.readBytes(chunk, sizeof(chunk));
            if (n > 0)
            {
                write(socket, chunk, n);
                totalSent += n;
            }
            else
                break;
        }
        file.close();
        Serial.printf("File transfer completed: %zu bytes sent\n", totalSent);
    }

    Response &Response::finalize()
    {
        this->finalized = true;
        return *this;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Common status codes
     **/
    Response &Response::OK()
    {
        if (this->body.empty())
        {
            this->text("OK");
        }
        this->statusCode = 200;
        return *this;
    }

    Response &Response::NotFound()
    {
        if (this->body.empty())
        {
            this->text("Not Found");
        }
        this->statusCode = 404;
        return *this;
    }

    Response &Response::Created()
    {
        if (this->body.empty())
            this->text("Created");
        this->statusCode = 201;
        return *this;
    }

    Response &Response::BadRequest()
    {
        if (this->body.empty())
            this->text("Bad Request");
        this->statusCode = 400;
        return *this;
    }

    Response &Response::Unauthorized()
    {
        if (this->body.empty())
            this->text("Unauthorized");
        this->statusCode = 401;
        return *this;
    }

    Response &Response::Forbidden()
    {
        if (this->body.empty())
            this->text("Forbidden");
        this->statusCode = 403;
        return *this;
    }

    Response &Response::InternalServerError()
    {
        if (this->body.empty())
            this->text("Internal Server Error");
        this->statusCode = 500;
        return *this;
    }

    Response &Response::Redirect(const std::string &location)
    {
        this->header("Location", location);
        this->statusCode = 302;
        return *this;
    }

    Response &Response::status(int status)
    {
        this->statusCode = status;
        return *this;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Response body helpers for different content types
     **/
    Response &Response::file(const std::string &path)
    {
        File file = LittleFS.open(path.c_str(), "r");
        if (!file)
        {
            Serial.println("❌ File not found - sending 404");
            this->NotFound().text("File Not Found");
            return *this;
        }

        if (file.size() == 0)
        {
            Serial.println("⚠️  WARNING: File size is 0 bytes!");
        }

        this->fileSize = file.size();
        this->filePath = path;
        this->isFileMode = true;

        Serial.printf("✅ File '%s' is ready to be served (size: %d bytes)\n", path.c_str(), this->fileSize);

        file.close();

        this->status(200);

        std::string ext;
        const auto dot = path.find_last_of('.');
        if (dot != std::string::npos)
        {
            ext = path.substr(dot);
        }

        static const std::map<std::string, const char *> mimeTypes = {
            {".css", "text/css"},
            {".html", "text/html; charset=utf-8"},
            {".ico", "image/x-icon"},
            {".js", "application/javascript"},
            {".json", "application/json"},
            {".jpeg", "image/jpeg"},
            {".jpg", "image/jpeg"},
            {".png", "image/png"},
            {".svg", "image/svg+xml"},
            {".txt", "text/plain; charset=utf-8"},
            {".xml", "application/xml"},
        };

        auto it = mimeTypes.find(ext);
        if (it != mimeTypes.end())
            this->header("Content-Type", it->second);
        else
            this->binary();

        return *this;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Content Type setters
     **/
    Response &Response::binary()
    {
        this->header("Content-Type", "application/octet-stream");
        return *this;
    }

    Response &Response::text()
    {
        this->header("Content-Type", "text/plain; charset=utf-8");
        return *this;
    }

    Response &Response::json()
    {
        this->header("Content-Type", "application/json");
        return *this;
    }

    Response &Response::html()
    {
        this->header("Content-Type", "text/html; charset=utf-8");
        return *this;
    }

    Response &Response::text(const std::string &text)
    {
        this->body = text;
        this->isFileMode = false;
        this->text();
        return *this;
    }

    Response &Response::json(const JsonDocument &bodyJson)
    {
        std::string jsonStr;
        serializeJson(bodyJson, jsonStr);

        this->body = jsonStr;
        this->isFileMode = false;
        this->json();
        return *this;
    }

    Response &Response::html(const std::string &htmlBody)
    {
        this->text(htmlBody);
        this->html();
        return *this;
    }

}
