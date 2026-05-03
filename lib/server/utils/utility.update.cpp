// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <utils/utility.update.h>

namespace ESP32WebServer
{

    void post_UpdateWebsite(Request &req, Response &res)
    {

        const std::string &filePath = req.body.file("web.tar.gz");
        Serial.print("File located at: ");
        Serial.println(filePath.c_str());

        clearFolder(FOLDER_WEB);
        unzip(filePath, FOLDER_WEB);
    }

    void post_UpdateCode(Request &req, Response &res)
    {
        size_t updateSize = req.body.contentLength;

        if (Update.begin(req.body.contentLength))
        {
            uint8_t buff[1024];
            int available = 0;
            do
            {
                available = req.body.chunks(buff, sizeof(buff));
                if (available > 0)
                    Update.write(buff, available);

            } while (available > 0);

            if (Update.end() && Update.isFinished())
            {
                post_AdminRestart(req, res);
            }
            else
            {
                Serial.println(Update.getError());
            }
        }
    }

}
