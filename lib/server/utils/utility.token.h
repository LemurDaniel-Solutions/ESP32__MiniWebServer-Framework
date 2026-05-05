// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>
#include <mbedtls/sha256.h>

#include <vector>
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
    extern std::vector<std::string> TOKEN_ACTIONS;

    class Token
    {
    private:
        bool _isValid = true;
        long _expires = -1;

    public:
        static Token NullToken();

        Token() = default;
        Token(std::string name, std::string value, std::vector<std::string> action = {});

        static Token getSessionToken(long seconds, const std::vector<std::string> &action);
        static Token getApiToken(const std::string &name, const std::vector<std::string> &action);

        std::vector<std::string> action;
        std::string value;
        std::string name;

        const long expires();
        bool isPrivileged();
        bool isValid();
        bool isAllowed(const std::string &action);
    };

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

        std::map<std::string, Token> _SESSION_TOKENS;
        std::map<std::string, Token> *_API_TOKENS = nullptr;

        TokenManager() {}
        TokenManager(const TokenManager &) = delete;
        void operator=(const TokenManager &) = delete;

    public:
        static TokenManager &instance()
        {
            static TokenManager _instance;
            return _instance;
        }

        std::string generateSHA256(const std::string &text);

        /*-------------------------------------------------------------------------------------------------
         *
         * Set / Get Credentials
         *
         **/

        void setSalt(const std::string &salt);
        void setCredentials(const std::string &username, const std::string &password);

        bool checkCredentials(const std::string &username, const std::string &password);

        Token checkToken(const std::string &authToken);

        /*-------------------------------------------------------------------------------------------------
         *
         * Session Tokens
         *
         **/

        Token getSessionToken(long seconds, const std::vector<std::string> &actions);
        void removeToken(const std::string &token);

        /*-------------------------------------------------------------------------------------------------
         *
         * API Tokens
         *
         **/

        Token getApiToken(const std::string &name, const std::vector<std::string> &action);
        void removeApiToken(const std::string &name);
        std::vector<Token> listApiTokens();
    };
}