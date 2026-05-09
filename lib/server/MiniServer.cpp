// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <MiniServer.h>

/********** TCP Server Implementation **********/

namespace EspWeb
{

    MiniServer::MiniServer()
    {
        if (!LittleFS.begin(true))
        {
            Serial.println("Error setting up File System!");
        }
    }
    MiniServer::~MiniServer()
    {
        closeServer();
    }

    void MiniServer::closeServer()
    {
        close(_server_socket);
    }

    void MiniServer::connectWiFi(const std::string &ssid, const std::string &password)
    {
        WiFiUtility::instance().addWiFiConfig(ssid, password);
    }

    void MiniServer::clearWiFi()
    {
        WiFiUtility::instance().clearWiFiConfig();
    }

    void MiniServer::defaultAdminSalt(const std::string &salt)
    {
        TokenManager::instance().setSalt(salt);
    }

    void MiniServer::defaultAdminCredentials(const std::string &username, const std::string &password)
    {
        TokenManager::instance().setCredentials(username, password);
    }

    void MiniServer::setTokenActions(const std::vector<std::string> actions)
    {
        TOKEN_ACTIONS = actions;
        TOKEN_ACTIONS.insert(TOKEN_ACTIONS.begin(), "admin");
    }

    void MiniServer::disableAdmin()
    {
        _is_admin_enabled = false;
        disableAdminDashboard();
    }
    void MiniServer::disableAdminDashboard()
    {
        _is_dashboard_enabled = false;
    }

    void MiniServer::dns(const std::string &dnsName)
    {
        _dnsName = dnsName;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle client requests
     *
     *
     **/

    void MiniServer::handleClient(int client_socket)
    {
        Response response = Response();

        // Parse the raw HTTP request into a structured Request object
        Request request = Request::parse(client_socket);
        if (request.rejected)
        {
            response.status(413).text(request.error);
            Response::send(client_socket, response);
            struct linger sl = {1, 0};
            setsockopt(client_socket, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));
            return;
        }

        // Search for matching middlewares
        for (const auto &entry : _middlewares)
        {
            if (request.path.find(entry.first) != 0)
            {
                continue;
            }

            for (const RequestHandler &handler : entry.second)
            {
                handler(request, response);
                if (response.isFinalized())
                {
                    return Response::send(client_socket, response);
                }
            }
        }

        // Search for matching route
        std::string routeKey;
        routeKey.reserve(request.method.size() + 1 + request.path.size());
        routeKey = request.method;
        routeKey += ' ';
        routeKey += request.path;

        auto entry = _routes.find(routeKey);
        if (entry == _routes.end())
        {
            response.NotFound();
            Response::send(client_socket, response);
            Serial.printf("No handler found for route: %s\n", routeKey.c_str());
            return;
        }

        const std::vector<RequestHandler> &route = entry->second;
        for (const RequestHandler &handler : route)
        {
            handler(request, response);
            if (response.isFinalized())
                break;
        }

        Response::send(client_socket, response);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Middleware
     *
     **/

    void MiniServer::use(const std::string &prefix, const RequestHandler &handler)
    {
        auto entry = _middlewares.find(prefix);
        if (entry != _middlewares.end())
        {
            std::vector<RequestHandler> &list = entry->second;
            list.push_back(handler);
        }
        else
        {
            std::vector<RequestHandler> list{handler};
            _middlewares.insert({prefix, list});
        }
    }
    void MiniServer::use(const RequestHandler &handler)
    {
        use("/", handler);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Implement Routes
     *
     **/

    void MiniServer::addRoute(const std::string &method, const std::string &path, const std::vector<RequestHandler> &handlers)
    {
        _routes.insert({method + " " + path, handlers});
    }
    void MiniServer::addRoute(const std::string &method, const std::string &path, const RequestHandler &handler)
    {
        const std::vector<RequestHandler> wrapper = {handler};
        addRoute(method, path, wrapper);
    }

    void MiniServer::registerRouter(const EspWeb::Router &router)
    {
        for (const auto &route : router.routes)
        {
            Serial.printf("Registering route: %s %s\n", route.method.c_str(), route.path.c_str());
            addRoute(route.method, route.path, route.handler);
        }

        for (const auto &entry : router.middlewares)
        {
            Serial.printf("Registering Middelware for Prefix: %s\n", entry.first.c_str());
            for (const RequestHandler &handler : entry.second)
            {
                use(entry.first, handler);
            }
        }
    }

    void MiniServer::route(const std::string &method, const std::string &path, const RequestHandler &handler)
    {
        addRoute(method, path, handler);
    }

    void MiniServer::get(const std::string &path, const RequestHandler &handler)
    {
        addRoute("GET", path, handler);
    }

    void MiniServer::post(const std::string &path, const RequestHandler &handler)
    {
        addRoute("POST", path, handler);
    }

    void MiniServer::put(const std::string &path, const RequestHandler &handler)
    {
        addRoute("PUT", path, handler);
    }

    void MiniServer::patch(const std::string &path, const RequestHandler &handler)
    {
        addRoute("PATCH", path, handler);
    }

    void MiniServer::del(const std::string &path, const RequestHandler &handler)
    {
        addRoute("DELETE", path, handler);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Serve and handle files:
     *
     **/

    void MiniServer::staticFile(const std::string &path, const std::string &file_path)
    {
        Serial.printf("Adding file response: %s -> %s\n", path.c_str(), file_path.c_str());

        const RequestHandler &handler = [file_path](const EspWeb::Request &req, EspWeb::Response &res)
        {
            res.file(file_path);
        };

        addRoute("GET", path, handler);
    }

    void MiniServer::root(const std::string &folder_path, const std::string &prefix)
    {
        _is_root_set = true;
        FOLDER_WEB = folder_path;
        std::vector<FileInfo> files = listFiles(folder_path);
        for (FileInfo info : files)
        {
            if (info.isDirectory)
            {
                root(info.path, prefix + info.name + "/");
            }
            else
            {
                staticFile(prefix + info.name, info.path);
            }
        }
    }

    void MiniServer::index(const std::string &index_path)
    {
        _is_index_set = true;
        staticFile("/", index_path);
        staticFile("/index", index_path);
        staticFile("/index.html", index_path);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle multiple connections and cleanup:
     *
     **/
    void MiniServer::workerTask(void *param)
    {

        MiniServer *server = static_cast<MiniServer *>(param);
        int client_socket;

        while (true)
        {
            // Hier wartet der Task, verbraucht 0% CPU währenddessen
            if (xQueueReceive(server->_handleQueue, &client_socket, portMAX_DELAY))
            {
                // --- TIMEOUT SETUP START ---
                struct timeval tv;
                tv.tv_sec = 5;
                tv.tv_usec = 0;

                setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
                // --- TIMEOUT SETUP END ---
                Serial.println("--------------------------------------------------------------------");
                Serial.printf("Worker handling client on socket %d\n", client_socket);

                server->handleClient(client_socket);

                Serial.printf("Worker finished handling client on socket %d\n", client_socket);
                Serial.println("--------------------------------------------------------------------");
                shutdown(client_socket, SHUT_RDWR);
                close(client_socket);
            }
        }
    }

    void MiniServer::acceptClientTask(void *param)
    {
        MiniServer *server = static_cast<MiniServer *>(param);
        const int server_socket = server->_server_socket;

        while (true)
        {
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);

            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &len);

            if (client_socket < 0)
            {
                Serial.println("Failed to accept client connection");
                return;
            }

            Serial.printf("Accepted new client on socket %d\n", client_socket);

            if (xQueueSend(server->_handleQueue, &client_socket, 0) != pdTRUE)
            {
                Serial.println("Queue full! Rejecting new client.");
                write(client_socket, "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 19\r\n\r\nService Unavailable", 75);
                close(client_socket);
            }
        }
    }

    int MiniServer::start(int port, const std::string &ip_addr)
    {

        if (_is_running)
        {
            Serial.println("Server is already running");
            return 0;
        }

        if (_is_admin_enabled)
        {
            this->registerRouter(EspWeb::AdminRouter(_is_dashboard_enabled));
        }

        if (!_is_root_set && !_is_index_set)
        {
            this->root(FOLDER_WEB);
            this->index(FOLDER_WEB + "/" + "index.html");
        }

        if (!WiFiUtility::instance().isNetworkReady())
        {
            WiFiUtility::instance().setup();
        }

        memset(&_address, 0, sizeof(_address));
        _address.sin_family = AF_INET;
        _address.sin_port = htons(port);
        _address_len = sizeof(_address);
        if (inet_pton(AF_INET, ip_addr.c_str(), &_address.sin_addr) <= 0)
        {
            Serial.println("Invalid IP address");
            return 1;
        }

        _server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_server_socket < 0)
        {
            Serial.println("Failed to create socket");
            return 1;
        }

        int opt = 1;
        if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            Serial.println("Failed to set socket options");
            return 1;
        }

        if (bind(_server_socket, (struct sockaddr *)&_address, sizeof(_address)) < 0)
        {
            Serial.println("Failed to bind socket");
            return 1;
        }

        if (listen(_server_socket, 3) < 0)
        {
            Serial.println("Failed to listen on socket");
            return 1;
        }

        if (!_dnsName.empty())
        {
            if (!MDNS.begin(_dnsName.c_str()))
            {
                Serial.println("mDNS failed to start!");
            }
            else
            {
                MDNS.addService("http", "tcp", port);
                Serial.printf("mDNS started: http://%s.local\n", _dnsName.c_str());
            }

            WiFi.onEvent([this, port](WiFiEvent_t event, WiFiEventInfo_t info)
                         {
                MDNS.end();
                if (MDNS.begin(_dnsName.c_str()))
                    MDNS.addService("http", "tcp", port); },
                         ARDUINO_EVENT_WIFI_STA_GOT_IP);
        }

        Serial.println("Starting worker tasks...");
        for (int i = 0; i < EspWeb::WORKER_TASK_COUNT; i++)
        {
            std::string taskName = "worker" + std::to_string(i);
            xTaskCreatePinnedToCore(workerTask, taskName.c_str(), 8192, this, 1, NULL, i % 2);
        }

        Serial.println("Starting accept client task...");
        xTaskCreatePinnedToCore(acceptClientTask, "accept", 8192, this, 2, NULL, 0);

        Serial.println("Starting WiFi manager task");
        xTaskCreate(WiFiUtility::wifiManagerTask, "WiFiManager", 4096, nullptr, 1, nullptr);

        Serial.println("Server started and listening for clients...");
        _is_running = true;
        return 0;
    }

}