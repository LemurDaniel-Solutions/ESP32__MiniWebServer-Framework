// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>

#include <router/request.h>
#include <router/response.h>

namespace ESP32WebServer
{
    using RequestHandler = std::function<void(Request &, Response &)>;

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
    };

}
