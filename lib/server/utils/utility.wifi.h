// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

#include <algorithm>
#include <vector>
#include <string>

#include <utils/utility.file.h>

#define WIFI_CONFIG_FILE "/WiFiConfig.json"
#define DEFAULT_WIFI_SSID "ESP32_MiniWebServer"

namespace ESP32WebServer
{

    struct WiFiConfig
    {
        std::string ssid;
        std::string password;
        int signalStrength = 0;
        std::string ipAddress = "Not Connected";
    };

    class WiFiUtility
    {
    private:
        WiFiUtility() = default;

        const int _WIFI_TIMEOUT_SEC = 30;

    public:
        static WiFiUtility &instance()
        {
            static WiFiUtility inst;
            return inst;
        }

        int isApMode();
        int isWiFiConnected();
        bool isNetworkReady();
        void clearWiFiConfig();

        /*-------------------------------------------------------------------------------------------------
         *
         * Handle Saved / Nearby Networks
         *
         **/

        std::vector<WiFiConfig> scanNetworks();

        std::vector<WiFiConfig> getSavedNetworks();

        std::vector<WiFiConfig> getNearestNetworks();

        /*-------------------------------------------------------------------------------------------------
         *
         * Add Remove Connect
         *
         **/

        void addWiFiConfig(const std::string &ssid, const std::string &password);

        void removeWiFiConfig(const std::string &ssid);

        WiFiConfig getActiveWiFi();

        bool attemptConnect(WiFiConfig network);

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

        void setup();

        static void wifiManagerTask(void *param);
    };
}