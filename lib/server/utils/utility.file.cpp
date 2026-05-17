// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <utils/utility.file.h>

namespace EspWeb
{

    std::string FOLDER_TEMP = "/tmp";
    std::string FOLDER_WEB = "/web";

    static JsonFileHandler::JsonFileHandler fileHandler;

    std::string getTempFolder()
    {
        return fileHandler.getTempFolder();
    }

    bool fileExists(const std::string &filePath)
    {
        return fileHandler.exists(filePath);
    }

    FileInfo getFileInfo(const std::string &filePath)
    {
        return fileHandler.getInfo(filePath);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Helper Methods
     *
     **/

    std::string randomString(int size)
    {
        return fileHandler.randomString(size);
    }

    std::string trim(const std::string &s)
    {
        size_t start = s.find_first_not_of(" \t");
        if (start == std::string::npos)
            return {};
        size_t end = s.find_last_not_of(" \t");
        return s.substr(start, end - start + 1);
    }

    std::vector<std::string> split(const std::string &text, const std::string &splitter)
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

    std::vector<FileInfo> listFiles(const std::string &folderPath)
    {
        return fileHandler.listFiles(folderPath);
    }

    void clearFolder(const std::string &folderPath)
    {
        fileHandler.clearFolder(folderPath);
    }

    void removeFolder(const std::string &folderPath)
    {
        fileHandler.removeFolder(folderPath);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Files
     *
     **/

    int removeFile(const std::string &filePath)
    {
        return fileHandler.removeFile(filePath) ? 0 : -1;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Read and write JSON
     *
     **/

    JsonDocument readJsonFile(const std::string &filePath)
    {
        return fileHandler.readJson(filePath);
    }

    bool writeJsonFile(const std::string &filePath, const JsonDocument &doc)
    {
        fileHandler.writeJson(filePath, doc);
        return true;
    }
}
