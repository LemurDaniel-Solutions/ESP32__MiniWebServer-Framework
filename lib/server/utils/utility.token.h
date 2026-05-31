// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#pragma once

#include <ArduinoJson.h>
#include <mbedtls/sha256.h>

#include <vector>
#include <string>
#include <map>

#include <utils/utility.file.h>

/** @brief LittleFS path for the persistent API token store. */
#define ADMIN_PERMANENT_TOKENS "/admin_perm_token.json"

/** @brief LittleFS path for the admin credentials store. */
#define ADMIN_CREDENTIALS_FILE "/admin_credentials.json"

/** @brief Name of the session cookie used for admin authentication. */
#define DEFAULT_ADMIN_COOKIE "adminToken"

/** @brief Default admin username (should be changed in production). */
#define DEFAULT_ADMIN_USER "admin"

/** @brief Default admin password (should be changed in production). */
#define DEFAULT_ADMIN_PWD "admin"

namespace EspWeb
{
    /** @brief Global list of action names that can be granted to tokens. */
    extern std::vector<std::string> TOKEN_ACTIONS;

    /**
     * @brief Represents an authenticated token (session or API).
     *
     * Carries the token value, associated actions, and expiry time.
     * Use Token::NullToken() for unauthenticated / invalid states.
     */
    class Token
    {
        friend class TokenManager;

    private:
        /** @brief False if the token was explicitly invalidated or never initialized. */
        bool _isValid = true;

        /** @brief Unix timestamp when the token expires; -1 for non-expiring API tokens. */
        long _expires = -1;

        /** @brief True if this is a session token (time-limited, stored in memory). */
        bool _isSessionToken = false;

        /** @brief True if this is a permanent API token (no expiry, stored in LittleFS). */
        bool _isApiToken = false;


    public:
        Token() = default;

        /**
         * @brief Creates an invalid placeholder token.
         * @return A Token where isValid() returns false.
         */
        static Token NullToken();

        /**
         * @brief Creates a time-limited session token with a generated random value.
         * @param seconds Lifetime in seconds from now.
         * @param action  List of permitted action names.
         * @return A new Token with `_isSessionToken = true`.
         */
        static Token createSessionToken(long seconds, const std::vector<std::string> &action);

        /**
         * @brief Creates a permanent API token with a generated random value.
         * @param name   Human-readable identifier for the token.
         * @param action List of permitted action names.
         * @return A new Token with `_isApiToken = true`.
         */
        static Token createApiToken(const std::string &name, const std::vector<std::string> &action);

        /** @brief List of action names this token is authorized for. */
        std::vector<std::string> action;

        /** @brief The raw token string value. */
        std::string value;

        /** @brief Human-readable name / identifier for this token. */
        std::string name;

        /**
         * @brief Returns the Unix timestamp when this token expires.
         * @return Expiry timestamp, or -1 for non-expiring tokens.
         */
        const long expires();

        /**
         * @brief Checks whether this token has full admin (privileged) access.
         * @return `true` if the token carries all TOKEN_ACTIONS permissions.
         */
        bool isPrivileged();

        /**
         * @brief Checks whether this token is currently valid (not expired, not null).
         * @return `true` if valid.
         */
        bool isValid();

        /** @brief Returns `true` if this is a session token (time-limited, stored in memory). */
        bool isSessionToken() { return _isSessionToken; }

        /** @brief Returns `true` if this is a permanent API token (no expiry, stored in LittleFS). */
        bool isApiToken() { return _isApiToken; }

        /**
         * @brief Checks whether this token is authorized for a specific action.
         * @param action Action name to check.
         * @return `true` if the action is in the token's action list.
         */
        bool isAllowed(const std::string &action);
    };

    /**
     * @brief Singleton manager for session and API tokens and admin credentials.
     *
     * Handles SHA-256 hashing, credential verification, and persistent storage
     * of API tokens in LittleFS. Access via TokenManager::instance().
     */
    class TokenManager
    {
    private:
        /** @brief Cached admin credentials JSON document loaded from LittleFS. */
        JsonDocument *_admin = nullptr;

        /**
         * @brief Reads a value from the admin credentials store.
         * @param key Key to look up.
         * @return Value string, or empty string if not found.
         */
        std::string getValue(const std::string &key);

        /**
         * @brief Writes a value to the admin credentials store and persists it.
         * @param key   Key to write.
         * @param value Value to store.
         */
        void setValue(const std::string &key, const std::string &value);

        /** @brief In-memory map of active session token values → Token objects. */
        std::map<std::string, Token> _SESSION_TOKENS;

        /** @brief Lazily-loaded map of persistent API token names → Token objects. */
        std::map<std::string, Token> *_API_TOKENS = nullptr;

        TokenManager() {}
        TokenManager(const TokenManager &) = delete;
        void operator=(const TokenManager &) = delete;

    public:
        /**
         * @brief Returns the singleton TokenManager instance.
         * @return Reference to the global TokenManager.
         */
        static TokenManager &instance()
        {
            static TokenManager _instance;
            return _instance;
        }

        /**
         * @brief Computes the SHA-256 hex digest of a string.
         * @param text Input string to hash.
         * @return Lowercase hex-encoded SHA-256 digest.
         */
        std::string generateSHA256(const std::string &text);

        /*-------------------------------------------------------------------------------------------------
         *
         * Credentials
         *
         **/

        /**
         * @brief Sets the salt used when hashing admin passwords.
         * @param salt Salt string appended before hashing.
         */
        void setSalt(const std::string &salt);

        /**
         * @brief Stores new admin credentials (hashed with SHA-256 + salt).
         * @param username New admin username.
         * @param password Plaintext password (stored as a salted hash).
         */
        void setCredentials(const std::string &username, const std::string &password);

        /**
         * @brief Validates a username/password pair against stored credentials.
         * @param username Username to check.
         * @param password Plaintext password to check.
         * @return `true` if the credentials match.
         */
        bool checkCredentials(const std::string &username, const std::string &password);

        /**
         * @brief Looks up a raw token string and returns the matching Token.
         * @param authToken Raw token string from a cookie or Authorization header.
         * @return Matching Token if valid; NullToken() otherwise.
         */
        Token checkToken(const std::string &authToken);

        /*-------------------------------------------------------------------------------------------------
         *
         * Session Tokens
         *
         **/

        /**
         * @brief Issues a new time-limited session token.
         * @param seconds  Lifetime in seconds from now.
         * @param actions  List of permitted action names.
         * @return The newly created Token.
         */
        Token getSessionToken(long seconds, const std::vector<std::string> &actions);

        /**
         * @brief Revokes a session token by its raw value.
         * @param token Raw token string to remove.
         */
        void removeSessionToken(const std::string &token);

        /*-------------------------------------------------------------------------------------------------
         *
         * API Tokens
         *
         **/

        /**
         * @brief Creates or retrieves a named permanent API token.
         * @param name   Token identifier.
         * @param action List of permitted action names.
         * @return The API Token.
         */
        Token getApiToken(const std::string &name, const std::vector<std::string> &action);

        /**
         * @brief Permanently removes a named API token from storage.
         * @param name Name of the API token to delete.
         */
        void removeApiToken(const std::string &name);

        /**
         * @brief Returns all currently stored API tokens.
         * @return Vector of all API Token objects.
         */
        std::vector<Token> listApiTokens();
    };
}
