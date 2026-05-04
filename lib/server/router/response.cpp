// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <router/response.h>

namespace EspWeb
{

    void Response::send(int socket, Response &res)
    {
        res.send(socket);
    }

    void Response::send(int socket)
    {
        std::string header = getHeaders();
        write(socket, header.c_str(), header.size());

        if (!isFileMode)
        {
            write(socket, body.c_str(), body.size());
            return;
        }

        File file = LittleFS.open(filePath.c_str(), "r");
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

    Response &Response::header(const std::string &key, const std::string &value)
    {
        headers[key] = value;
        return *this;
    }

    std::string Response::getHeaders()
    {
        this->header("Content-Length", std::to_string(isFileMode ? fileSize : body.size()));

        std::string header;
        header.reserve(512);
        header = "HTTP/1.1 ";
        header += std::to_string(status_code);
        header += "\r\n";

        for (const auto &h : headers)
        {
            header += h.first;
            header += ": ";
            header += h.second;
            header += "\r\n";
        }

        header += "Connection: close\r\n\r\n";

        return header;
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
        this->status_code = 200;
        return *this;
    }

    Response &Response::NotFound()
    {
        if (this->body.empty())
        {
            this->text("Not Found");
        }
        this->status_code = 404;
        return *this;
    }

    Response &Response::Created()
    {
        if (this->body.empty())
            this->text("Created");
        this->status_code = 201;
        return *this;
    }

    Response &Response::BadRequest()
    {
        if (this->body.empty())
            this->text("Bad Request");
        this->status_code = 400;
        return *this;
    }

    Response &Response::Unauthorized()
    {
        if (this->body.empty())
            this->text("Unauthorized");
        this->status_code = 401;
        return *this;
    }

    Response &Response::Forbidden()
    {
        if (this->body.empty())
            this->text("Forbidden");
        this->status_code = 403;
        return *this;
    }

    Response &Response::InternalServerError()
    {
        if (this->body.empty())
            this->text("Internal Server Error");
        this->status_code = 500;
        return *this;
    }

    Response &Response::Redirect(const std::string &location)
    {
        this->header("Location", location);
        this->status_code = 302;
        return *this;
    }

    Response &Response::status(int status)
    {
        this->status_code = status;
        return *this;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Response body helpers for different content types
     **/
    Response &Response::file(const std::string &path)
    {
        this->binaryFile(path);
        this->header("Content-Type", "text/html; charset=utf-8;");
        return *this;
    }

    Response &Response::binaryFile(const std::string &path)
    {
        File file = LittleFS.open(path.c_str(), "r");
        if (!file)
        {
            Serial.println("❌ File not found - sending 404");
            this->NotFound().text("File Not Found");
            file.close();
            return *this;
        }

        if (file.size() == 0)
        {
            Serial.println("⚠️  WARNING: File size is 0 bytes!");
        }

        this->header("Content-Type", "application/octet-stream");
        this->fileSize = file.size();
        this->filePath = path;
        this->isFileMode = true;

        Serial.printf("✅ File '%s' is ready to be served (size: %d bytes)\n", path.c_str(), this->fileSize);

        file.close();
        return *this;
    }

    Response &Response::text(const std::string &text)
    {
        this->body = text;
        this->isFileMode = false;
        this->header("Content-Type", "text/plain");
        return *this;
    }

    Response &Response::json(const JsonDocument &bodyJson)
    {
        std::string jsonStr;
        serializeJson(bodyJson, jsonStr);

        this->body = jsonStr;
        this->isFileMode = false;
        this->header("Content-Type", "application/json");
        return *this;
    }

    Response &Response::html(const std::string &htmlBody)
    {
        this->text(htmlBody);
        this->header("Content-Type", "text/html; charset=utf-8");
        return *this;
    }

}
