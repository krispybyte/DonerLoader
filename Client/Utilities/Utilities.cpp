#include "Utilities.hpp"
#include "../HardwareId/HardwareId.hpp"

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
    const CryptoPP::SecByteBlock AesIv = Crypto::Aes256::GenerateIv();
    const std::string AesIvStr = std::string(reinterpret_cast<const char*>(AesIv.data()), AesIv.size());

    return
    {
        Crypto::Hex::Encode(Crypto::Aes256::Encrypt(Plain, Network::AesKey, AesIv)),
        Crypto::Hex::Encode(AesIvStr)
    };
}

std::string Utilities::EncryptMessage(const std::string& Plain, const std::string& AesIvStr)
{
    const std::string DecodedAesIvStr = Crypto::Hex::Decode(AesIvStr);
    const CryptoPP::SecByteBlock AesIv = CryptoPP::SecByteBlock((const CryptoPP::byte*)DecodedAesIvStr.data(), DecodedAesIvStr.size());

    return Crypto::Hex::Encode(Crypto::Aes256::Encrypt(Plain, Network::AesKey, AesIv));
}

std::string Utilities::DecryptMessage(const std::string& Cipher, const std::string& EncodedIv)
{
    // Decode iv string
    const std::string DecodedAesIvStr = Crypto::Hex::Decode(EncodedIv);

    // Convert iv into a SecByteBlock from string
    const CryptoPP::SecByteBlock AesIv = CryptoPP::SecByteBlock(reinterpret_cast<const CryptoPP::byte*>(DecodedAesIvStr.data()), DecodedAesIvStr.size());

    // Decode cipher
    const std::string DecodedCipher = Crypto::Hex::Decode(Cipher);

    // Decrypt
    return Crypto::Aes256::Decrypt(DecodedCipher, Network::AesKey, AesIv);
}

std::string Utilities::GetPublicKeyStr(CryptoPP::RSA::PrivateKey& PrivateKey)
{
    // Generate key based on private key passed.
    const CryptoPP::RSA::PublicKey ClientPublicKey = Crypto::Rsa::GeneratePublic(PrivateKey);

    // Return key in PEM format as a string.
    return Crypto::Hex::Encode(Crypto::PEM::ExportKey(ClientPublicKey));
}

std::string Utilities::GenerateIv()
{
    // Generate initialization vector
    const CryptoPP::SecByteBlock AesIv = Crypto::Aes256::GenerateIv();
    const std::string AesIvStr = std::string(reinterpret_cast<const char*>(AesIv.data()), AesIv.size());
    return Crypto::Hex::Encode(AesIvStr);
}

json Utilities::GenerateHardwareId()
{
    const json HardwareId =
    {
        { "Cpu",
            { 
                { "Name", HardwareId::Cpu::Name().c_str() },                    // string
                { "Id", HardwareId::Cpu::Id().c_str() },                        // string
                { "Cores", HardwareId::Cpu::CoreCount() }                       // int
            }                                                                   
        },                                                                      

        { "Gpu", HardwareId::Gpu::Name().c_str() },                             // string

        { "DiskSerial", HardwareId::Disk::Serial().c_str() },                   // string

        { "Ram",                                                     
            {                                                        
                { "Serial", HardwareId::Ram::Serial().c_str() },                // string
                { "Manufacturer", HardwareId::Ram::Manufacturer().c_str() }     // string
            }
        },
    };

    return HardwareId;
}