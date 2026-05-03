// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <LittleFS.h>

#include <ArduinoJson.h>
#include <ESP32-targz.h>

#include <string>
#include <vector>

namespace ESP32WebServer
{

    const std::string TEMP_FOLDER = "/tmp";

    struct FileInfo
    {
        std::string name;
        std::string path;
        std::string extension;
        std::string baseName;
    };

    bool fileExists(const std::string &filePath);
    std::string randomString(int size = 32);
    std::string getTempFolder();

    /*-------------------------------------------------------------------------------------------------
     *
     * Unzipping
     *
     **/

    std::string unzip(const std::string &filePath);

    /*-------------------------------------------------------------------------------------------------
     *
     * Folder and Folder Contents
     *
     **/

    std::vector<FileInfo> listFiles(const std::string &folderPath, std::vector<FileInfo> &files, const std::string &prefix = "");
    std::vector<FileInfo> listFiles(const std::string &folderPath);
    void clearFolder(const std::string &folderPath);

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Files
     *
     **/

    int removeFile(const std::string &filePath);

    /*-------------------------------------------------------------------------------------------------
     *
     * Read and write JSON
     *
     **/

    JsonDocument readJsonFile(const std::string &filePath);
    bool writeJsonFile(const std::string &filePath, const JsonDocument &doc);

}
