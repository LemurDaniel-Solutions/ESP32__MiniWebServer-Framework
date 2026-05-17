// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <utils/utility.wifi.h>

namespace EspWeb
{
    void WiFiUtility::wifiManagerTask(void *param)
    {
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(30000));
            WiFiUtility::instance().setup();
        }
    }

    int WiFiUtility::isApMode()
    {
        return WiFi.getMode() == WIFI_AP;
    }

    int WiFiUtility::isWiFiConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }

    bool WiFiUtility::isNetworkReady()
    {
        return WiFi.isConnected() || WiFi.getMode() == WIFI_AP;
    }

    void WiFiUtility::clearWiFiConfig()
    {
        fileHandler.removeFile(WIFI_CONFIG_FILE);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Saved / Nearby Networks
     *
     **/

    std::vector<WiFiConfig> WiFiUtility::scanNetworks()
    {
        std::vector<WiFiConfig> ssids;

        int n = WiFi.scanNetworks();
        for (int i = 0; i < n; ++i)
        {
            WiFiConfig config;
            config.ssid = WiFi.SSID(i).c_str();
            config.signalStrength = WiFi.RSSI(i);
            ssids.push_back(config);
        }

        return ssids;
    }

    std::vector<WiFiConfig> WiFiUtility::getSavedNetworks()
    {
        std::vector<WiFiConfig> savedNetworks;

        if (!fileHandler.exists(WIFI_CONFIG_FILE))
            return savedNetworks;

        JsonDocument doc = fileHandler.readJson(WIFI_CONFIG_FILE);
        for (JsonPair entry : doc["networks"].as<JsonObject>())
        {
            WiFiConfig config;
            config.ssid = entry.key().c_str();
            config.password = entry.value()["password"] | "";
            savedNetworks.push_back(config);
        }

        return savedNetworks;
    }

    std::vector<WiFiConfig> WiFiUtility::getNearestNetworks()
    {
        const std::vector<WiFiConfig> scannedNetworks = scanNetworks();
        const std::vector<WiFiConfig> savedNetworks = getSavedNetworks();
        std::vector<WiFiConfig> nearbyNetworks;

        for (const WiFiConfig &scanned : scannedNetworks)
        {
            for (const WiFiConfig &saved : savedNetworks)
            {
                if (scanned.ssid != saved.ssid)
                    continue;

                WiFiConfig match = saved;
                match.signalStrength = scanned.signalStrength;
                nearbyNetworks.push_back(match);
                break;
            }
        }

        std::sort(nearbyNetworks.begin(), nearbyNetworks.end(), [](const WiFiConfig &a, const WiFiConfig &b)
                  { return a.signalStrength > b.signalStrength; });

        return nearbyNetworks;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Add Remove Connect
     *
     **/
    void WiFiUtility::addWiFiConfig(const std::string &ssid, const std::string &password)
    {
        JsonDocument doc;
        if (fileHandler.exists(WIFI_CONFIG_FILE))
            doc = fileHandler.readJson(WIFI_CONFIG_FILE);

        doc["networks"][ssid]["password"] = password;

        fileHandler.writeJson(WIFI_CONFIG_FILE, doc);
    }

    void WiFiUtility::removeWiFiConfig(const std::string &ssid)
    {
        if (!fileHandler.exists(WIFI_CONFIG_FILE))
            return;

        JsonDocument doc = fileHandler.readJson(WIFI_CONFIG_FILE);
        doc["networks"].as<JsonObject>().remove(ssid.c_str());
        fileHandler.writeJson(WIFI_CONFIG_FILE, doc);
    }

    WiFiConfig WiFiUtility::getActiveWiFi()
    {

        WiFiConfig activeWiFi;
        if (!isNetworkReady())
        {
            return activeWiFi;
        }

        activeWiFi.ssid = WiFi.SSID().c_str();
        activeWiFi.signalStrength = WiFi.RSSI();
        activeWiFi.ipAddress = WiFi.localIP().toString().c_str();

        std::vector<WiFiConfig> savedNetworks = getSavedNetworks();
        for (const WiFiConfig &config : savedNetworks)
        {
            if (config.ssid != activeWiFi.ssid)
                continue;
            activeWiFi.password = config.password;
        }

        return activeWiFi;
    }

    bool WiFiUtility::attemptConnect(WiFiConfig network)
    {
        Serial.printf("Connecting to: %s (%d dBm)\n", network.ssid.c_str(), network.signalStrength);

        WiFi.mode(WIFI_STA);
        WiFi.begin(network.ssid.c_str(), network.password.c_str());

        const int timeStart = millis() / 1000;
        while (!WiFi.isConnected())
        {
            vTaskDelay(pdMS_TO_TICKS(500));
            Serial.print(".");

            if ((int)(millis() / 1000) - timeStart >= _WIFI_TIMEOUT_SEC)
            {
                Serial.println("\nConnection timed out.");
                WiFi.disconnect();
                return false;
            }
        }

        Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }

    /*-------------------------------------------------------------------------------------------------

        Setup Wifi


        Continually checks every 30 seconds if the device is connected to a WiFi.
        If it is NOT, then attempts its connection routine:

        -> Find the nearest Saved network and connect to it
        -> On Failure try the next network

        -> Fallback to AP-Mode if
            -> No Connection could be made
            -> No saved network is nearby


    */
    void WiFiUtility::setup()
    {
        if (isWiFiConnected())
        {
            return;
        }

        bool wasApMode = isApMode();

        std::vector<WiFiConfig> nearby = getNearestNetworks();
        for (const WiFiConfig &network : nearby)
        {
            if (attemptConnect(network))
            {
                return;
            }
        }

        if (wasApMode)
        {
            Serial.println("Already in AP Mode...");
        }

        else if (nearby.empty())
        {
            Serial.println("No saved networks in range, starting AP mode...");
        }
        else
        {
            Serial.println("No Connection could be made, starting in AP mode...");
        }

        WiFi.mode(WIFI_AP);
        WiFi.softAP(DEFAULT_WIFI_SSID);
        Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    }

}