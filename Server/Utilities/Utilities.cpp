#include "Utilities.hpp"
#include <string>
#include <random>
#include <cryptopp/files.h>

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

std::tuple<std::string, std::string> Utilities::EncryptMessage(const std::string& Plain, SecByteBlock& AesKey)
{
    // Generate initialization vector
    const SecByteBlock AesIv = Crypto::Aes256::GenerateIv();
    const std::string AesIvStr = std::string(reinterpret_cast<const char*>(AesIv.data()), AesIv.size());

    return
    {
        Crypto::Hex::Encode(Crypto::Aes256::Encrypt(Plain, AesKey, AesIv)),
        Crypto::Hex::Encode(AesIvStr)
    };
}

std::string Utilities::DecryptMessage(const std::string& EncodedCipher, const std::string& EncodedIv, SecByteBlock& AesKey)
{
    // Decode iv string
    const std::string DecodedAesIvStr = Crypto::Hex::Decode(EncodedIv);

    // Convert iv into a SecByteBlock from string
    const SecByteBlock AesIv = SecByteBlock(reinterpret_cast<const byte*>(DecodedAesIvStr.data()), DecodedAesIvStr.size());

    // Decode cipher
    const std::string DecodedCipher = Crypto::Hex::Decode(EncodedCipher);

    // Decrypt
    return Crypto::Aes256::Decrypt(DecodedCipher, AesKey, AesIv);
}

std::string Utilities::GetPublicKeyStr(RSA::PrivateKey& PrivateKey)
{
    // Generate key based on private key passed.
    const RSA::PublicKey ClientPublicKey = Crypto::Rsa::GeneratePublic(PrivateKey);

    // Return key in PEM format as a string.
    return Crypto::Hex::Encode(Crypto::PEM::ExportKey(ClientPublicKey));
}

bool Utilities::ReadFile(const std::string_view FilePath, std::vector<uint8_t>& ReadFile)
{
    std::ifstream FileStream(FilePath, std::ios::binary);

    if (!FileStream)
    {
        return false;
    }

    ReadFile.assign(std::istreambuf_iterator<char>(FileStream), std::istreambuf_iterator<char>());

    FileStream.close();
    return ReadFile.size();
}