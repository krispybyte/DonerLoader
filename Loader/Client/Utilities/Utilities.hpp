#pragma once
#include <string>
#include <random>
#include "../Network/Network.hpp"

namespace Utilities
{
    std::string RandomString(const std::size_t Length);
    // Wrapper for aes encryption and hex encoding.
    std::tuple<std::string, std::string> EncryptMessage(const std::string& Plain);
    // Wrapper for aes encryption and hex encoding
    // but without the generation of a new iv.
    std::string EncryptMessage(const std::string& Plain, const std::string& AesIvStr);
    // Wrapper for aes decryption and hex decoding.
    std::string DecryptMessage(const std::string& Cipher, const std::string& EncodedIv);
    // Wrapper for getting the string of a public key.
    // Generates the public key based on the private key
    // fed and returns it as a string in the PEM format.
    std::string GetPublicKeyStr(CryptoPP::RSA::PrivateKey& PrivateKey);
    // Generates custom initialization vector.
    std::string GenerateIv();
    // Generate hardware id.
    json GenerateHardwareId();
}