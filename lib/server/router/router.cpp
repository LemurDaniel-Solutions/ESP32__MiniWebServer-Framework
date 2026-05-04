// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <router/router.h>

namespace EspWeb
{

    /*-------------------------------------------------------------------------------------------------
     *
     * TokenManager
     *
     **/

    bool TokenManager::setSalt(const std::string &salt)
    {
        JsonDocument doc = JsonDocument();

        if (fileExists(ADMIN_CREDENTIALS_FILE))
            doc = readJsonFile(ADMIN_CREDENTIALS_FILE);

        doc["admin_salt"] = salt;
        writeJsonFile(ADMIN_CREDENTIALS_FILE, doc);
        return true;
    }

    bool TokenManager::setCredentials(const std::string &username, const std::string &password)
    {
        JsonDocument doc = JsonDocument();

        if (fileExists(ADMIN_CREDENTIALS_FILE))
            doc = readJsonFile(ADMIN_CREDENTIALS_FILE);

        const std::string password_hash = generateSHA256(password, getCredentials()[0]);
        doc["admin_user"] = username;
        doc["admin_pwd"] = password_hash;
        writeJsonFile(ADMIN_CREDENTIALS_FILE, doc);
        return true;
    }

    std::vector<std::string> TokenManager::getCredentials()
    {
        std::vector<std::string> creds;
        const JsonDocument &doc = readJsonFile(ADMIN_CREDENTIALS_FILE);

        if (doc["admin_salt"].is<std::string>())
            creds.push_back(doc["admin_salt"].as<std::string>());
        else
        {
            const std::string salt = randomString(32);
            creds.push_back(salt);
            setSalt(salt);
        }

        creds.push_back(doc["admin_user"].is<std::string>() ? doc["admin_user"].as<std::string>() : DEFAULT_ADMIN_USER);
        creds.push_back(doc["admin_pwd"].is<std::string>() ? doc["admin_pwd"].as<std::string>() : generateSHA256(DEFAULT_ADMIN_PWD, creds[0]));

        return creds;
    }

    bool TokenManager::checkCredentials(const std::string &username, const std::string &password)
    {
        const std::vector<std::string> &creds = getCredentials();
        return username == creds[1] && generateSHA256(password, creds[0]) == creds[2];
    }

    std::string TokenManager::generateSHA256(const std::string &text, const std::string &salt)
    {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, (unsigned char *)text.c_str(), text.length());
        mbedtls_sha256_update(&ctx, (unsigned char *)salt.c_str(), salt.length());

        unsigned char shaResult[32];
        mbedtls_sha256_finish(&ctx, shaResult);
        mbedtls_sha256_free(&ctx);

        std::string hashStr;
        for (int i = 0; i < 32; i++)
        {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02x", shaResult[i]);
            hashStr += buf;
        }
        return hashStr;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Session Tokens
     *
     **/
    void TokenManager::addToken(const std::string &token)
    {
        _ADMIN_TOKENS.insert({token, (millis() / 1000) + 3600});
    }

    void TokenManager::removeToken(const std::string &token)
    {
        _ADMIN_TOKENS.erase(_ADMIN_TOKENS.find(token));
    }

    std::string TokenManager::getToken()
    {
        const std::string &token = generateSHA256(randomString() + String(millis()).c_str(), getCredentials()[0]);
        addToken(token);
        return token;
    }

    bool TokenManager::checkToken(const std::string &authToken)
    {
        unsigned long currentTime = millis() / 1000;
        const auto &entry = _ADMIN_TOKENS.find(authToken);
        if (entry == _ADMIN_TOKENS.end())
            return false;
        if (currentTime > entry->second)
        {
            _ADMIN_TOKENS.erase(entry);
            return false;
        }
        return true;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * API Tokens
     *
     **/
    std::string TokenManager::addPermToken(const std::string &name)
    {
        JsonDocument doc = readJsonFile(ADMIN_PERMANENT_TOKENS);
        const std::string &token = generateSHA256(randomString() + String(millis()).c_str(), name);
        doc[name] = token;
        _PERM_TOKENS.insert({token, name});
        writeJsonFile(ADMIN_PERMANENT_TOKENS, doc);
        return doc[name].as<std::string>();
    }

    void TokenManager::removePermToken(const std::string &name)
    {
        JsonDocument doc = readJsonFile(ADMIN_PERMANENT_TOKENS);
        doc.remove(name);
        _hasReadFile = false;
        _PERM_TOKENS = {};
        writeJsonFile(ADMIN_PERMANENT_TOKENS, doc);
    }

    bool TokenManager::checkPermToken(const std::string &authToken)
    {
        if (!_hasReadFile)
            listPermTokens();
        return _PERM_TOKENS.find(authToken) != _PERM_TOKENS.end();
    }

    std::vector<std::string> TokenManager::listPermTokens()
    {
        if (!_hasReadFile)
        {
            JsonDocument doc = readJsonFile(ADMIN_PERMANENT_TOKENS);
            for (JsonPair entry : doc.as<JsonObject>())
                _PERM_TOKENS.insert({entry.value().as<std::string>(), entry.key().c_str()});
            _hasReadFile = true;
        }

        std::vector<std::string> result;
        for (const auto &entry : _PERM_TOKENS)
            result.push_back(entry.second);
        return result;
    }

    bool TokenManager::isPermTokenValid(Request &req)
    {
        const auto &header = req.headers.find("Authorization");
        if (header == req.headers.end())
            return false;
        return checkPermToken(header->second);
    }

    bool TokenManager::isSessionTokenValid(Request &req)
    {
        const auto &entry = req.cookies.find(DEFAULT_ADMIN_COOKIE);
        if (entry == req.cookies.end())
            return false;
        return checkToken(entry->second);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Router
     *
     **/

    Router::Router() {}

    void Router::route(const std::string &method, const std::string &path, std::vector<RequestHandler> handlers)
    {
        routes.push_back({method, path, handlers});
    }

    void Router::route(const std::string &method, const std::string &path, RequestHandler handler)
    {
        route(method, path, std::vector<RequestHandler>{handler});
    }

    void Router::use(const std::string &prefix, const RequestHandler &handler)
    {
        const auto &entry = middlewares.find(prefix);
        if (entry != middlewares.end())
        {
            std::vector<RequestHandler> &list = entry->second;
            list.push_back(handler);
        }
        else
        {
            std::vector<RequestHandler> list{handler};
            middlewares.insert({prefix, list});
        }
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Middleware
     *
     **/

    RequestHandler Router::cors(const std::string &origin)
    {
        return [origin](Request &req, Response &res)
        {
            res.header("Access-Control-Allow-Origin", origin);
        };
    }

    RequestHandler Router::auth()
    {
        return [](Request &req, Response &res)
        {
            if (!TokenManager::instance().isPermTokenValid(req) &&
                !TokenManager::instance().isSessionTokenValid(req))
                res.Unauthorized().finalize();
        };
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Make Token Handleing available in Router
     *
     **/

    std::string Router::getSessionToken()
    {
        return TokenManager::instance().getToken();
    }

    std::string Router::getApiToken(const std::string &name)
    {
        return TokenManager::instance().addPermToken(name);
    }
    void Router::removeApiToken(const std::string &name)
    {
        TokenManager::instance().removePermToken(name);
    }

    bool Router::isApiTokenValid(Request &req)
    {
        return TokenManager::instance().isPermTokenValid(req);
    }
    bool Router::isSessionTokenValid(Request &req)
    {
        return TokenManager::instance().isSessionTokenValid(req);
    }

}
