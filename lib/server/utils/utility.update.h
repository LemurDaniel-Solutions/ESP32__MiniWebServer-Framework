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

namespace EspWeb
{
    void post_UploadFile(Request &req, Response &res);
    void post_UpdateCode(Request &req, Response &res);
    void get_ClearWebsite(Request &req, Response &res);

    class UpdateRouter : public EspWeb::Router
    {
    public:
        UpdateRouter()
        {
            use("/upload", auth_handler);

            route("POST", "/upload/code", post_UpdateCode);
            route("POST", "/upload/file", post_UploadFile);
            route("GET", "/upload/clear/website", get_ClearWebsite);
        }
    };
}
