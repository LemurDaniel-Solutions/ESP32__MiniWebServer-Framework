// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <LittleFS.h>

#include <ArduinoJson.h>
#include <vector>
#include <string>

namespace EspWeb
{
    struct FileInfo
    {
        int isDirectory;
        std::string name;
        std::string path;
        std::string extension;
        std::string baseName;
    };

    class FileHandler
    {
    private:
        std::string _tempFolder = "/tmp";

    public:
        /*-------------------------------------------------------------------------------------------------
         *
         * Helper Methods
         *
         **/

        std::string getTempFolder();
        std::string getTempFile();
        std::string randomString(int size = 32);
        FileInfo getInfo(const std::string &path);
        bool exists(const std::string &path);

        std::string trim(const std::string &s);
        std::vector<std::string> split(const std::string &text, const std::string &splitter);

        /*-------------------------------------------------------------------------------------------------
         *
         * Folder and Folder Contents
         *
         **/

        std::vector<FileInfo> listFiles(const std::string &path);
        void clearFolder(const std::string &path);
        void removeFolder(const std::string &path);

        /*-------------------------------------------------------------------------------------------------
         *
         * Handle Files
         *
         **/

        bool removeFile(const std::string &path);
        bool moveFile(const std::string &path, const std::string &destination);

        /*-------------------------------------------------------------------------------------------------
         *
         * Read and write JSON
         *
         **/

        JsonDocument readJson(const std::string &path);
        void writeJson(const std::string &path, const JsonDocument &doc);
    };

    extern FileHandler fileHandler;
    extern std::string FOLDER_TEMP;
    extern std::string FOLDER_WEB;
}
