/**
 * @file api_key_credential.hpp
 * @brief API key storage and verification for logscope-web (v2.2.2).
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "foundation/result.hpp"

namespace scope::web
{

/**
 * @brief Resolved API key material — disabled, legacy plaintext, or salted SHA-256 hash at rest.
 *
 * Stored hash format: {@code sha256:<salt_hex>:<digest_hex>} where digest = SHA256(salt || utf8(key)).
 */
class ApiKeyCredential
{
public:
    ApiKeyCredential() = default;

    [[nodiscard]] static ApiKeyCredential disabled();

    [[nodiscard]] static ApiKeyCredential fromPlaintext(std::string key);

    /** @brief Legacy plaintext from a properties file (emits a startup warning). */
    [[nodiscard]] static ApiKeyCredential fromPlaintextInConfig(std::string key);

    /**
     * @brief Parses {@code sha256:<salt_hex>:<digest_hex>} from config or env.
     */
    [[nodiscard]] static foundation::Result<ApiKeyCredential> fromStoredHash(const std::string& stored);

    /**
     * @brief Generates a salted hash string suitable for {@code web.api_key_hash}.
     */
    [[nodiscard]] static foundation::Result<std::string> hashPlaintextForStorage(const std::string& plaintext);

    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] bool isPlaintextInConfig() const noexcept;

    [[nodiscard]] bool verify(const std::string& presented) const;

private:
    enum class Mode
    {
        Disabled,
        Plaintext,
        Hashed
    };

    Mode m_mode = Mode::Disabled;
    std::string m_plaintext;
    std::vector<std::uint8_t> m_salt;
    std::vector<std::uint8_t> m_digest;
    bool m_plaintextInConfig = false;

    ApiKeyCredential(Mode mode, std::string plaintext, bool plaintextInConfig);
    ApiKeyCredential(std::vector<std::uint8_t> salt, std::vector<std::uint8_t> digest);
};

} // namespace scope::web
