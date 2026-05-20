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
        close(_serverSocket);
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

    void MiniServer::setTokenActions(const std::vector<std::string> &actions)
    {
        TOKEN_ACTIONS = actions;
        TOKEN_ACTIONS.insert(TOKEN_ACTIONS.begin(), "admin");
    }

    void MiniServer::setCustomLink(const std::string &name, const std::string &href)
    {
        CUSTOM_LINKS.insert({name, href});
    }

    void MiniServer::disableAdmin()
    {
        _isAdminEnabled = false;
        disableAdminDashboard();
    }
    void MiniServer::disableAdminDashboard()
    {
        _isDashboardEnabled = false;
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

    void MiniServer::handleClient(int clientSocket)
    {
        Response response = Response(clientSocket);

        // Parse the raw HTTP request into a structured Request object
        Request request = Request::parse(clientSocket);
        if (request.rejected)
        {
            response.status(413).text(request.error).send();
            struct linger sl = {1, 0};
            setsockopt(clientSocket, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));
            return;
        }

        // Search for matching middlewares
        const std::vector<RequestHandler> middlewares = _middlewares.resolve(request.path, request, true);
        Serial.printf("Middlewares resolved for %s: %d\n", request.path.c_str(), middlewares.size());
        for (const RequestHandler &handler : middlewares)
        {
            handler(request, response);
            if (response.isFinalized())
            {
                Serial.printf("Middleware finalized response for %s\n", request.path.c_str());
                response.send();
                return;
            }
        }

        // Search for matching route
        std::string routeKey;
        routeKey.reserve(request.method.size() + request.path.size());
        routeKey = request.method;
        routeKey += request.path;

        const std::vector<RequestHandler> routes = _routes.resolve(routeKey, request, false);
        if (routes.size() == 0)
        {
            response.NotFound().send();
            Serial.printf("No handler found for route: %s\n", routeKey.c_str());
            return;
        }

        for (const RequestHandler &handler : routes)
        {
            handler(request, response);
            if (response.isFinalized())
            {
                Serial.printf("route finalized response for %s\n", request.path.c_str());
                response.send();
                return;
            }
        }

        response.send();
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Middleware
     *
     **/

    void MiniServer::use(const std::string &prefix, const RequestHandler &handler)
    {
        if (prefix[0] != '/')
        {
            _middlewares.add('/' + prefix, {handler});
        }
        else
        {
            _middlewares.add(prefix, {handler});
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
        if (path[0] != '/')
        {
            _routes.add(method + "/" + path, handlers);
        }
        else
        {
            _routes.add(method + path, handlers);
        }
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

    void MiniServer::staticFile(const std::string &path, const std::string &filePath)
    {
        Serial.printf("Adding file response: %s -> %s\n", path.c_str(), filePath.c_str());

        std::string normalizedPath = filePath;
        if (normalizedPath[0] != '/')
        {
            normalizedPath = '/' + normalizedPath;
        }

        const RequestHandler &handler = [normalizedPath](const EspWeb::Request &req, EspWeb::Response &res)
        {
            res.file(normalizedPath);
        };

        addRoute("GET", path, handler);
    }

    void MiniServer::root(const std::string &folderPath, const std::string &prefix)
    {
        _isRootSet = true;
        FOLDER_WEB = folderPath;
        std::vector<FileInfo> files = fileHandler.listFiles(folderPath);
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

    void MiniServer::index(const std::string &indexPath)
    {
        _isIndexSet = true;
        staticFile("/", indexPath);
        staticFile("/index", indexPath);
        staticFile("/index.html", indexPath);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle multiple connections and cleanup:
     *
     **/
    void MiniServer::workerTask(void *param)
    {

        MiniServer *server = static_cast<MiniServer *>(param);
        int clientSocket;

        while (true)
        {
            // Hier wartet der Task, verbraucht 0% CPU währenddessen
            if (xQueueReceive(server->_handleQueue, &clientSocket, portMAX_DELAY))
            {
                // --- TIMEOUT SETUP START ---
                struct timeval tv;
                tv.tv_sec = CONNECTION_TIMEOUT_SEC;
                tv.tv_usec = 0;

                setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

                // --- TIMEOUT SETUP END ---
                Serial.println("--------------------------------------------------------------------");
                Serial.printf("Worker handling client on socket %d\n", clientSocket);

                server->handleClient(clientSocket);

                Serial.printf("Worker finished handling client on socket %d\n", clientSocket);
                Serial.println("--------------------------------------------------------------------");
                shutdown(clientSocket, SHUT_RDWR);
                close(clientSocket);
            }
        }
    }

    void MiniServer::acceptClientTask(void *param)
    {
        MiniServer *server = static_cast<MiniServer *>(param);
        const int serverSocket = server->_serverSocket;

        while (true)
        {
            struct sockaddr_in clientAddr;
            socklen_t len = sizeof(clientAddr);

            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &len);

            if (clientSocket < 0)
            {
                Serial.println("Failed to accept client connection");
                return;
            }

            Serial.printf("Accepted new client on socket %d\n", clientSocket);

            if (xQueueSend(server->_handleQueue, &clientSocket, 0) != pdTRUE)
            {
                Serial.println("Queue full! Rejecting new client.");
                write(clientSocket, "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 19\r\n\r\nService Unavailable", 75);
                close(clientSocket);
            }
        }
    }

    int MiniServer::start(int port, const std::string &ipAddr, int workerCount)
    {

        if (_isRunning)
        {
            Serial.println("Server is already running");
            return 0;
        }

        if (_isAdminEnabled)
        {
            this->registerRouter(EspWeb::AdminRouter(_isDashboardEnabled));
        }

        if (!_isRootSet && !_isIndexSet)
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
        _addressLen = sizeof(_address);
        if (inet_pton(AF_INET, ipAddr.c_str(), &_address.sin_addr) <= 0)
        {
            Serial.println("Invalid IP address");
            return 1;
        }

        _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (_serverSocket < 0)
        {
            Serial.println("Failed to create socket");
            return 1;
        }

        int opt = 1;
        if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            Serial.println("Failed to set socket options");
            return 1;
        }

        if (bind(_serverSocket, (struct sockaddr *)&_address, sizeof(_address)) < 0)
        {
            Serial.println("Failed to bind socket");
            return 1;
        }

        if (listen(_serverSocket, 3) < 0)
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
        for (int i = 0; i < workerCount; i++)
        {
            std::string taskName = "worker" + std::to_string(i);
            xTaskCreatePinnedToCore(workerTask, taskName.c_str(), 8192, this, 1, NULL, i % 2);
        }

        Serial.println("Starting accept client task...");
        xTaskCreatePinnedToCore(acceptClientTask, "accept", 8192, this, 2, NULL, 0);

        Serial.println("Starting WiFi manager task");
        xTaskCreate(WiFiUtility::wifiManagerTask, "WiFiManager", 4096, nullptr, 1, nullptr);

        Serial.println("Server started and listening for clients...");
        _isRunning = true;
        return 0;
    }

}
