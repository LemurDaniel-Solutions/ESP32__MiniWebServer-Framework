// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Update.h>

#include <router/router.h>
#include <utils/utility.wifi.h>

namespace EspWeb
{
    extern std::map<std::string, std::string> CUSTOM_LINKS;

    void get_AdminLinks(Request &req, Response &res);
    void get_AdminLogin(Request &req, Response &res);
    void get_AdminDashboard(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Request Handlers Authentication
     *
     **/

    void auth_handler(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Admin Credentials
     *
     **/

    void get_AdminLogout(Request &req, Response &res);
    void post_AdminLogin(Request &req, Response &res);
    void post_AdminUpdateAuth(Request &req, Response &res);
    void post_AdminRestart(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Permanent API Tokens
     *
     **/

    void get_PermTokens(Request &req, Response &res);
    void get_TokenActions(Request &req, Response &res);
    void post_PermToken(Request &req, Response &res);
    void delete_PermToken(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle WiFi Configuration
     *
     **/

    void get_WiFiActive(Request &req, Response &res);
    void get_WiFiScan(Request &req, Response &res);
    void get_WiFiSavedNetworks(Request &req, Response &res);
    void delete_WiFiSavedNetwork(Request &req, Response &res);
    void post_WiFiSavedNetwork(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Updates
     *
     **/

    void post_UploadFile(Request &req, Response &res);
    void post_UpdateCode(Request &req, Response &res);
    void get_ClearWebsite(Request &req, Response &res);

    class AdminRouter : public EspWeb::Router
    {
    public:
        AdminRouter(int enableDashboard)
        {
            use("/admin", auth_handler);
            use("/upload", auth_handler);

            route("POST", "/upload/code", post_UpdateCode);
            route("POST", "/upload/file", post_UploadFile);
            route("GET", "/upload/clear/website", get_ClearWebsite);

            // Returns the html sites
            if (enableDashboard)
            {
                route("GET", "/admin", get_AdminLogin);
                route("GET", "/admin/dashboard", get_AdminDashboard);
            }

            // Unprotected Routes
            route("GET", "/admin/logout", get_AdminLogout);
            route("POST", "/admin/login", post_AdminLogin);

            route("GET", "/admin/links", get_AdminLinks);
            route("POST", "/admin/auth", post_AdminUpdateAuth);

            // Wifi config routes for admin dashboard
            route("POST", "/admin/restart", post_AdminRestart);

            // Get al WiFi Networks in reach of the device
            route("GET", "/admin/wifi/scan", get_WiFiScan);
            route("GET", "/admin/wifi/active", get_WiFiActive);

            // Get all WiFi networks to possibly connect to
            route("GET", "/admin/wifi/networks", get_WiFiSavedNetworks);

            // Add or Remove a WiFi-Config
            route("POST", "/admin/wifi/network", post_WiFiSavedNetwork);
            route("DELETE", "/admin/wifi/network", delete_WiFiSavedNetwork);

            // Permanent API Tokens
            route("GET", "/admin/tokens", get_PermTokens);
            route("GET", "/admin/token/actions", get_TokenActions);
            route("POST", "/admin/token", post_PermToken);
            route("DELETE", "/admin/token", delete_PermToken);
        }
    };
}
