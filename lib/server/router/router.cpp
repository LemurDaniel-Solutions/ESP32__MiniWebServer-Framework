// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <router/router.h>

namespace ESP32WebServer
{

    Router::Router() {}

    void Router::route(const std::string &method, const std::string &path, std::vector<RequestHandler> handlers)
    {
        routes.push_back({method, path, handlers});
    }

    void Router::route(const std::string &method, const std::string &path, RequestHandler handler)
    {
        route(method, path, std::vector<RequestHandler>{handler});
    }

    void Router::use(const std::string &prefix, const RequestHandler &handler)
    {
        const auto &entry = middlewares.find(prefix);
        if (entry != middlewares.end())
        {
            std::vector<RequestHandler> &list = entry->second;
            list.push_back(handler);
        }
        else
        {
            std::vector<RequestHandler> list{handler};
            middlewares.insert({prefix, list});
        }
    }

}
