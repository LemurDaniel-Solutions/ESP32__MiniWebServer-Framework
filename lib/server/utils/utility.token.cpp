// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <utils/utility.token.h>

namespace EspWeb
{
    std::vector<std::string> TOKEN_ACTIONS = {"admin"};
    /*-------------------------------------------------------------------------------------------------
     *
     * Token
     *
     **/

    Token::Token(std::string name, std::string value, std::vector<std::string> action)
    {
        this->name = name;
        this->value = value;
        this->action = action;
    }

    Token Token::NullToken()
    {
        Token token("", "", {});
        token._isValid = false;
        return token;
    }

    Token Token::getSessionToken(long durationSeconds, const std::vector<std::string> &action)
    {
        std::string value = TokenManager::instance().generateSHA256(fileHandler.randomString());
        Token token("Session Token", value, action);
        token._expires = ::millis() / 1000 + durationSeconds;
        return token;
    }

    Token Token::getApiToken(const std::string &name, const std::vector<std::string> &action)
    {
        std::string value = TokenManager::instance().generateSHA256(fileHandler.randomString());
        return Token(name, value, action);
    }

    const long Token::expires() { return _expires; }
    bool Token::isPrivileged() { return isAllowed("admin"); }
    bool Token::isValid() { return _isValid; }

    bool Token::isAllowed(const std::string &action)
    {
        if (!_isValid)
            return false;

        for (const std::string &entry : this->action)
            if (entry == action)
                return true;

        return false;
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Handle config file
     *
     **/

    void TokenManager::setValue(const std::string &key, const std::string &value)
    {
        if (_admin)
            (*_admin)[key] = value;

        JsonDocument doc = fileHandler.readJson(ADMIN_CREDENTIALS_FILE);
        doc[key] = value;
        fileHandler.writeJson(ADMIN_CREDENTIALS_FILE, doc);
    }

    std::string TokenManager::getValue(const std::string &key)
    {
        if (_admin == nullptr)
        {
            _admin = new JsonDocument();
            *_admin = fileHandler.readJson(ADMIN_CREDENTIALS_FILE);
        }

        if ((*_admin)[key].is<std::string>())
            return (*_admin)[key].as<std::string>();

        else if (key == "admin_salt")
        {
            const std::string salt = fileHandler.randomString(32);
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
        auto it = _SESSION_TOKENS.find(token);
        if (it != _SESSION_TOKENS.end())
            _SESSION_TOKENS.erase(it);
    }

    Token TokenManager::getSessionToken(long seconds, const std::vector<std::string> &actions)
    {
        Token token = Token::getSessionToken(seconds, actions);
        _SESSION_TOKENS.insert({token.value, token});
        return token;
    }

    Token TokenManager::checkToken(const std::string &authToken)
    {

        // Session Token check
        long currentTime = millis() / 1000;
        auto entry = _SESSION_TOKENS.find(authToken);
        if (entry != _SESSION_TOKENS.end())
        {
            if (currentTime > entry->second.expires())
            {
                _SESSION_TOKENS.erase(entry);
            }
            else
            {
                return entry->second;
            }
        }

        // API Token Check
        if (_API_TOKENS == nullptr)
            listApiTokens();

        entry = _API_TOKENS->find(authToken);
        if (entry != _API_TOKENS->end())
        {
            return entry->second;
        }

        return Token::NullToken();
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * API Tokens
     *
     **/
    Token TokenManager::getApiToken(const std::string &name, const std::vector<std::string> &action)
    {
        const Token &token = Token::getApiToken(name, action);

        JsonDocument doc = fileHandler.readJson(ADMIN_PERMANENT_TOKENS);

        JsonObject tokenObj = doc[name].to<JsonObject>();
        tokenObj["value"] = token.value;
        JsonArray actionArr = tokenObj["action"].to<JsonArray>();
        for (const std::string &entry : token.action)
            actionArr.add(entry);

        fileHandler.writeJson(ADMIN_PERMANENT_TOKENS, doc);

        delete _API_TOKENS;
        _API_TOKENS = nullptr;
        listApiTokens();

        return token;
    }

    void TokenManager::removeApiToken(const std::string &name)
    {
        JsonDocument doc = fileHandler.readJson(ADMIN_PERMANENT_TOKENS);
        doc.remove(name);
        delete _API_TOKENS;
        _API_TOKENS = nullptr;
        fileHandler.writeJson(ADMIN_PERMANENT_TOKENS, doc);
    }

    std::vector<Token> TokenManager::listApiTokens()
    {
        if (_API_TOKENS == nullptr)
        {
            _API_TOKENS = new std::map<std::string, Token>();
            JsonDocument doc = fileHandler.readJson(ADMIN_PERMANENT_TOKENS);
            for (JsonPair kv : doc.as<JsonObject>())
            {
                std::vector<std::string> actions;
                for (JsonVariant v : kv.value()["action"].as<JsonArray>())
                    actions.push_back(v.as<std::string>());

                Token token(
                    kv.key().c_str(),
                    kv.value()["value"].as<std::string>(),
                    actions);

                (*_API_TOKENS).insert({token.value, token});
            }
        }

        std::vector<Token> result;
        for (const auto &entry : (*_API_TOKENS))
            result.push_back(entry.second);

        return result;
    }

}