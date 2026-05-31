// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Update.h>

#include <router/router.h>
#include <utils/utility.wifi.h>

namespace EspWeb
{
    /** @brief Map of custom navigation links injected into the admin dashboard. Key = display name, value = href. */
    extern std::map<std::string, std::string> CUSTOM_LINKS;

    /** @brief Returns the custom links JSON used by the admin dashboard frontend. */
    void get_AdminLinks(Request &req, Response &res);

    /** @brief Serves the admin login HTML page. */
    void get_AdminLogin(Request &req, Response &res);

    /** @brief Serves the admin dashboard HTML page (requires authentication). */
    void get_AdminDashboard(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Authentication middleware
     *
     **/

    /**
     * @brief Middleware that enforces admin authentication.
     *
     * Validates the admin session cookie. Redirects to `/admin` on failure.
     * Registered automatically on all `/admin` and `/upload` prefixes.
     */
    void auth_handler(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Admin credential management
     *
     **/

    /** @brief Clears the admin session cookie and redirects to the login page. */
    void get_AdminLogout(Request &req, Response &res);

    /** @brief Validates username/password from the request body and issues a session cookie. */
    void post_AdminLogin(Request &req, Response &res);

    /** @brief Updates the admin username and/or password. */
    void post_AdminUpdateAuth(Request &req, Response &res);

    /** @brief Triggers an ESP32 restart. */
    void post_AdminRestart(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * Permanent API token management
     *
     **/

    /** @brief Returns the list of all stored permanent API tokens. */
    void get_PermTokens(Request &req, Response &res);

    /** @brief Returns the configured list of available token actions. */
    void get_TokenActions(Request &req, Response &res);

    /** @brief Creates a new permanent API token from the request body. */
    void post_PermToken(Request &req, Response &res);

    /** @brief Deletes a permanent API token by name. */
    void delete_PermToken(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * WiFi configuration management
     *
     **/

    /** @brief Returns details of the currently active WiFi connection. */
    void get_WiFiActive(Request &req, Response &res);

    /** @brief Scans for nearby WiFi networks and returns the results. */
    void get_WiFiScan(Request &req, Response &res);

    /** @brief Returns all saved WiFi network configurations. */
    void get_WiFiSavedNetworks(Request &req, Response &res);

    /** @brief Removes a saved WiFi network by SSID. */
    void delete_WiFiSavedNetwork(Request &req, Response &res);

    /** @brief Saves a new WiFi network configuration (SSID + password). */
    void post_WiFiSavedNetwork(Request &req, Response &res);

    /*-------------------------------------------------------------------------------------------------
     *
     * OTA update and file upload
     *
     **/

    /** @brief Receives a file upload and stores it in LittleFS. */
    void post_UploadFile(Request &req, Response &res);

    /** @brief Receives a firmware binary and performs an OTA update. */
    void post_UpdateCode(Request &req, Response &res);

    /** @brief Clears all files in the web assets folder. */
    void get_ClearWebsite(Request &req, Response &res);

    /**
     * @brief Pre-configured Router that registers all built-in admin and upload routes.
     *
     * Automatically wires the auth_handler middleware on `/admin` and `/upload` prefixes.
     * If `enableDashboard` is non-zero, the HTML pages for login and dashboard are also served.
     */
    class AdminRouter : public EspWeb::Router
    {
    public:
        /**
         * @brief Constructs the AdminRouter and registers all admin routes.
         * @param enableDashboard Non-zero to also register the login and dashboard HTML page routes.
         */
        AdminRouter(int enableDashboard)
        {
            use("/admin", auth_handler);
            use("/upload", auth_handler);

            route("POST", "/upload/code", post_UpdateCode);
            route("POST", "/upload/file", post_UploadFile);
            route("GET", "/upload/clear/website", get_ClearWebsite);

            if (enableDashboard)
            {
                route("GET", "/admin", get_AdminLogin);
                route("GET", "/admin/dashboard", get_AdminDashboard);
            }

            // Unprotected routes
            route("GET", "/admin/logout", get_AdminLogout);
            route("POST", "/admin/login", post_AdminLogin);

            route("GET", "/admin/links", get_AdminLinks);
            route("POST", "/admin/auth", post_AdminUpdateAuth);

            route("POST", "/admin/restart", post_AdminRestart);

            route("GET", "/admin/wifi/scan", get_WiFiScan);
            route("GET", "/admin/wifi/active", get_WiFiActive);

            route("GET", "/admin/wifi/networks", get_WiFiSavedNetworks);

            route("POST", "/admin/wifi/network", post_WiFiSavedNetwork);
            route("DELETE", "/admin/wifi/network", delete_WiFiSavedNetwork);

            route("GET", "/admin/tokens", get_PermTokens);
            route("GET", "/admin/token/actions", get_TokenActions);
            route("POST", "/admin/token", post_PermToken);
            route("DELETE", "/admin/token", delete_PermToken);
        }
    };
}
