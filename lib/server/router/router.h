// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>

#include <vector>
#include <string>
#include <map>

#include <router/request.h>
#include <router/response.h>
#include <utils/utility.token.h>
#include <utils/utility.file.h>

namespace EspWeb
{
    /** @brief Function signature for all request handler and middleware functions. */
    using RequestHandler = std::function<void(Request &, Response &)>;

    /**
     * @brief Groups related routes and middleware into a mountable unit.
     *
     * A Router is constructed independently and then passed to MiniServer::registerRouter()
     * to register all its routes and middleware at once.
     */
    class Router
    {
    public:
        /** @brief Shared LittleFS file handler available to all route handlers. */
        static FileHandler fs;

        /**
         * @brief Describes a single registered route.
         */
        struct Route
        {
            /** @brief HTTP method string (e.g. `"GET"`, `"POST"`). */
            std::string method;

            /** @brief URL path pattern, supports `:param` segments. */
            std::string path;

            /** @brief Ordered list of handler functions for this route. */
            std::vector<RequestHandler> handler;
        };

        /** @brief All routes registered on this router. */
        std::vector<Route> routes;

        /** @brief Middleware chains keyed by URL prefix. */
        std::map<std::string, std::vector<RequestHandler>> middlewares;

        Router();

        /**
         * @brief Registers a route with multiple chained handler functions.
         * @param method   HTTP method string.
         * @param path     URL path pattern.
         * @param handlers Ordered list of handler functions.
         */
        void route(const std::string &method, const std::string &path, std::vector<RequestHandler> handlers);

        /**
         * @brief Registers a route with a single handler function.
         * @param method  HTTP method string.
         * @param path    URL path pattern.
         * @param handler Handler function.
         */
        void route(const std::string &method, const std::string &path, RequestHandler handler);

        /**
         * @brief Adds a middleware scoped to a URL prefix.
         * @param prefix  URL prefix to match (e.g. `"/api"`).
         * @param handler Middleware function executed before matched route handlers.
         */
        void use(const std::string &prefix, const RequestHandler &handler);

        /*-------------------------------------------------------------------------------------------------
         *
         * Middleware factories
         *
         **/

        /**
         * @brief Creates a CORS middleware that sets the appropriate headers.
         * @param origin Allowed origin value for the `Access-Control-Allow-Origin` header (default `"*"`).
         * @return RequestHandler that sets CORS headers and responds to OPTIONS preflight requests.
         */
        static RequestHandler cors(const std::string &origin = "*");

        /**
         * @brief Creates a token authentication middleware.
         * @return RequestHandler that validates session and API tokens from cookies/headers,
         *         rejecting the request with 401 if no valid token is present.
         */
        static RequestHandler auth();

        /*-------------------------------------------------------------------------------------------------
         *
         * Token helpers
         *
         **/

        /**
         * @brief Issues a time-limited session token.
         * @param seconds  Lifetime of the token in seconds.
         * @param actions  List of actions the token is permitted to perform (default: TOKEN_ACTIONS).
         * @return Signed token string to be stored in a session cookie.
         */
        static std::string getSessionToken(long seconds, const std::vector<std::string> &actions = TOKEN_ACTIONS);

        /**
         * @brief Creates or retrieves a named permanent API token.
         * @param name    Human-readable token name (used as its identifier).
         * @param actions List of actions the token is permitted to perform (default: TOKEN_ACTIONS).
         * @return Token value string.
         */
        static std::string getApiToken(const std::string &name, const std::vector<std::string> &actions = TOKEN_ACTIONS);

        /**
         * @brief Permanently removes a named API token.
         * @param name Name of the API token to remove.
         */
        static void removeApiToken(const std::string &name);
    };

}
