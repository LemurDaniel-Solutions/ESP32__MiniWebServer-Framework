// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include <vector>
#include <string>
#include <map>

#include <ArduinoJson.h>

#include <utils/utility.token.h>
#include <utils/utility.file.h>
#include <utils/utility.wifi.h>
#include <utils/utility.admin.h>
#include <utils/utility.path.h>

#include <router/router.h>

namespace EspWeb
{

    const int WORKER_TASK_COUNT = 4;
    const int CONNECTION_LIMIT = 10;
    const int CONNECTION_TIMEOUT_SEC = 30;

    class MiniServer
    {
    public:
        MiniServer();
        ~MiniServer();

        int start(const int port, const std::string &ip_addr = "0.0.0.0");
        void dns(const std::string &dnsName);

        // Connect to WiFi network via SSID (Name of WiFi) and password
        // If not used, the server will start in AP mode with SSID "ESP32_MiniWebServer" and a default admin page for WiFi configuration
        void connectWiFi(const std::string &ssid, const std::string &password);
        void clearWiFi();

        // Serve a static file as index.html on the root path
        void index(const std::string &index_path);

        // Serve all files in there as root;
        void root(const std::string &folder_path, const std::string &prefix = "/");

        // Add a static file response for a specific path
        void staticFile(const std::string &path, const std::string &file_path);

        // Register routes with method, path and handler function
        void registerRouter(const EspWeb::Router &router);
        void route(const std::string &method, const std::string &path, const RequestHandler &handler);

        // Quick functions
        void get(const std::string &path, const RequestHandler &handler);
        void post(const std::string &path, const RequestHandler &handler);
        void put(const std::string &path, const RequestHandler &handler);
        void patch(const std::string &path, const RequestHandler &handler);
        // delete is a keyword in c++, hence using del
        void del(const std::string &path, const RequestHandler &handler);

        // Add default middleware handler
        void use(const RequestHandler &handler);
        void use(const std::string &prefix, const RequestHandler &handler);

        RequestHandler cors(const std::string &origin = "*") { return Router::cors(origin); }
        RequestHandler auth() { return Router::auth(); }

        // Disables the admin dashboard entirly
        void disableAdmin();
        void disableAdminDashboard();

        // Overrides the default admin credentials
        void defaultAdminSalt(const std::string &salt);
        void defaultAdminCredentials(const std::string &username, const std::string &password);

        void setTokenActions(const std::vector<std::string> actions);

    private:
        std::string _dnsName;
        struct sockaddr_in _address;
        unsigned int _address_len;
        int _server_socket;
        void closeServer();

        int _is_running = false;
        int _is_root_set = false;
        int _is_index_set = false;
        int _is_admin_enabled = true;
        int _is_dashboard_enabled = true;

        void processHandlers(Request &req, Response &res);
        void handleClient(int client_socket);

        // Map of "METHOD PATH" to handler function for dynamic routes
        PathResolver _routes;
        PathResolver _middlewares;
        void addRoute(const std::string &method, const std::string &path, const RequestHandler &handler);
        void addRoute(const std::string &method, const std::string &path, const std::vector<RequestHandler> &handlers);

        // Queue for incoming connections to be processed by worker threads
        QueueHandle_t _handleQueue = xQueueCreate(CONNECTION_LIMIT, sizeof(int));
        static void workerTask(void *param);
        static void acceptClientTask(void *param);
    };
}