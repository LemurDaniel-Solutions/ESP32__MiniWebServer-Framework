// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <utils/utility.path.h>

namespace EspWeb
{

    /*-------------------------------------------------------------------------------------------------
     *
     * Path Resolver
     *
     **/

    void PathResolver::add(const std::string &path, const std::vector<RequestHandler> &handlers)
    {
        this->root.add(path, handlers);
    }
    std::vector<RequestHandler> PathResolver::resolve(const std::string &path, Request &req, bool accumulate)
    {
        return this->root.resolve(path, req, accumulate);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Path Node - Messy Code incoming - This is supposed to be a TRIE-Structure
     *
     **/
    std::vector<RequestHandler> PathNode::resolve(const std::string &path, Request &req, bool accumulate)
    {
        std::vector<std::string> segments = split(path, "/");
        std::vector<std::string> reversed;
        while (segments.size() > 0)
        {
            reversed.push_back(segments[segments.size() - 1]);
            segments.pop_back();
        }
        return this->resolve(reversed, req, accumulate);
    }

    std::vector<RequestHandler> PathNode::resolve(std::vector<std::string> &path, Request &req, bool accumulate)
    {
        if (path.size() <= 0)
        {
            return this->handlers;
        }

        // Accumulated handlers
        std::vector<RequestHandler> handlers;
        if (accumulate)
        {
            for (const RequestHandler &handler : this->handlers)
            {
                handlers.push_back(handler);
            }
        }

        const std::string segment = path[path.size() - 1];
        path.pop_back();

        auto node = nodes.find(segment);
        if (node != nodes.end())
        {
            const std::vector<RequestHandler> &result = nodes[segment].resolve(path, req, accumulate);
            for (const RequestHandler &handler : result)
            {
                handlers.push_back(handler);
            }

            return handlers;
        }

        // Search for route parameter
        node = nodes.find(":");
        if (node != nodes.end())
        {
            req.route.insert({node->second.segment, segment});
            const std::vector<RequestHandler> &result = nodes[":"].resolve(path, req, accumulate);
            for (const RequestHandler &handler : result)
            {
                handlers.push_back(handler);
            }
            return handlers;
        }

        return handlers;
    }

    void PathNode::add(const std::string &path, const std::vector<RequestHandler> &handlers)
    {
        std::vector<std::string> segments = split(path, "/");
        std::vector<std::string> reversed;
        while (segments.size() > 0)
        {
            reversed.push_back(segments[segments.size() - 1]);
            segments.pop_back();
        }
        this->add(reversed, handlers);
    }

    void PathNode::add(std::vector<std::string> &path, const std::vector<RequestHandler> &handlers)
    {
        if (path.size() <= 0)
        {
            for (const RequestHandler &handler : handlers)
            {
                this->handlers.push_back(handler);
            }
            return;
        }

        std::string segment = path[path.size() - 1];
        std::string normalized = path[path.size() - 1];
        path.pop_back();

        // If its a route parameter
        if (segment[0] == ':')
        {
            normalized = segment.substr(1);
            segment = ":";
        }

        // If no node already exists, create one.
        auto node = nodes.find(segment);
        if (node == nodes.end())
        {
            nodes[segment].segment = normalized;
            nodes[segment].add(path, handlers);
        }
        else
        {
            node->second.add(path, handlers);
        }
    }

}