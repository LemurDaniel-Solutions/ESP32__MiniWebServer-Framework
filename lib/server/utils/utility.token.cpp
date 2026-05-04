// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <utils/utility.token.h>

namespace EspWeb
{
    /*-------------------------------------------------------------------------------------------------
     *
     * Handle config file
     *
     **/

    void TokenManager::setValue(const std::string &key, const std::string &value)
    {
        if (_admin)
            (*_admin)[key] = value;

        JsonDocument doc = readJsonFile(ADMIN_CREDENTIALS_FILE);
        doc[key] = value;
        writeJsonFile(ADMIN_CREDENTIALS_FILE, doc);
    }

    std::string TokenManager::getValue(const std::string &key)
    {
        if (_admin == nullptr)
        {
            _admin = new JsonDocument();
            *_admin = readJsonFile(ADMIN_CREDENTIALS_FILE);
        }

        if ((*_admin)[key].is<std::string>())
            return (*_admin)[key].as<std::string>();

        else if (key == "admin_salt")
        {
            const std::string salt = randomString(32);
            setSalt(salt);
            return salt;
        }
        else if (key == "admin_user")
            return DEFAULT_ADMIN_USER;
        else if (key == "admin_pwd")
        {
            const std::string hashed = generateSHA256(DEFAULT_ADMIN_PWD);
            setValue("admin_pwd", hashed);
            return hashed;
        }

        return "";
    }

    std::string TokenManager::generateSHA256(const std::string &text)
    {
        const std::string &salt = getValue("admin_salt");

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
     * Set / Get Credentials
     *
     **/

    void TokenManager::setSalt(const std::string &salt)
    {
        setValue("admin_salt", salt);
    }

    void TokenManager::setCredentials(const std::string &username, const std::string &password)
    {
        const std::string password_hash = generateSHA256(password);
        setValue("admin_user", username);
        setValue("admin_pwd", password_hash);
    }

    bool TokenManager::checkCredentials(const std::string &username, const std::string &password)
    {
        return username == getValue("admin_user") && generateSHA256(password) == getValue("admin_pwd");
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Session Tokens
     *
     **/

    void TokenManager::removeToken(const std::string &token)
    {
        _ADMIN_TOKENS.erase(_ADMIN_TOKENS.find(token));
    }

    std::string TokenManager::getToken()
    {
        const std::string &token = generateSHA256(randomString());
        _ADMIN_TOKENS.insert({token, (millis() / 1000) + 3600});
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
        const std::string &token = generateSHA256(randomString());

        if (_PERM_TOKENS == nullptr)
            listPermTokens();

        doc[name] = token;
        (*_PERM_TOKENS).insert({token, name});
        writeJsonFile(ADMIN_PERMANENT_TOKENS, doc);

        return doc[name].as<std::string>();
    }

    void TokenManager::removePermToken(const std::string &name)
    {
        JsonDocument doc = readJsonFile(ADMIN_PERMANENT_TOKENS);
        doc.remove(name);
        _PERM_TOKENS = nullptr;
        writeJsonFile(ADMIN_PERMANENT_TOKENS, doc);
    }

    bool TokenManager::checkPermToken(const std::string &authToken)
    {
        if (_PERM_TOKENS == nullptr)
            listPermTokens();
        return (*_PERM_TOKENS).find(authToken) != (*_PERM_TOKENS).end();
    }

    std::vector<std::string> TokenManager::listPermTokens()
    {
        if (_PERM_TOKENS == nullptr)
        {
            _PERM_TOKENS = new std::map<std::string, std::string>();
            JsonDocument doc = readJsonFile(ADMIN_PERMANENT_TOKENS);
            for (JsonPair entry : doc.as<JsonObject>())
                (*_PERM_TOKENS).insert({entry.value().as<std::string>(), entry.key().c_str()});
        }

        std::vector<std::string> result;
        for (const auto &entry : (*_PERM_TOKENS))
            result.push_back(entry.second);

        return result;
    }

}