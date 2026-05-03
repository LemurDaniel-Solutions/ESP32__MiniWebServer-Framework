// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <stdlib.h>
#include <Update.h>

#include <vector>
#include <string>

#include <router/router.h>
#include <utils/utility.admin.h>
#include <utils/utility.file.h>

namespace ESP32WebServer
{
    void post_UpdateWebsite(Request &req, Response &res);
    void post_UpdateCode(Request &req, Response &res);

    class UpdateRouter : public ESP32WebServer::Router
    {
    public:
        UpdateRouter()
        {
            route("POST", "/upload/code", post_UpdateCode);
            route("POST", "/upload/website", post_UpdateWebsite);
        }
    };
}
