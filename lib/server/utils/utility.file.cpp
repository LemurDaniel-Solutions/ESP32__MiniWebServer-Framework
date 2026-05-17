// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <vector>
#include <string>

#include <utils/utility.file.h>

namespace EspWeb
{

    FileHandler fileHandler;
    std::string FOLDER_TEMP = "/tmp";
    std::string FOLDER_WEB = "/web";

    std::string FileHandler::getTempFolder()
    {
        return _tempFolder;
    }

    std::string FileHandler::getTempFile()
    {
        return _tempFolder + "/" + randomString(8);
    }

    bool FileHandler::exists(const std::string &path)
    {
        if (!LittleFS.begin())
        {
            Serial.println("Failed to mount LittleFs");
            return false;
        }

        return LittleFS.exists(path.c_str());
    }

    FileInfo FileHandler::getInfo(const std::string &path)
    {
        FileInfo info;

        if (!LittleFS.begin())
        {
            Serial.println("Failed to mount LittleFs");
            return info;
        }

        File file = LittleFS.open(path.c_str(), "r");
        if (!file)
        {
            Serial.println(("Failed to open file: " + path).c_str());
            return info;
        }

        info.isDirectory = file.isDirectory();
        info.name = file.name();
        info.path = file.path();

        info.baseName = info.name;
        info.extension = "";

        size_t posDot = info.name.rfind('.');
        if (posDot != std::string::npos && posDot > 0)
        {
            info.extension = info.name.substr(posDot + 1);
            info.baseName = info.name.substr(0, posDot);
        }

        file.close();

        return info;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Helper Methods
     *
     **/

    std::string FileHandler::randomString(int size)
    {
        std::string charSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::string result;
        for (int i = 0; i < size; i++)
        {
            int index = random(0, charSet.length());
            result += charSet[index];
        }
        return result;
    }

    std::string FileHandler::trim(const std::string &s)
    {
        size_t start = s.find_first_not_of(" \t");
        if (start == std::string::npos)
            return {};
        size_t end = s.find_last_not_of(" \t");
        return s.substr(start, end - start + 1);
    }

    std::vector<std::string> FileHandler::split(const std::string &text, const std::string &splitter)
    {
        std::vector<std::string> elements;

        size_t pos = 0;
        while (pos < text.size())
        {
            size_t end = text.find(splitter, pos);
            if (end == std::string::npos)
                end = text.size();

            elements.push_back(trim(text.substr(pos, end - pos)));
            pos = end + splitter.size();
        }

        return elements;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Folder and Folder Contents
     *
     **/

    std::vector<FileInfo> FileHandler::listFiles(const std::string &path)
    {
        std::vector<FileInfo> files;

        if (!LittleFS.begin())
        {
            Serial.println("Failed to mount LittleFs");
            return files;
        }

        File folder = LittleFS.open(path.c_str());
        if (!folder || !folder.isDirectory())
        {
            return files;
        }

        std::string filePath = folder.getNextFileName().c_str();
        while (!filePath.empty())
        {
            FileInfo info = getInfo(filePath);
            files.push_back(info);
            filePath = folder.getNextFileName().c_str();
        }

        return files;
    }

    void FileHandler::clearFolder(const std::string &path)
    {
        if (!LittleFS.begin())
        {
            Serial.println("Failed to mount LittleFs");
            return;
        }

        File dir = LittleFS.open(path.c_str());
        if (!dir)
        {
            LittleFS.mkdir(path.c_str());
            return;
        }

        if (!dir.isDirectory())
        {
            dir.close();
            return;
        }

        File entry = dir.openNextFile();
        while (entry)
        {
            std::string entryPath = entry.path();
            bool isDir = entry.isDirectory();
            entry.close();

            if (isDir)
                removeFolder(entryPath);
            else
                LittleFS.remove(entryPath.c_str());

            entry = dir.openNextFile();
        }
        dir.close();
    }

    void FileHandler::removeFolder(const std::string &path)
    {
        clearFolder(path);
        LittleFS.rmdir(path.c_str());
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Files
     *
     **/

    bool FileHandler::removeFile(const std::string &path)
    {
        if (!LittleFS.begin())
        {
            Serial.println("Failed to mount LittleFs");
            return false;
        }

        if (!LittleFS.exists(path.c_str()))
        {
            Serial.printf("ℹ️ No file found at %s to remove.\n", path.c_str());
            return false;
        }

        return LittleFS.remove(path.c_str());
    }

    bool FileHandler::moveFile(const std::string &path, const std::string &destination)
    {
        if (!LittleFS.begin())
        {
            Serial.println("Failed to mount LittleFs");
            return false;
        }

        if (!LittleFS.exists(path.c_str()))
        {
            Serial.printf("ℹ️ No file found at %s to move.\n", path.c_str());
            return false;
        }

        const FileInfo info = getInfo(path);
        std::string destinationPath = destination + "/" + info.name;

        Serial.printf("ℹ️ Move from %s to %s\n", path.c_str(), destinationPath.c_str());

        return LittleFS.rename(path.c_str(), destinationPath.c_str());
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Read and write JSON
     *
     **/

    JsonDocument FileHandler::readJson(const std::string &path)
    {
        if (!LittleFS.begin())
        {
            Serial.println("Failed to mount LittleFs");
            return JsonDocument();
        }

        if (!LittleFS.exists(path.c_str()))
        {
            Serial.printf("❌ CRITICAL: JSON file %s not found!\n", path.c_str());
            return JsonDocument();
        }

        File file = LittleFS.open(path.c_str(), "r");
        if (!file)
        {
            Serial.printf("❌ CRITICAL: Failed to open JSON file %s for reading!\n", path.c_str());
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
            Serial.printf("❌ CRITICAL: Failed to parse JSON file %s! Error: %s\n", path.c_str(), error.c_str());
            return JsonDocument();
        }

        return doc;
    }

    void FileHandler::writeJson(const std::string &path, const JsonDocument &doc)
    {
        if (!LittleFS.begin())
        {
            Serial.println("Failed to mount LittleFs");
            return;
        }

        File file = LittleFS.open(path.c_str(), "w", true);
        if (!file)
        {
            Serial.printf("❌ CRITICAL: Failed to open JSON file %s for writing!\n", path.c_str());
            return;
        }

        std::string jsonStr;
        serializeJson(doc, jsonStr);

        file.print(jsonStr.c_str());
        file.close();

        Serial.printf("✅ Successfully wrote JSON file %s\n", path.c_str());
    }
}
