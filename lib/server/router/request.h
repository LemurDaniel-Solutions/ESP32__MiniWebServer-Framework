// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>
#include <LittleFS.h>

#include <string>
#include <vector>
#include <map>

#include <utils/utility.token.h>
#include <utils/utility.file.h>

namespace EspWeb
{
    /** @brief Body size threshold in bytes above which the body is spooled to a temp file. */
    const size_t BODY_SIZE_TRESHOLD = 8192;

    /*-------------------------------------------------------------------------------------------------
     *
     * SocketReader
     *
     **/

    /**
     * @brief Buffered socket reader that allows prepending bytes back into the read stream.
     *
     * Used to "unread" bytes that were consumed during header parsing but belong to the body.
     */
    struct SocketReader
    {
        /** @brief File descriptor of the client socket. -1 if uninitialized. */
        int clientSocket = -1;

        /** @brief Internal lookahead buffer for prepended bytes. */
        uint8_t bytes[256];

        /** @brief Number of valid bytes currently in the lookahead buffer. */
        size_t bytesLen = 0;

        /**
         * @brief Prepends bytes into the internal buffer so they are returned first by read().
         * @param data Pointer to the bytes to prepend.
         * @param len  Number of bytes to prepend (capped at buffer size).
         */
        void prepend(const uint8_t *data, size_t len)
        {
            bytesLen = std::min(len, sizeof(bytes));
            memcpy(bytes, data, bytesLen);
        }

        /**
         * @brief Reads up to `len` bytes, draining the lookahead buffer first.
         * @param buf Destination buffer.
         * @param len Maximum number of bytes to read.
         * @return Total number of bytes actually read.
         */
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

    /**
     * @brief Lazy-loaded HTTP request body supporting text, JSON, file, and chunked reads.
     *
     * The body is not read until one of the accessor methods is first called.
     * Bodies larger than BODY_SIZE_TRESHOLD are spooled to a LittleFS temp file.
     */
    class RequestBody
    {
        friend class Request;

    private:
        /** @brief Socket reader used to stream body bytes on demand. */
        SocketReader socket;

        /** @brief Cached text body (populated on first call to text()). */
        std::string bodyText;

        /** @brief LittleFS path of the spooled temp file (populated on first call to file()). */
        std::string filePath;

        /** @brief Cached parsed JSON body (populated on first call to json()). */
        JsonDocument bodyJson;

        /** @brief Number of bytes already consumed from the socket. */
        size_t readSize = 0;

    public:
        RequestBody() = default;

        /** @brief Total body size in bytes as reported by the Content-Length header. */
        size_t contentLength = 0;

        /** @brief MIME type from the Content-Type request header. */
        std::string contentType = "application/text";

        /**
         * @brief Reads and parses the body as JSON.
         * @return Reference to the parsed JsonDocument. Cached after the first call.
         */
        JsonDocument &json();

        /**
         * @brief Reads the full body into a string.
         * @return Reference to the body string. Cached after the first call.
         */
        const std::string &text();

        /**
         * @brief Spools the body to a LittleFS temp file under the given name.
         * @param name Suggested filename (used to derive the extension for the temp file).
         * @return LittleFS path to the spooled temp file.
         */
        const std::string &file(const std::string &name);

        /**
         * @brief Reads the next chunk of raw body bytes directly from the socket.
         * @param chunk     Destination buffer.
         * @param chunkSize Maximum number of bytes to read per call.
         * @return Number of bytes actually read; 0 when the body is exhausted.
         */
        size_t chunks(uint8_t *chunk, size_t chunkSize);
    };

    /*-------------------------------------------------------------------------------------------------
     *
     * Request
     *
     **/

    /**
     * @brief Represents a parsed incoming HTTP request.
     *
     * Non-copyable; move-only. Created by the static Request::parse() factory which reads
     * the status line and headers from the socket. The body is read lazily via RequestBody.
     */
    class Request
    {
    private:
        /**
         * @brief Reads and splits raw header lines from the socket.
         * @return Vector of raw header strings.
         */
        std::vector<std::string> extractHeader();

        /** @brief Buffered socket reader used during header parsing. */
        SocketReader socket;

    public:
        Request() = default;

        Request(Request &&other) = default;
        Request &operator=(Request &&other) = default;

        Request(const Request &) = delete;
        Request &operator=(const Request &) = delete;

        ~Request();

        /** @brief True if a middleware has rejected this request (e.g. auth failure). */
        int rejected = false;

        /** @brief Error message set by a middleware when rejecting the request. */
        std::string error;

        /** @brief Validated token extracted from the request (session or API). */
        Token token;

        /** @brief HTTP method in uppercase (e.g. `"GET"`, `"POST"`). */
        std::string method;

        /** @brief Request path without query string (e.g. `"/api/data"`). */
        std::string path;

        /** @brief Map of request header names to their values (lowercase keys). */
        std::map<std::string, std::string> headers;

        /** @brief Map of parsed cookie name → value pairs from the Cookie header. */
        std::map<std::string, std::string> cookies;

        /** @brief Map of URL query parameter names → values (e.g. `?foo=bar`). */
        std::map<std::string, std::string> query;

        /** @brief Map of named route parameters extracted from path patterns (e.g. `:id`). */
        std::map<std::string, std::string> route;

        /** @brief Lazy body reader providing access to the request body. */
        RequestBody body;

        /**
         * @brief Parses an HTTP request from the given client socket.
         * @param clientSocket File descriptor of the accepted client connection.
         * @return Fully populated Request object. Check `rejected` on parse error.
         */
        static Request parse(int clientSocket);
    };
}
