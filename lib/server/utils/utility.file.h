// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <LittleFS.h>

#include <ArduinoJson.h>

#include <string>
#include <vector>

namespace EspWeb
{
    extern std::string FOLDER_TEMP;
    extern std::string FOLDER_WEB;

    struct FileInfo
    {
        int isDirectory;
        std::string name;
        std::string path;
        std::string extension;
        std::string baseName;
    };

    std::string getTempFolder();
    FileInfo getFileInfo(const std::string &filePath);
    bool fileExists(const std::string &filePath);

    /*-------------------------------------------------------------------------------------------------
     *
     * Helper Methods
     *
     **/

    std::string randomString(int size = 32);
    std::string trim(const std::string &s);

    size_t findBytes(const std::vector<uint8_t> &data, const std::string &pattern, size_t start = 0);
    std::string extractString(const std::vector<uint8_t> &data, size_t start, size_t end);
    std::vector<std::string> split(const std::string &text, const std::string &splitter);

    /*-------------------------------------------------------------------------------------------------
     *
     * Folder and Folder Contents
     *
     **/

    std::vector<FileInfo> listFiles(const std::string &folderPath);
    void clearFolder(const std::string &folderPath);
    void removeFolder(const std::string &folderPath);

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Files
     *
     **/

    void moveFile(const std::string &filePath, const std::string &destinationFolder, const std::string &relativePath = "");
    int removeFile(const std::string &filePath);

    /*-------------------------------------------------------------------------------------------------
     *
     * Read and write JSON
     *
     **/

    JsonDocument readJsonFile(const std::string &filePath);
    bool writeJsonFile(const std::string &filePath, const JsonDocument &doc);

}
