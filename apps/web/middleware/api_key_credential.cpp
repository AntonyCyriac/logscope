/**
 * @file api_key_credential.cpp
 */

#include "api_key_credential.hpp"

#include <cctype>
#include <string_view>

#ifdef LOGSCOPE_WEB_API_KEY_HASHING
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif

namespace scope::web
{

namespace
{

constexpr char kHashPrefix[] = "sha256:";
constexpr std::size_t kSaltBytes = 16U;
constexpr std::size_t kDigestBytes = 32U;

bool constantTimeEqual(const std::vector<std::uint8_t>& left, const std::vector<std::uint8_t>& right)
{
    if (left.size() != right.size())
    {
        return false;
    }

    std::uint8_t difference = 0U;

    for (std::size_t index = 0; index < left.size(); ++index)
    {
        difference |= static_cast<std::uint8_t>(left[index] ^ right[index]);
    }

    return difference == 0U;
}

bool isHexDigit(const char character)
{
    return std::isxdigit(static_cast<unsigned char>(character)) != 0;
}

foundation::Result<std::vector<std::uint8_t>> decodeHex(const std::string& hex)
{
    if (hex.empty() || (hex.size() % 2U) != 0U)
    {
        return foundation::Result<std::vector<std::uint8_t>>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Invalid hex encoding for API key hash."));
    }

    std::vector<std::uint8_t> bytes;
    bytes.reserve(hex.size() / 2U);

    auto hexValue = [](const char character) -> int {
        if (character >= '0' && character <= '9')
        {
            return character - '0';
        }

        if (character >= 'a' && character <= 'f')
        {
            return 10 + character - 'a';
        }

        if (character >= 'A' && character <= 'F')
        {
            return 10 + character - 'A';
        }

        return -1;
    };

    for (std::size_t index = 0; index < hex.size(); index += 2U)
    {
        if (!isHexDigit(hex[index]) || !isHexDigit(hex[index + 1U]))
        {
            return foundation::Result<std::vector<std::uint8_t>>(
                foundation::Error(foundation::ErrorCode::InvalidArgument, "Invalid hex encoding for API key hash."));
        }

        const int high = hexValue(hex[index]);
        const int low = hexValue(hex[index + 1U]);
        bytes.push_back(static_cast<std::uint8_t>((high << 4) | low));
    }

    return foundation::Result<std::vector<std::uint8_t>>(std::move(bytes));
}

std::string encodeHex(const std::vector<std::uint8_t>& bytes)
{
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string hex;
    hex.reserve(bytes.size() * 2U);

    for (const std::uint8_t byte : bytes)
    {
        hex.push_back(kDigits[(byte >> 4) & 0x0FU]);
        hex.push_back(kDigits[byte & 0x0FU]);
    }

    return hex;
}

#ifdef LOGSCOPE_WEB_API_KEY_HASHING

foundation::Result<std::vector<std::uint8_t>> sha256Digest(const std::vector<std::uint8_t>& salt,
                                                           const std::string& plaintext)
{
    EVP_MD_CTX* context = EVP_MD_CTX_new();

    if (context == nullptr)
    {
        return foundation::Result<std::vector<std::uint8_t>>(
            foundation::Error(foundation::ErrorCode::Unknown, "Failed to allocate SHA-256 context."));
    }

    std::vector<std::uint8_t> digest(kDigestBytes);

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1)
    {
        EVP_MD_CTX_free(context);

        return foundation::Result<std::vector<std::uint8_t>>(
            foundation::Error(foundation::ErrorCode::Unknown, "Failed to initialize SHA-256."));
    }

    if (!salt.empty() && EVP_DigestUpdate(context, salt.data(), salt.size()) != 1)
    {
        EVP_MD_CTX_free(context);

        return foundation::Result<std::vector<std::uint8_t>>(
            foundation::Error(foundation::ErrorCode::Unknown, "Failed to update SHA-256 with salt."));
    }

    if (EVP_DigestUpdate(context, plaintext.data(), plaintext.size()) != 1)
    {
        EVP_MD_CTX_free(context);

        return foundation::Result<std::vector<std::uint8_t>>(
            foundation::Error(foundation::ErrorCode::Unknown, "Failed to update SHA-256 with API key."));
    }

    unsigned int digestLength = 0U;

    if (EVP_DigestFinal_ex(context, digest.data(), &digestLength) != 1 || digestLength != kDigestBytes)
    {
        EVP_MD_CTX_free(context);

        return foundation::Result<std::vector<std::uint8_t>>(
            foundation::Error(foundation::ErrorCode::Unknown, "Failed to finalize SHA-256 digest."));
    }

    EVP_MD_CTX_free(context);

    return foundation::Result<std::vector<std::uint8_t>>(std::move(digest));
}

foundation::Result<std::vector<std::uint8_t>> randomSalt()
{
    std::vector<std::uint8_t> salt(kSaltBytes);

    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1)
    {
        return foundation::Result<std::vector<std::uint8_t>>(
            foundation::Error(foundation::ErrorCode::Unknown, "Failed to generate API key salt."));
    }

    return foundation::Result<std::vector<std::uint8_t>>(std::move(salt));
}

#endif

bool startsWith(const std::string& value, const char* prefix)
{
    const std::string_view prefixView(prefix);
    return value.size() >= prefixView.size() && value.compare(0, prefixView.size(), prefix) == 0;
}

} // namespace

ApiKeyCredential ApiKeyCredential::disabled()
{
    return ApiKeyCredential{};
}

ApiKeyCredential ApiKeyCredential::fromPlaintext(std::string key)
{
    if (key.empty())
    {
        return disabled();
    }

    return ApiKeyCredential(Mode::Plaintext, std::move(key), false);
}

ApiKeyCredential ApiKeyCredential::fromPlaintextInConfig(std::string key)
{
    if (key.empty())
    {
        return disabled();
    }

    return ApiKeyCredential(Mode::Plaintext, std::move(key), true);
}

foundation::Result<ApiKeyCredential> ApiKeyCredential::fromStoredHash(const std::string& stored)
{
    if (!startsWith(stored, kHashPrefix))
    {
        return foundation::Result<ApiKeyCredential>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "API key hash must start with 'sha256:' (use hashPlaintextForStorage to generate)."));
    }

    const std::string payload = stored.substr(sizeof(kHashPrefix) - 1U);
    const std::size_t separator = payload.find(':');

    if (separator == std::string::npos)
    {
        return foundation::Result<ApiKeyCredential>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "API key hash must use format sha256:<salt_hex>:<digest_hex>."));
    }

    const auto salt = decodeHex(payload.substr(0, separator));

    if (!salt)
    {
        return foundation::Result<ApiKeyCredential>(salt.error());
    }

    const auto digest = decodeHex(payload.substr(separator + 1U));

    if (!digest)
    {
        return foundation::Result<ApiKeyCredential>(digest.error());
    }

    if (salt->size() != kSaltBytes || digest->size() != kDigestBytes)
    {
        return foundation::Result<ApiKeyCredential>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "API key hash has invalid salt or digest length."));
    }

#ifndef LOGSCOPE_WEB_API_KEY_HASHING
    return foundation::Result<ApiKeyCredential>(foundation::Error(
        foundation::ErrorCode::Unknown,
        "logscope-web was built without OpenSSL; API key hashing is unavailable."));
#else
    return foundation::Result<ApiKeyCredential>(ApiKeyCredential(std::move(*salt), std::move(*digest)));
#endif
}

foundation::Result<std::string> ApiKeyCredential::hashPlaintextForStorage(const std::string& plaintext)
{
    if (plaintext.empty())
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Cannot hash an empty API key."));
    }

#ifndef LOGSCOPE_WEB_API_KEY_HASHING
    return foundation::Result<std::string>(foundation::Error(
        foundation::ErrorCode::Unknown,
        "logscope-web was built without OpenSSL; API key hashing is unavailable."));
#else
    const auto salt = randomSalt();

    if (!salt)
    {
        return foundation::Result<std::string>(salt.error());
    }

    const auto digest = sha256Digest(*salt, plaintext);

    if (!digest)
    {
        return foundation::Result<std::string>(digest.error());
    }

    return foundation::Result<std::string>(std::string(kHashPrefix) + encodeHex(*salt) + ':' + encodeHex(*digest));
#endif
}

bool ApiKeyCredential::empty() const noexcept
{
    return m_mode == Mode::Disabled;
}

bool ApiKeyCredential::isPlaintextInConfig() const noexcept
{
    return m_plaintextInConfig;
}

bool ApiKeyCredential::verify(const std::string& presented) const
{
    if (m_mode == Mode::Disabled)
    {
        return true;
    }

    if (presented.empty())
    {
        return false;
    }

    if (m_mode == Mode::Plaintext)
    {
        if (presented.size() != m_plaintext.size())
        {
            return false;
        }

        return constantTimeEqual(std::vector<std::uint8_t>(presented.begin(), presented.end()),
                                 std::vector<std::uint8_t>(m_plaintext.begin(), m_plaintext.end()));
    }

#ifdef LOGSCOPE_WEB_API_KEY_HASHING
    const auto digest = sha256Digest(m_salt, presented);

    if (!digest)
    {
        return false;
    }

    return constantTimeEqual(*digest, m_digest);
#else
    return false;
#endif
}

ApiKeyCredential::ApiKeyCredential(const Mode mode, std::string plaintext, const bool plaintextInConfig)
    : m_mode(mode)
    , m_plaintext(std::move(plaintext))
    , m_plaintextInConfig(plaintextInConfig)
{
}

ApiKeyCredential::ApiKeyCredential(std::vector<std::uint8_t> salt, std::vector<std::uint8_t> digest)
    : m_mode(Mode::Hashed)
    , m_salt(std::move(salt))
    , m_digest(std::move(digest))
{
}

} // namespace scope::web
