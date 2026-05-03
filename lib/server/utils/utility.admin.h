// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <stdlib.h>
#include "mbedtls/sha256.h"

#include <vector>
#include <string>
#include <map>

#include <router/router.h>
#include <utils/utility.wifi.h>

#define ADMIN_CREDENTIALS_FILE "/admin_credentials.json"

namespace ESP32WebServer
{
    class TokenManager
    {
    public:
        std::string DEFAULT_ADMIN_COOKIE = "adminToken";

        std::string DEFAULT_ADMIN_USER = "admin";
        std::string DEFAULT_ADMIN_PWD = "admin";

        std::string DEFAULT_ADMIN_SALT = "5B63F3F0104D1649B8E1A9C9E5F2A1";

        static TokenManager &instance()
        {
            static TokenManager _instance;
            return _instance;
        }

        /*-------------------------------------------------------------------------------------------------
         *
         * Handle credentials
         *
         **/

        bool setCredentials(std::string username, std::string password);
        std::vector<std::string> getCredentials();

        bool checkCredentials(const std::string &username, const std::string &password);
        std::string generateSHA256(const std::string &text, const std::string salt);

        /*-------------------------------------------------------------------------------------------------
         *
         * Handle tokens
         *
         **/

        void addToken(const std::string &token);
        void removeToken(const std::string &token);
        std::string getToken(const std::string &username);
        bool checkToken(const std::string &authToken);

    private:
        TokenManager() {}

        TokenManager(const TokenManager &) = delete;
        void operator=(const TokenManager &) = delete;

        std::map<std::string, unsigned long> ADMIN_TOKENS;
    };

    void get_AdminLogin(Request &req, Response &res);
    void get_AdminDashboard(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Request Handlers Authentication
     *
     **/

    bool isTokenValid(Request &req, Response &res);
    void verify_AdminAuth(Request &req, Response &res);
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
     * Handle WiFi Configuration
     *
     **/

    void get_WiFiActive(Request &req, Response &res);
    void get_WiFiScan(Request &req, Response &res);
    void get_WiFiSavedNetworks(Request &req, Response &res);
    void delete_WiFiSavedNetwork(Request &req, Response &res);
    void post_WiFiSavedNetwork(Request &req, Response &res);

    class AdminRouter : public ESP32WebServer::Router
    {
    public:
        AdminRouter()
        {
            use("/admin", auth_handler);

            // Unprotected Routes
            route("GET", "/admin", get_AdminLogin);
            route("GET", "/admin/logout", get_AdminLogout);
            route("GET", "/admin/login", verify_AdminAuth);
            route("POST", "/admin/login", post_AdminLogin);

            // Returns the html sites
            route("GET", "/admin/dashboard", get_AdminDashboard);

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
        }
    };
}
