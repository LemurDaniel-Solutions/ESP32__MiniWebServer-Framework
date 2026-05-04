// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>
#include <mbedtls/sha256.h>

#include <string>
#include <map>

#include <utils/utility.file.h>

#define ADMIN_PERMANENT_TOKENS "/admin_perm_token.json"
#define ADMIN_CREDENTIALS_FILE "/admin_credentials.json"

#define DEFAULT_ADMIN_COOKIE "adminToken"
#define DEFAULT_ADMIN_USER "admin"
#define DEFAULT_ADMIN_PWD "admin"

namespace EspWeb
{
    class TokenManager
    {
    private:
        /*-------------------------------------------------------------------------------------------------
         *
         * Handle config file
         *
         **/

        JsonDocument *_admin = nullptr;

        std::string getValue(const std::string &key);
        void setValue(const std::string &key, const std::string &value);

        std::string generateSHA256(const std::string &text);

        std::map<std::string, unsigned long> _ADMIN_TOKENS;
        std::map<std::string, std::string> *_PERM_TOKENS = nullptr;

        TokenManager() {}
        TokenManager(const TokenManager &) = delete;
        void operator=(const TokenManager &) = delete;

    public:
        static TokenManager &instance()
        {
            static TokenManager _instance;
            return _instance;
        }

        /*-------------------------------------------------------------------------------------------------
         *
         * Set / Get Credentials
         *
         **/

        void setSalt(const std::string &salt);
        void setCredentials(const std::string &username, const std::string &password);

        bool checkCredentials(const std::string &username, const std::string &password);
        std::string generateSHA256(const std::string &text, const std::string &salt);

        /*-------------------------------------------------------------------------------------------------
         *
         * Session Tokens
         *
         **/

        std::string getToken();
        void removeToken(const std::string &token);
        bool checkToken(const std::string &authToken);

        /*-------------------------------------------------------------------------------------------------
         *
         * API Tokens
         *
         **/

        std::string addPermToken(const std::string &name);
        void removePermToken(const std::string &name);
        bool checkPermToken(const std::string &authToken);
        std::vector<std::string> listPermTokens();
    };
}