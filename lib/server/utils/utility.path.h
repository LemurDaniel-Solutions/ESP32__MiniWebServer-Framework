// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>

#include <vector>
#include <string>
#include <map>

#include <router/router.h>
#include <utils/utility.file.h>

namespace EspWeb
{
    /**
     * @brief A single node in the path-matching trie.
     *
     * Each node represents one URL path segment. Literal segments are stored as
     * direct children; `:param` segments are stored under the key `":"`.
     * Handlers are stored at the node that terminates a registered path.
     */
    class PathNode
    {
    public:
        /** @brief Child nodes keyed by path segment (or `":"` for param segments). */
        std::map<std::string, PathNode> nodes;

        /** @brief Handler chain stored at this node if a route ends here. */
        std::vector<RequestHandler> handlers;

        /** @brief The path segment this node represents. */
        std::string segment;

        /**
         * @brief Registers a handler chain under the given path pattern.
         * @param path     Full URL path string (e.g. `"/api/users/:id"`).
         * @param handlers Ordered list of handler functions.
         */
        void add(const std::string &path, const std::vector<RequestHandler> &handlers);

        /**
         * @brief Recursive helper that inserts handlers into the trie.
         * @param path     Remaining path segments (modified in-place as recursion descends).
         * @param handlers Ordered list of handler functions.
         */
        void add(std::vector<std::string> &path, const std::vector<RequestHandler> &handlers);

        /**
         * @brief Resolves a request path against this trie node.
         * @param path       Full URL path string to match.
         * @param req        Request object; named `:param` values are written to req.route.
         * @param accumulate If `true`, collects handlers from all matching prefix nodes
         *                   (used for middleware); if `false`, returns only the exact match.
         * @return Matched handler chain, or empty vector if no match found.
         */
        std::vector<RequestHandler> resolve(const std::string &path, Request &req, bool accumulate);

        /**
         * @brief Recursive helper that resolves remaining path segments.
         * @param path       Remaining path segments (modified in-place as recursion descends).
         * @param req        Request object; named `:param` values are written to req.route.
         * @param accumulate If `true`, accumulates handlers at every matching prefix node.
         * @return Matched handler chain, or empty vector if no match found.
         */
        std::vector<RequestHandler> resolve(std::vector<std::string> &path, Request &req, bool accumulate);
    };

    /**
     * @brief Trie-based URL path resolver for routes and middleware.
     *
     * Wraps a PathNode root and exposes a clean add/resolve interface.
     * Used by MiniServer to dispatch incoming requests to the correct handler chain.
     */
    class PathResolver
    {
    private:
        /** @brief Root node of the trie (represents `"/"`). */
        PathNode root;

    public:
        /**
         * @brief Registers a handler chain under a path pattern.
         * @param path     URL path pattern (e.g. `"GET /api/users/:id"`).
         * @param handlers Ordered list of handler functions.
         */
        void add(const std::string &path, const std::vector<RequestHandler> &handlers);

        /**
         * @brief Resolves a request path to its handler chain.
         * @param path       URL path string to match.
         * @param req        Request object; route params are written to req.route on match.
         * @param accumulate If `true`, collects handlers from all prefix matches (for middleware).
         * @return Matched handler chain, or empty vector if no match found.
         */
        std::vector<RequestHandler> resolve(const std::string &path, Request &req, bool accumulate = false);
    };
}
