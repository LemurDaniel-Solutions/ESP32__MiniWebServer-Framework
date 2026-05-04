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
            if (!req.isApiTokenValid() && !req.isSessionTokenValid())
                res.Unauthorized().finalize();
        };
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Make Token Handleing available in Router
     *
     **/

    std::string Router::getSessionToken()
    {
        return TokenManager::instance().getToken();
    }

    std::string Router::getApiToken(const std::string &name)
    {
        return TokenManager::instance().addPermToken(name);
    }
    void Router::removeApiToken(const std::string &name)
    {
        TokenManager::instance().removePermToken(name);
    }

}
