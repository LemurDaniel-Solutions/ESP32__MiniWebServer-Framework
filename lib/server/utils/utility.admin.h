// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <router/router.h>
#include <utils/utility.wifi.h>

namespace EspWeb
{
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

    class AdminRouter : public EspWeb::Router
    {
    public:
        AdminRouter(int enableDashboard)
        {
            use("/admin", auth_handler);

            // Returns the html sites
            if (enableDashboard)
            {
                route("GET", "/admin", get_AdminLogin);
                route("GET", "/admin/dashboard", get_AdminDashboard);
            }

            // Unprotected Routes
            route("GET", "/admin/logout", get_AdminLogout);
            route("POST", "/admin/login", post_AdminLogin);

            // Return 401 if the token is not valid or missing for any /admin/* route
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
            route("POST", "/admin/token", post_PermToken);
            route("DELETE", "/admin/token", delete_PermToken);
        }
    };
}
