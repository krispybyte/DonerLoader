#pragma once
#include <string>
#include <random>
#include "../Network/Network.hpp"

namespace Utilities
{
    std::string RandomString(const std::size_t Length);
    // Wrapper for aes encryption and hex encoding.
    std::tuple<std::string, std::string> EncryptMessage(const std::string& Plain, SecByteBlock& AesKey);
    // Wrapper for aes decryption and hex decoding.
    std::string DecryptMessage(const std::string& EncodedCipher, const std::string& EncodedIv, SecByteBlock& AesKey);
    // Wrapper for getting the string of a public key.
    // Generates the public key based on the private key
    // fed and returns it as a string in the PEM format.
    std::string GetPublicKeyStr(RSA::PrivateKey& PrivateKey);
    // File reading for module storing.
    bool ReadFile(const std::string_view FilePath, std::vector<uint8_t>& ReadFile);
}