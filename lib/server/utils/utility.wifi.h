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

/** @brief LittleFS path for the saved WiFi network configuration file. */
#define WIFI_CONFIG_FILE "/WiFiConfig.json "

/** @brief Default AP-mode SSID used when no saved network is available. */
#define DEFAULT_WIFI_SSID "ESP32_MiniWebServer"

namespace EspWeb
{
    /**
     * @brief Describes a WiFi network configuration entry.
     */
    struct WiFiConfig
    {
        /** @brief Network SSID. */
        std::string ssid;

        /** @brief Network password. */
        std::string password;

        /** @brief Signal strength in dBm (populated during scans; 0 if not scanned). */
        int signalStrength = 0;

        /** @brief Current IP address string; `"Not Connected"` when not associated. */
        std::string ipAddress = "Not Connected";
    };

    /**
     * @brief Singleton utility for WiFi connection management and AP/STA mode control.
     *
     * Continuously monitors the connection state in a background FreeRTOS task and
     * reconnects to the nearest saved network on failure. Falls back to AP mode when
     * no saved network is reachable. Access via WiFiUtility::instance().
     */
    class WiFiUtility
    {
    private:
        WiFiUtility() = default;

        /** @brief Seconds to wait for a WiFi association attempt before giving up. */
        const int _WIFI_TIMEOUT_SEC = 30;

    public:
        /**
         * @brief Returns the singleton WiFiUtility instance.
         * @return Reference to the global WiFiUtility.
         */
        static WiFiUtility &instance()
        {
            static WiFiUtility inst;
            return inst;
        }

        /**
         * @brief Checks whether the device is currently in AP (access point) mode.
         * @return Non-zero if in AP mode, 0 if in STA mode.
         */
        int isApMode();

        /**
         * @brief Checks whether the device has an active WiFi association.
         * @return Non-zero if connected to a WiFi network, 0 otherwise.
         */
        int isWiFiConnected();

        /**
         * @brief Checks whether the network is fully ready (connected and has an IP).
         * @return `true` if the device has a valid IP address.
         */
        bool isNetworkReady();

        /**
         * @brief Removes all saved WiFi credentials, causing AP-mode fallback on next setup().
         */
        void clearWiFiConfig();

        /*-------------------------------------------------------------------------------------------------
         *
         * Handle Saved / Nearby Networks
         *
         **/

        /**
         * @brief Performs a blocking WiFi scan and returns all visible networks.
         * @return Vector of WiFiConfig entries for all detected networks (sorted by signal strength).
         */
        std::vector<WiFiConfig> scanNetworks();

        /**
         * @brief Loads all saved network configurations from LittleFS.
         * @return Vector of saved WiFiConfig entries (no signal strength data).
         */
        std::vector<WiFiConfig> getSavedNetworks();

        /**
         * @brief Returns saved networks that are currently detectable, sorted by signal strength.
         * @return Vector of WiFiConfig entries present in both saved list and current scan.
         */
        std::vector<WiFiConfig> getNearestNetworks();

        /*-------------------------------------------------------------------------------------------------
         *
         * Add / Remove / Connect
         *
         **/

        /**
         * @brief Saves a new WiFi network configuration to LittleFS.
         * @param ssid     Network SSID.
         * @param password Network password.
         */
        void addWiFiConfig(const std::string &ssid, const std::string &password);

        /**
         * @brief Removes a saved WiFi network configuration from LittleFS.
         * @param ssid SSID of the network to remove.
         */
        void removeWiFiConfig(const std::string &ssid);

        /**
         * @brief Returns the currently active WiFi connection details.
         * @return WiFiConfig with SSID, IP address, and signal strength of the active connection,
         *         or a default entry if not connected.
         */
        WiFiConfig getActiveWiFi();

        /**
         * @brief Attempts to connect to a specific WiFi network.
         * @param network WiFiConfig containing the SSID and password to connect to.
         * @return `true` if the connection succeeded within the timeout, `false` otherwise.
         */
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

        /**
         * @brief Initializes WiFi and starts the background connection manager task.
         *
         * Connection routine:
         * 1. Scan for nearby networks.
         * 2. Connect to the strongest saved network in range.
         * 3. Fall back to AP mode (`DEFAULT_WIFI_SSID`) if no saved network is reachable.
         * 4. Repeats the check every `_WIFI_TIMEOUT_SEC` seconds.
         */
        void setup();

        /**
         * @brief FreeRTOS task that periodically checks and restores WiFi connectivity.
         * @param param Pointer to the owning WiFiUtility instance.
         */
        static void wifiManagerTask(void *param);
    };
}
