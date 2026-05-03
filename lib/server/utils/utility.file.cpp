// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <utils/utility.file.h>

namespace ESP32WebServer
{

    std::string getTempFolder()
    {
        return std::string(FOLDER_TEMP) + "/" + randomString(8);
    }

    bool fileExists(const std::string &filePath)
    {
        return LittleFS.exists(filePath.c_str());
    }

    FileInfo getFileInfo(const std::string &filePath)
    {
        File file = LittleFS.open(filePath.c_str(), "r");

        FileInfo info;
        info.isDirectory = file.isDirectory();
        info.name = file.name();
        info.path = file.path();
        info.baseName = info.name;
        info.extension = "";

        size_t dot = info.name.find_first_of('.');
        if (dot != std::string::npos && dot > 0)
        {
            info.extension = info.name.substr(dot);
            info.baseName = info.name.substr(0, dot);
        }

        file.close();

        return info;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Helper Methods
     *
     **/

    std::string randomString(int size)
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

    std::string trim(const std::string &s)
    {
        size_t start = s.find_first_not_of(" \t");
        if (start == std::string::npos)
            return {};
        size_t end = s.find_last_not_of(" \t");
        return s.substr(start, end - start + 1);
    }

    size_t findBytes(const std::vector<uint8_t> &data, const std::string &pattern, size_t start)
    {
        size_t pat_length = pattern.length();
        if (pat_length == 0 || data.size() < pat_length || start > data.size() - pat_length)
            return std::string::npos;

        for (size_t i = start; i <= data.size() - pat_length; ++i)
        {
            if (memcmp(data.data() + i, pattern.c_str(), pat_length) == 0)
                return i;
        }

        return std::string::npos;
    }

    std::string extractString(const std::vector<uint8_t> &data, size_t start, size_t end)
    {
        return std::string(reinterpret_cast<const char *>(data.data() + start), end - start);
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
     * Unzipping
     *
     **/

    void unzip(const std::string &filePath, const std::string &targetFolder)
    {
        LittleFS.mkdir(targetFolder.c_str());

        const FileInfo &info = getFileInfo(filePath);

        Serial.printf("Extension is %s", info.extension.c_str());
        if (info.extension == ".tar")
        {
            TarUnpacker *TARUnpacker = new TarUnpacker();

            TARUnpacker->haltOnError(false);
            TARUnpacker->setTarVerify(false);
            TARUnpacker->setupFSCallbacks([]()
                                          { return (unsigned long long)LittleFS.totalBytes(); }, []()
                                          { return (unsigned long long)LittleFS.totalBytes() - LittleFS.usedBytes(); });
            TARUnpacker->setTarProgressCallback(BaseUnpacker::defaultProgressCallback);
            TARUnpacker->setTarStatusProgressCallback(BaseUnpacker::defaultTarStatusProgressCallback);
            TARUnpacker->setTarMessageCallback(BaseUnpacker::targzPrintLoggerCallback);

            if (!TARUnpacker->tarExpander(LittleFS, filePath.c_str(), LittleFS, targetFolder.c_str()))
            {
                Serial.printf("tarExpander failed with return code #%d\n", TARUnpacker->tarGzGetError());
            }
        }
        else if (info.extension == ".tar.gz")
        {
            TarGzUnpacker *TARGZUnpacker = new TarGzUnpacker();

            TARGZUnpacker->haltOnError(false);
            TARGZUnpacker->setTarVerify(false);
            TARGZUnpacker->setupFSCallbacks([]()
                                            { return (unsigned long long)LittleFS.totalBytes(); }, []()
                                            { return (unsigned long long)LittleFS.totalBytes() - LittleFS.usedBytes(); });
            TARGZUnpacker->setGzProgressCallback(BaseUnpacker::defaultProgressCallback);                 // targzNullProgressCallback or defaultProgressCallback
            TARGZUnpacker->setLoggerCallback(BaseUnpacker::targzPrintLoggerCallback);                    // gz log verbosity
            TARGZUnpacker->setTarProgressCallback(BaseUnpacker::defaultProgressCallback);                // prints the untarring progress for each individual file
            TARGZUnpacker->setTarStatusProgressCallback(BaseUnpacker::defaultTarStatusProgressCallback); // print the filenames as they're expanded
            TARGZUnpacker->setTarMessageCallback(BaseUnpacker::targzPrintLoggerCallback);                // tar log verbosity

            if (!TARGZUnpacker->tarGzExpander(LittleFS, filePath.c_str(), LittleFS, targetFolder.c_str(), nullptr))
            {
                Serial.printf("tarGzExpander+intermediate file failed with return code #%d\n", TARGZUnpacker->tarGzGetError());
            }
        }
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Folder and Folder Contents
     *
     **/

    std::vector<FileInfo> listFiles(const std::string &folderPath)
    {
        std::vector<FileInfo> files;

        File folder = LittleFS.open(folderPath.c_str());
        if (!folder || !folder.isDirectory())
        {
            throw std::runtime_error("Path is not a directory");
        }

        std::string filePath = folder.getNextFileName().c_str();
        while (!filePath.empty())
        {
            const FileInfo &info = getFileInfo(filePath);

            files.push_back(info);
            filePath = folder.getNextFileName().c_str();
        }

        return files;
    }

    void clearFolder(const std::string &folderPath)
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
            if (entry.isDirectory())
            {
                std::string path = entry.path();
                entry.close();
                removeFolder(path);
            }
            else
            {
                std::string path = entry.path();
                entry.close();
                LittleFS.remove(path.c_str());
                entry = dir.openNextFile();
            }
        }
        dir.close();
    }

    void removeFolder(const std::string &folderPath)
    {
        clearFolder(folderPath);
        LittleFS.rmdir(folderPath.c_str());
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle Files
     *
     **/
    void moveFile(const std::string &filePath, const std::string &destinationFolder, const std::string &relativePath)
    {
        if (!fileExists(filePath))
        {
            Serial.printf("❌ CRITICAL: Failed to move file %s!\n", filePath.c_str());
            return;
        }

        const FileInfo &info = getFileInfo(filePath);
        std::string destinationPath = destinationFolder + "/" + info.name;

        if (!relativePath.empty())
        {
            std::string suffix = filePath.substr(relativePath.size());
            std::string newPath = destinationFolder + suffix;
        }

        Serial.printf("ℹ️ Move from %s to %s", filePath.c_str(), destinationPath.c_str());
        // LittleFS.rename(filePath.c_str(), destinationPath.c_str());
    }

    int removeFile(const std::string &filePath)
    {
        if (!fileExists(filePath))
        {
            Serial.printf("ℹ️ No file found at %s to remove.\n", filePath.c_str());
            return -1;
        }

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

    /*-------------------------------------------------------------------------------------------------
     *
     * Read and write JSON
     *
     **/

    JsonDocument readJsonFile(const std::string &filePath)
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

    bool writeJsonFile(const std::string &filePath, const JsonDocument &doc)
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
