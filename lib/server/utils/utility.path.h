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
    class PathNode
    {
    private:
    public:
        std::map<std::string, PathNode> nodes;
        std::vector<RequestHandler> handlers;
        std::string segment;

        void add(const std::string &path, const std::vector<RequestHandler> &handlers);
        void add(std::vector<std::string> &path, const std::vector<RequestHandler> &handlers);

        std::vector<RequestHandler> resolve(const std::string &path, Request &req, bool accumulate);
        std::vector<RequestHandler> resolve(std::vector<std::string> &path, Request &req, bool accumulate);
    };

    class PathResolver
    {
    private:
        PathNode root;

    public:
        void add(const std::string &path, const std::vector<RequestHandler> &handlers);
        std::vector<RequestHandler> resolve(const std::string &path, Request &req, bool accumulate = false);
    };
}