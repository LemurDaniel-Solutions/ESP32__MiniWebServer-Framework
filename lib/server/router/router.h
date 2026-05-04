// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "mbedtls/sha256.h"

#include <vector>
#include <string>
#include <map>
#include <functional>

#include <router/request.h>
#include <router/response.h>
#include <utils/utility.file.h>

#define ADMIN_PERMANENT_TOKENS "/admin_perm_token.json"
#define ADMIN_CREDENTIALS_FILE "/admin_credentials.json"
#define DEFAULT_ADMIN_COOKIE "adminToken"
#define DEFAULT_ADMIN_USER "admin"
#define DEFAULT_ADMIN_PWD "admin"

namespace EspWeb
{
    using RequestHandler = std::function<void(Request &, Response &)>;

    class TokenManager
    {
    public:
        static TokenManager &instance()
        {
            static TokenManager _instance;
            return _instance;
        }

        bool setSalt(const std::string &salt);
        bool setCredentials(const std::string &username, const std::string &password);
        std::vector<std::string> getCredentials();
        bool checkCredentials(const std::string &username, const std::string &password);
        std::string generateSHA256(const std::string &text, const std::string &salt);

        std::string getToken();
        void addToken(const std::string &token);
        void removeToken(const std::string &token);
        bool checkToken(const std::string &authToken);

        std::string addPermToken(const std::string &name);
        void removePermToken(const std::string &name);
        bool checkPermToken(const std::string &authToken);
        std::vector<std::string> listPermTokens();

        bool isPermTokenValid(Request &req);
        bool isSessionTokenValid(Request &req);

    private:
        TokenManager() {}
        TokenManager(const TokenManager &) = delete;
        void operator=(const TokenManager &) = delete;

        std::map<std::string, unsigned long> _ADMIN_TOKENS;
        std::map<std::string, std::string> _PERM_TOKENS;
        int _hasReadFile = false;
    };

    class Router
    {
    public:
        std::map<std::string, std::vector<RequestHandler>> middlewares;

        struct Route
        {
            std::string method;
            std::string path;
            std::vector<RequestHandler> handler;
        };
        std::vector<Route> routes;

        Router();

        void route(const std::string &method, const std::string &path, std::vector<RequestHandler> handlers);
        void route(const std::string &method, const std::string &path, RequestHandler handler);
        void use(const std::string &prefix, const RequestHandler &handler);

        /*-------------------------------------------------------------------------------------------------
         *
         * Middleware
         *
         **/

        static RequestHandler cors(const std::string &origin = "*");
        static RequestHandler auth();

        /*-------------------------------------------------------------------------------------------------
         *
         * Make Token Handleing available in Router
         *
         **/

        static std::string getSessionToken();

        static std::string getApiToken(const std::string &name);
        static void removeApiToken(const std::string &name);

        static bool isApiTokenValid(Request &req);
        static bool isSessionTokenValid(Request &req);
    };

}
