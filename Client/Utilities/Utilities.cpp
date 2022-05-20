#include "Utilities.hpp"

std::string Utilities::RandomString(const std::size_t Length)
{
    auto RandomCharacter = []() -> char
    {
        const char CharacterSet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        const std::size_t MaxIndex = (sizeof(CharacterSet) - 1);
        return CharacterSet[rand() % MaxIndex];
    };

    std::string String(Length, 0);
    std::generate_n(String.begin(), Length, RandomCharacter);
    return String;
}

std::tuple<std::string, std::string> Utilities::EncryptMessage(const std::string& Plain)
{
    // Generate initialization vector
    const SecByteBlock AesIv = Crypto::Aes256::GenerateIv();
    const std::string AesIvStr = std::string(reinterpret_cast<const char*>(AesIv.data()), AesIv.size());

    return
    {
        Crypto::Base64::Encode(Crypto::Aes256::Encrypt(Plain, Network::AesKey, AesIv)),
        Crypto::Base64::Encode(AesIvStr)
    };
}

std::string Utilities::DecryptMessage(const std::string& Cipher, const std::string& EncodedIv)
{
    // Decode iv string
    const std::string DecodedAesIvStr = Crypto::Base64::Decode(EncodedIv);

    // Convert iv into a SecByteBlock from string
    const SecByteBlock AesIv = SecByteBlock(reinterpret_cast<const byte*>(DecodedAesIvStr.data()), DecodedAesIvStr.size());

    // Decode cipher
    const std::string DecodedCipher = Crypto::Base64::Decode(Cipher);

    // Decrypt
    return Crypto::Aes256::Decrypt(DecodedCipher, Network::AesKey, AesIv);
}

std::string Utilities::GetPublicKeyStr(RSA::PrivateKey& PrivateKey)
{
    // Generate key based on private key passed.
    const RSA::PublicKey ClientPublicKey = Crypto::Rsa::GeneratePublic(PrivateKey);
    // Return key in PEM format as a string.
    return Crypto::Base64::Encode(Crypto::PEM::ExportKey(ClientPublicKey));
}