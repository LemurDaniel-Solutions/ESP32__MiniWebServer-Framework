// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <LittleFS.h>

#include <ArduinoJson.h>
#include <vector>
#include <string>

#include <Preferences.h>

namespace EspWeb
{
    /**
     * @brief Metadata describing a single file or directory in LittleFS.
     */
    struct FileInfo
    {
        /** @brief True if this entry is a directory. */
        int isDirectory;

        /** @brief File or directory name without path (e.g. `"index.html"`). */
        std::string name;

        /** @brief Full absolute LittleFS path (e.g. `"/web/index.html"`). */
        std::string path;

        /** @brief File extension including the dot (e.g. `".html"`), empty for directories. */
        std::string extension;

        /** @brief File name without extension (e.g. `"index"`). */
        std::string baseName;
    };

    /**
     * @brief Utility class for LittleFS file system operations.
     *
     * Provides helpers for file/folder management, JSON persistence, and
     * string utilities used across the framework. A global instance `fileHandler`
     * is available in the EspWeb namespace.
     */
    class FileHandler
    {
    private:
        /** @brief Path of the temporary folder used for spooled uploads. */
        std::string _tempFolder = "/tmp";

        /** @brief FNV-1a 32-bit hash of key → 8-char hex string safe for NVS (max 15 chars). */
        std::string hashKey(const std::string &key);

    public:
        /*-------------------------------------------------------------------------------------------------
         *
         * Helper Methods
         *
         **/

        /**
         * @brief Returns the path of the temporary folder.
         * @return Absolute LittleFS path to the temp folder (default `"/tmp"`).
         */
        std::string getTempFolder();

        /**
         * @brief Creates a unique temp file path inside the temp folder.
         * @return Absolute LittleFS path to a new, unused temp file.
         */
        std::string getTempFile();

        /**
         * @brief Generates a random alphanumeric string.
         * @param size Length of the string to generate (default 32).
         * @return Random string of the given length.
         */
        std::string randomString(int size = 32);

        /**
         * @brief Retrieves metadata for a file or directory.
         * @param path Absolute LittleFS path.
         * @return FileInfo struct populated with the entry's metadata.
         */
        FileInfo getInfo(const std::string &path);

        /**
         * @brief Checks whether a file or directory exists.
         * @param path Absolute LittleFS path.
         * @return `true` if the entry exists, `false` otherwise.
         */
        bool exists(const std::string &path);

        /**
         * @brief Strips leading and trailing whitespace from a string.
         * @param s Input string.
         * @return Trimmed copy of the string.
         */
        std::string trim(const std::string &s);

        /**
         * @brief Splits a string by a delimiter into a vector of substrings.
         * @param text     Input string to split.
         * @param splitter Delimiter string.
         * @return Vector of split parts (delimiter not included).
         */
        std::vector<std::string> split(const std::string &text, const std::string &splitter);

        /*-------------------------------------------------------------------------------------------------
         *
         * Folder and Folder Contents
         *
         **/

        /**
         * @brief Lists all files and directories directly inside a folder.
         * @param path Absolute LittleFS folder path.
         * @return Vector of FileInfo for each entry in the folder.
         */
        std::vector<FileInfo> listFiles(const std::string &path);

        /**
         * @brief Deletes all files inside a folder without removing the folder itself.
         * @param path Absolute LittleFS folder path.
         */
        void clearFolder(const std::string &path);

        /**
         * @brief Recursively deletes a folder and all its contents.
         * @param path Absolute LittleFS folder path.
         */
        void removeFolder(const std::string &path);

        /*-------------------------------------------------------------------------------------------------
         *
         * Handle Files
         *
         **/

        /**
         * @brief Deletes a single file.
         * @param path Absolute LittleFS file path.
         * @return `true` if the file was deleted, `false` if it did not exist.
         */
        bool removeFile(const std::string &path);

        /**
         * @brief Moves (renames) a file to a new location.
         * @param path        Absolute LittleFS source path.
         * @param destination Absolute LittleFS destination path.
         * @return `true` on success, `false` on failure.
         */
        bool moveFile(const std::string &path, const std::string &destination);

        /*-------------------------------------------------------------------------------------------------
         *
         * Read and write JSON
         *
         **/

        /**
         * @brief Reads a file from Little FS.
         * @param key File Path.
         * @return std::string read from file.
         */
        std::string readFile(const std::string &key);

        /**
         * @brief Reads and parses a JSON value from Non-Volatile Storage.
         * @param key Non Volatile Storage Key.
         * @return Parsed JsonDocument; empty document on read or parse error.
         */
        JsonDocument readJsonConfig(const std::string &key);

        /**
         * @brief Serializes a JsonDocument and writes it to Non-Volatile Storage.
         * @param key Non Volatile Storage Key.
         * @param doc JsonDocument to serialize.
         */
        void writeJsonConfig(const std::string &key, const JsonDocument &doc);

        /**
         * @brief Reads and parses a JSON file from LittleFS.
         * @param path Absolute LittleFS file path.
         * @return Parsed JsonDocument; empty document on read or parse error.
         */
        JsonDocument readJsonFile(const std::string &path);

        /**
         * @brief Serializes a JsonDocument and writes it to a LittleFS file.
         * @param path Absolute LittleFS file path.
         * @param doc JsonDocument to serialize.
         */
        void writeJsonFile(const std::string &path, const JsonDocument &doc);

        /** @deprecated Use readJsonConfig instead. */
        inline JsonDocument readJson(const std::string &key) { return readJsonConfig(key); }

        /** @deprecated Use writeJsonConfig instead. */
        inline void writeJson(const std::string &key, const JsonDocument &doc) { writeJsonConfig(key, doc); }
    };

    /** @brief Global FileHandler instance shared across the framework. */
    extern FileHandler fileHandler;

    /** @brief Absolute LittleFS path of the temporary upload folder. */
    extern std::string FOLDER_TEMP;

    /** @brief Absolute LittleFS path of the web assets root folder. */
    extern std::string FOLDER_WEB;
}
