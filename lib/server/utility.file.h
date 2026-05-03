// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <Arduino.h>
#include <LittleFS.h>

#include <ArduinoJson.h>
#include <ESP32-targz.h>

namespace ESP32WebServer
{

    const std::string TEMP_FOLDER = "/tmp";

    inline bool fileExists(const std::string &filePath)
    {
        return LittleFS.exists(filePath.c_str());
    }

    struct FileInfo
    {
        std::string name;
        std::string path;
        std::string extension;
        std::string baseName;
    };

    inline std::string randomString(int size = 32)
    {
        std::string charSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::string result;
        for (int i = 0; i < size; i++)
        {
            int index = random(0, charSet.size());
            result += charSet[index];
        }
        return result;
    }

    inline std::string getTempFolder()
    {
        return TEMP_FOLDER + randomString(8);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Unzipping
     *
     **/

    inline std::string unzip(const std::string &filePath)
    {
        const std::string unzippedFolder = getTempFolder();
        LittleFS.mkdir(unzippedFolder.c_str());

        TarUnpacker *TARUnpacker = new TarUnpacker();

        TARUnpacker->haltOnError(true);                                                            // stop on fail (manual restart/reset required)
        TARUnpacker->setTarVerify(true);                                                           // true = enables health checks but slows down the overall process
        TARUnpacker->setupFSCallbacks(targzTotalBytesFn, targzFreeBytesFn);                        // prevent the partition from exploding, recommended
        TARUnpacker->setTarProgressCallback(BaseUnpacker::defaultProgressCallback);                // prints the untarring progress for each individual file
        TARUnpacker->setTarStatusProgressCallback(BaseUnpacker::defaultTarStatusProgressCallback); // print the filenames as they're expanded
        TARUnpacker->setTarMessageCallback(BaseUnpacker::targzPrintLoggerCallback);                // tar log verbosity

        if (!TARUnpacker->tarExpander(LittleFS, filePath.c_str(), LittleFS, unzippedFolder.c_str()))
        {
            Serial.printf("tarExpander failed with return code #%d\n", TARUnpacker->tarGzGetError());
        }

        return unzippedFolder;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Folder and Folder Contents
     *
     **/

    inline std::vector<FileInfo> listFiles(const std::string &folderPath, std::vector<FileInfo> &files, const std::string &prefix = "")
    {
        File folder = LittleFS.open(folderPath.c_str());
        if (!folder || !folder.isDirectory())
        {
            throw std::runtime_error("Path is not a directory");
        }

        File file = folder.openNextFile();
        while (file)
        {
            if (file.isDirectory())
            {
                listFiles(folder.path(), files, prefix + "/" + folder.name());
                continue;
            }

            FileInfo info;
            info.name = file.name();
            info.path = file.path();
            info.baseName = info.name;
            info.extension = "";

            size_t dot = info.name.find_last_of('.');
            if (dot != std::string::npos && dot > 0)
            {
                info.extension = info.name.substr(dot);
                info.baseName = info.name.substr(0, dot);
            }

            files.push_back(info);
            file = folder.openNextFile();
        }

        return files;
    }

    inline std::vector<FileInfo> listFiles(const std::string &folderPath)
    {
        std::vector<FileInfo> files;
        return listFiles(folderPath, files);
    }

    static void clearFolder(const std::string &folderPath)
    {
        File dir = LittleFS.open(folderPath.c_str());
        if (!dir || !dir.isDirectory())
        {
            LittleFS.mkdir(folderPath.c_str());
            return;
        }
        File entry = dir.openNextFile();
        while (entry)
        {
            LittleFS.remove(entry.path());
            entry = dir.openNextFile();
        }
        dir.close();
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Files
     *
     **/

    inline int removeFile(const std::string &filePath)
    {
        if (fileExists(filePath))
        {
            if (LittleFS.remove(filePath.c_str()))
            {
                Serial.printf("✅ Successfully removed file %s\n", filePath.c_str());
                return 0;
            }
            else
            {
                Serial.printf("❌ CRITICAL: Failed to remove file %s!\n", filePath.c_str());
                return -1;
            }
        }
        else
        {
            Serial.printf("ℹ️ No file found at %s to remove.\n", filePath.c_str());
            return -1;
        }
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Read and write JSON
     *
     **/

    inline JsonDocument readJsonFile(const std::string &filePath)
    {
        if (!fileExists(filePath))
        {
            Serial.printf("❌ CRITICAL: JSON file %s not found!\n", filePath.c_str());
            return JsonDocument();
        }

        File file = LittleFS.open(filePath.c_str(), "r");
        if (!file)
        {
            Serial.printf("❌ CRITICAL: Failed to open JSON file %s for reading!\n", filePath.c_str());
            return JsonDocument();
        }

        size_t size = file.size();
        std::string jsonStr(size, '\0');
        file.readBytes(&jsonStr[0], size);
        file.close();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonStr);
        if (error)
        {
            Serial.printf("❌ CRITICAL: Failed to parse JSON file %s! Error: %s\n", filePath.c_str(), error.c_str());
            return JsonDocument();
        }

        return doc;
    }

    inline bool writeJsonFile(const std::string &filePath, const JsonDocument &doc)
    {
        File file = LittleFS.open(filePath.c_str(), "w");
        if (!file)
        {
            throw "CRITICAL: Failed to open File to write";
        }

        std::string jsonStr;
        serializeJson(doc, jsonStr);

        file.print(jsonStr.c_str());
        file.close();

        Serial.printf("✅ Successfully wrote JSON file %s\n", filePath.c_str());

        return true;
    }

}