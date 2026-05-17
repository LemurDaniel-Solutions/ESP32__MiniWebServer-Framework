// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>

#include <vector>
#include <string>
#include <map>
#include <jsonFileHandler.h>

#include <router/request.h>
#include <router/response.h>
#include <utils/utility.token.h>
#include <utils/utility.file.h>

namespace EspWeb
{
    using RequestHandler = std::function<void(Request &, Response &)>;

    class Router
    {
    public:
        static JsonFileHandler::JsonFileHandler fs;
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

        static std::string getSessionToken(long seconds, const std::vector<std::string> &actions = TOKEN_ACTIONS);

        static std::string getApiToken(const std::string &name, const std::vector<std::string> &actions = TOKEN_ACTIONS);
        static void removeApiToken(const std::string &name);

        static bool isApiTokenValid(Request &req);
        static bool isSessionTokenValid(Request &req);
    };

}
