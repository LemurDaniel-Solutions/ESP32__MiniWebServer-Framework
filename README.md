# 🚀 (Prototype) ESP32 Mini WebServer Framework

<div align="center">

![ESP32](https://img.shields.io/badge/ESP32-000000?style=for-the-badge&logo=Espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-FF6000?style=for-the-badge&logo=PlatformIO&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)

### ⚠️ This is still a Prototype for my Personal Projects

### 🤓 So Please be nice with your Feedback

</div>


## 🎯 Overview

A lightweight **Mini WebServer Framework** for ESP32 microcontrollers as a personal project! 🎉

Add it to your PlatformIO project via `lib_deps` and include `server.h`.

May not work with the RISC V Processor
- ESP32-C3
- ESP32-C6
- ESP32-H2

It Should work with following families:
- ESP32 (WROOM, WROVER, CAM ...)
- ESP32-S2
- ESP32-S3

## 🙏 Acknowledgments

- 🎉 **Arduino Community** for the amazing ecosystem
- 🔧 **PlatformIO** for the excellent development platform
- 🌐 **ESP32** community for inspiration and support
- 💖 **Open Source** contributors worldwide


## 📁 Project Structure

```
📦 ESP32 Mini WebServer Framework
├── 📁 lib/
│   └── 📁 server/
│       ├── 🌐 server.h/.cpp        # Core web server
│       ├── 🛤️ router.h             # Routing engine
│       ├── 📥 request.h            # HTTP request handling
│       ├── 📤 response.h           # HTTP response handling
│       ├── 🔐 utility.admin.h      # Admin Dashboard
│       ├── 🛜 utility.wifi.h       # WiFi utility
│       └── 📂 utility.file.h       # File utility
├── 📁 src/                         # Example application
│   ├── 🎯 main.cpp
│   └── 📁 routes/
│       └── 🛤️ routes.example.h/.cpp
├── 📁 data/
│   └── 📁 web/
│       └── 🎨 index.html           # Example web interface
├── 📋 library.json                 # PlatformIO library config
├── ⚙️ platformio.ini               # Build configuration
└── 📖 README.md                    # This file
```

## 📦 Installation

Add the following to your `platformio.ini`:

```ini
[env:your_board]
platform = espressif32
framework = arduino
board = <your-board>
board_build.filesystem = littlefs

lib_deps =
    https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework.git

; List any static files you want embedded in the LittleFS image
board_build.include_files_txt =
    data/web/index.html
    data/web/main.css
```

Then include `server.h` in your code:

```cpp
#include <server.h>
```

PlatformIO downloads the library and its dependencies (ArduinoJson) automatically on the next build.

---

## 🎮 Usage

### 🚀 Quick Start

#### Prerequisites

- ✅ [PlatformIO IDE](https://platformio.org/platformio-ide)
- ✅ ESP32 development board
- ✅ USB cable for programming
- ✅ WiFi network (optional — can be configured via AP mode after flashing)

#### Minimal example

```cpp
#include <Arduino.h>
#include <server.h>

void setup()
{
    Serial.begin(115200);

    ESP32WebServer::MiniServer *Server = new ESP32WebServer::MiniServer();

    // Optional: pre-save WiFi credentials in code.
    // Without this the device starts in AP mode — see WiFi Setup below.
    // Server->connectWiFi("YOUR_SSID", "YOUR_PASSWORD");

    Server->get("/hello", [](ESP32WebServer::Request &req, ESP32WebServer::Response &res) {
        res.text("Hello from ESP32!").OK();
    });

    Server->start(80);
}

void loop() { delay(10); }
```

After uploading, open the serial monitor — the device prints its IP address once connected. Navigate to that address in your browser.

---

### 🛜 WiFi Setup

The device supports two WiFi modes that switch automatically:

**Station mode** — connects to a saved WiFi network on boot and prints the assigned IP to the serial monitor.

**AP mode** — fallback when no saved network is in range. The device creates a hotspot called `ESP32_MiniWebServer` at `192.168.4.1`. Open the Admin Dashboard at `/admin` to scan for networks, enter credentials, and save them.

#### Configuring WiFi in code

Credentials are saved permanently to LittleFS. Multiple networks can be stored — on each boot the device picks whichever saved network has the strongest signal:

```cpp
Server->connectWiFi("HOME_SSID", "HOME_PASSWORD");
Server->connectWiFi("OFFICE_SSID", "OFFICE_PASSWORD");

// Remove all saved networks and force AP mode
// Server->clearWiFi();
```

#### Connection routine

On `Server->start()` and every **30 seconds** while offline:

1. **Scan** for nearby networks and match against all saved networks.
2. **Sort** matches by signal strength.
3. **Connect** to each candidate in order (30-second timeout per attempt).
4. **Fallback to AP mode** if no saved network is reachable.

#### Default admin credentials

| Field | Default |
|-------|---------|
| Username | `admin` |
| Password | `admin` |

Override in code — these are only applied on first boot if no credentials file exists yet:

```cpp
Server->defaultAdminCredentials("admin", "my-secure-password");
Server->defaultAdminSalt("optional-salt");

// Disable the admin dashboard entirely
// Server->disableAdmin();
```

---

### 🔨 Build & Upload

**1. Upload the filesystem** — required if you serve static files from LittleFS (e.g. a frontend). Skip if you are not using `Server->root()` or `Server->staticFile()`.

![Build Filesystem](.assets/pio.build-filesystem.png)

**2. Upload firmware and open the serial monitor**

![Upload and Monitor](.assets/pio.upload-monitor.png)

---

<details>
<summary>🛤️ Router System</summary>

### Best Practices

**Organize by functionality:**
```
src/routes/
├── routes.sensors.h/.cpp   # 🌡️ Temperature, humidity, pressure
├── routes.control.h/.cpp   # 💡 LED, relay, motor control
├── routes.system.h/.cpp    # 📊 System info, diagnostics
└── routes.auth.h/.cpp      # 🔐 Authentication & user management
```

**Naming convention:**
```cpp
// Namespace: routes_{functionality}
namespace routes_sensors { ... }

// Functions: {method}_{endpoint}
void get_temperature(Request &req, Response &res) { ... }
void post_led_control(Request &req, Response &res) { ... }
```

### 1. Create the header file

Declare handler functions and a `Router` class that registers them:

```cpp
#include <Arduino.h>
#include <WiFi.h>

#include <router.h>

namespace routes_example
{
    class Router : public ESP32WebServer::Router
    {
    public:
        Router()
        {
            route("GET", "/hello", get_hello);
            route("GET", "/status", get_status);
            route("GET", "/example", get_example);
            route("POST", "/data", post_data);
        }

    private:
        static void get_hello(ESP32WebServer::Request &req, ESP32WebServer::Response &res);
        static void get_status(ESP32WebServer::Request &req, ESP32WebServer::Response &res);
        static void get_example(ESP32WebServer::Request &req, ESP32WebServer::Response &res);
        static void post_data(ESP32WebServer::Request &req, ESP32WebServer::Response &res);
    };

}
```

### 2. Implement route handlers

In the `.cpp`, include the header and implement each function:

```cpp


#include <routes/routes.example.h>

namespace routes_example
{
    void Router::get_hello(ESP32WebServer::Request &req, ESP32WebServer::Response &res)
    {
        res.text("Hello, World! This is a simple response from the ESP32.").status(200);
    }

    void Router::get_status(ESP32WebServer::Request &req, ESP32WebServer::Response &res)
    {
        JsonDocument status;

        status["device"] = "ESP32";
        status["firmware"] = "1.0.0";
        status["uptime"] = static_cast<double>(millis());
        status["free_heap"] = static_cast<double>(ESP.getFreeHeap());
        status["wifi_rssi"] = static_cast<double>(WiFi.RSSI());

        res.json(status).status(200);
    }

    void Router::get_example(ESP32WebServer::Request &req, ESP32WebServer::Response &res)
    {
        res.text("This is an example route!").status(200);
    }

    void Router::post_data(ESP32WebServer::Request &req, ESP32WebServer::Response &res)
    {
        JsonDocument response;
        response["message"] = "Data received successfully";
        response["timestamp"] = millis();
        res.json(response).status(201);
    }

}
```

### 3. Register with the server

```cpp
#include <Arduino.h>
#include <server.h>
#include <routes/routes.example.h>

void setup()
{
    ESP32WebServer::MiniServer *Server = new ESP32WebServer::MiniServer();

    // Will start enter WiFi setup, if this function isn't used.
    // Credentials are permanently stored via LittleFs.
    // Server->connectWiFi("<SSID / Wlan Name >", "***<PASSWORD>***");

    // For testing purposes, remove WiFi config to trigger AP mode
    // Server->clearWiFi();

    // Hardcode default credentials (Can be set via Dashboard without hardcoding!)
    // Server->defaultAdminCredentials("admin", "admin");
    // Server->defaultAdminSalt("");

    // Disables admin routes entirely
    // Server->disableAdmin();

    Server->root("/web");
    Server->index("/web/index.html");

    Server->registerRouter(routes_example::Router());

    Server->start(80);
}

void loop() { delay(10); }
```

</details>

---

<details>
<summary>📤 Response Handling</summary>

A Response object is provided and passed to all handler functions

The response is sent automatically 
- after the last handler finishes
- or a response in the chain is finalized

### Response types

| Method | Content-Type | Example |
|--------|-------------|---------|
| `text(str)` | `text/plain` | `res.text("Hello").status(200)` |
| `json(doc)` | `application/json` | `res.json(doc).status(200)` |
| `html(str)` | `text/html` | `res.html("<h1>Hi</h1>").status(200)` |
| `file(path)` | `text/html` | `res.file("/web/index.html")` |
| `binaryFile(path)` | `application/octet-stream` | `res.binaryFile("/data.bin")` |

### Status shorthands

These set the status code and fill in a default body if none was set yet:

| Method | Status | Default body |
|--------|--------|-------------|
| `OK()` | 200 | `"OK"` |
| `NotFound()` | 404 | `"Not Found"` |
| `InternalServerError()` | 500 | `"Internal Server Error"` |

```cpp
res.json(doc).OK();              // 200, JSON body
res.text("Missing").NotFound();  // 404, custom text body
res.text("Created").status(201); // any status code
```

### Stopping the middleware chain

Call `finalize()` to prevent any further handlers from running — typically used in middleware:

```cpp
res.text("Unauthorized").status(401).finalize();
```

➡️ See **🔗 Middleware** below for full details.

</details>

---

<details>
<summary>📥 Request Body</summary>

The request body is accessed via `req.body` and is read lazily — nothing is read from the socket until you call one of the methods below.

| Method | Description |
|--------|-------------|
| `req.body.text()` | Read body as a `std::string` |
| `req.body.json()` | Parse body as JSON, returns `JsonDocument` |
| `req.body.file()` | Write body to a temp file on LittleFS, returns the file path |
| `req.body.chunks(buf, size)` | Read body in chunks into a buffer, returns bytes read |

`req.body.contentLength` and `req.body.contentType` are always available without triggering a read.

### JSON body

```cpp
Server->post("/data", [](ESP32WebServer::Request &req, ESP32WebServer::Response &res) {
    JsonDocument body = req.body.json();
    const char* name = body["name"];
    res.text(name).OK();
});
```

### Binary / streaming (e.g. OTA firmware)

```cpp
Server->post("/ota", [](ESP32WebServer::Request &req, ESP32WebServer::Response &res) {
    if (Update.begin(req.body.contentLength)) {
        uint8_t buf[1024];
        size_t n;
        while ((n = req.body.chunks(buf, sizeof(buf))) > 0)
            Update.write(buf, n);

        if (Update.end() && Update.isFinished())
            res.text("Update OK").OK();
        else
            res.text("Update failed").InternalServerError();
    }
});
```

</details>

---

<details>
<summary>🔗 Middleware Chain</summary>

Middleware allows you to chain multiple handler functions for a single route. Each handler runs in order. Calling `res.finalize()`, stops processing further handlers in the chain

### **How it works**

Instead of a single `RequestHandler`, pass a `std::vector<RequestHandler>` to `route()`:

```cpp
route("GET", "/secret", {
    authMiddleware,   // runs first — aborts with 401 if not authorized via finalize()
    get_secret        // only runs if authMiddleware did NOT finalize the response
});
```

The server iterates through all handlers and breaks as soon as `response.finalized == true` ([server.cpp](lib/server/server.cpp#L96-L107)).

### **Implementing Middleware**

A middleware handler has the same signature as a regular route handler. Call `res.finalize()` to short-circuit the chain:

```cpp
void authMiddleware(ESP32WebServer::Request &req, ESP32WebServer::Response &res)
{
    // Check for a valid session cookie or token
    if (req.cookies.find("session") == req.cookies.end())
    {
        res.text("Unauthorized").status(401).finalize();
        // Chain stops here — get_secret is never called
        return;
    }
    // No finalize() call → next handler in the chain runs
}

void get_secret(ESP32WebServer::Request &req, ESP32WebServer::Response &res)
{
    res.text("Secret data!").status(200);
}
```

### **Registering Middleware**

There are two ways to attach middleware:

**Per-route** — pass a handler list directly to `route()`. Only runs for that specific route:

**Global / prefix-based** — register on the server via `use()`. Runs before every matching route, without repeating it in each `route()` call. Useful for logging, CORS headers, or auth that spans multiple endpoints:

```cpp
class Router : public ESP32WebServer::Router
{
public:
    Router()
    {
        // Middleware on all paths of /
        use("/", defaultHandler);

        // Public routes — single handler
        route("GET", "/hello", get_hello);

        // Protected routes — middleware chain
        route("GET", "/secret",  { authMiddleware, get_secret  });
        route("POST", "/config", { authMiddleware, post_config });
    }
};
```

### **Execution Flow**

```
Request → use() handlers (global, then prefix-matched, in registration order)
              │
              ├─ finalize() called → Response sent immediately
              │
              └─ not finalized ──→ per-route middleware chain
                                        │
                                        ├─ finalize() called → Response sent
                                        │
                                        └─ not finalized ──→ route handler → Response sent
```

### **Common Middleware Patterns**

| Pattern | Where | Description |
|---------|-------|-------------|
| Logging | `use()` | Log every request without repeating per route |
| CORS headers | `use()` | Add headers to every response |
| Auth (global) | `use("/api")` | Protect all `/api/*` routes at once |
| Auth (per-route) | `route()` chain | Fine-grained control per endpoint |
| Rate Limiting | `use()` | Track request counts, abort with `429` |

➡️ See also: **📤 Response Handling** section above for all available response methods.

</details>

---

<details>
<summary>🛜 Admin Dashboard</summary>

The built-in Admin Dashboard is available at `/admin` on every device. In AP mode (no WiFi configured) connect to the `ESP32_MiniWebServer` hotspot and open `192.168.4.1/admin`.

![Admin Dashboard](.assets/admin.dashboard.overview.png)

It provides:

- **WiFi** — scan for nearby networks, select one, enter the password and save. Credentials persist across reboots. Multiple networks can be stored; the device always picks the strongest signal.
- **Security** — change the admin username and password.
- **Restart** — reboot the device directly from the browser.

![Admin Dashboard WiFi](.assets/admin.dashboard.wifi.png)

#### Configuration

Default credentials are `admin` / `admin`. Override in code:

```cpp
Server->defaultAdminCredentials("admin", "my-secure-password");
Server->defaultAdminSalt("optional-salt");

// Disable the dashboard entirely
// Server->disableAdmin();
```

</details>
