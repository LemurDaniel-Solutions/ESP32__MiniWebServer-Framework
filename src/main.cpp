#include <Arduino.h>

#include <MiniServer.h>

#include <routes/routes.example.h>

void setup()
{
  Serial.begin(115200);

  EspWeb::MiniServer *Server = new EspWeb::MiniServer();

  // Will start enter WiFi setup, if this function isn't used.
  // Credentials are permanently stored via LittleFs.
  // Server->connectWiFi("<SSID / Wlan Name >", "***<PASSWORD>***");

  // For testing purposes, remove WiFi config to trigger AP mode
  // Server->clearWiFi();

  // Hardcode default credentials (Can be set via Dashoard without hardcoding!)
  // Server->defaultAdminCredentials("admin", "admin");
  // Server->defaultAdminSalt("");

  // Disables admin routes entirly
  // Server->disableAdmin();

  // Take care of CORS
  // Server->use(Server->cors());

  // Server->root("/web");
  // Server->index("/web/index.html");

  // Use MDNS - avialable as myESP32.local. May not work depending on Browser.
  Server->dns("myESP32"); 

  Server->registerRouter(routes_example::Router());

  Server->start(80);
}

void loop()
{
  while (true)
  {
    delay(10);
  }
}
