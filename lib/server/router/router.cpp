// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <router/router.h>

namespace EspWeb
{

    /*-------------------------------------------------------------------------------------------------
     *
     * Router
     *
     **/

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

    /*-------------------------------------------------------------------------------------------------
     *
     * Middleware
     *
     **/

    RequestHandler Router::cors(const std::string &origin)
    {
        return [origin](Request &req, Response &res)
        {
            res.header("Access-Control-Allow-Origin", origin);
        };
    }

    RequestHandler Router::auth()
    {
        return [](Request &req, Response &res)
        {
            if (!req.token.isValid())
                res.Unauthorized().finalize();
        };
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Make Token Handleing available in Router
     *
     **/

    std::string Router::getSessionToken(long seconds, const std::vector<std::string> &actions)
    {
        return TokenManager::instance().getSessionToken(seconds, actions).value;
    }

    std::string Router::getApiToken(const std::string &name, const std::vector<std::string> &actions)
    {
        return TokenManager::instance().getApiToken(name, actions).value;
    }

    void Router::removeApiToken(const std::string &name)
    {
        TokenManager::instance().removeApiToken(name);
    }

}
