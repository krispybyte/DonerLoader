#include "AES256.hpp"

void Crypto::Aes256Gcm::Generate()
{
    SecByteBlock Key(AES::MAX_KEYLENGTH);
    Rng.GenerateBlock(Key, Key.size());

    SecByteBlock Iv(AES::BLOCKSIZE);
    Rng.GenerateBlock(Iv, Iv.size());

    this->KeyStr = std::string(reinterpret_cast<const char*>(Key.data()), Key.size());
    this->IvStr = std::string(reinterpret_cast<const char*>(Iv.data()), Iv.size());
}

std::string Crypto::Aes256Gcm::Encrypt(const std::string& Plain, const std::string& KeyStr, const std::string& IvStr)
{
    const SecByteBlock Key(reinterpret_cast<const unsigned char*>(KeyStr.data()), KeyStr.size());
    const SecByteBlock Iv(reinterpret_cast<const unsigned char*>(IvStr.data()), IvStr.size());

    GCM<AES>::Encryption Encryptor;
    Encryptor.SetKeyWithIV(Key, Key.size(), Iv, sizeof(Iv));

    std::string Cipher;
    const StringSource StringSource(Plain, true, new AuthenticatedEncryptionFilter(Encryptor, new StringSink(Cipher), false, this->TagSize));
    return Cipher;
}

std::string Crypto::Aes256Gcm::Decrypt(const std::string& Cipher, const std::string& KeyStr, const std::string& IvStr)
{
    const SecByteBlock Key(reinterpret_cast<const unsigned char*>(KeyStr.data()), KeyStr.size());
    const SecByteBlock Iv(reinterpret_cast<const unsigned char*>(IvStr.data()), IvStr.size());

    GCM<AES>::Decryption Decryptor;
    Decryptor.SetKeyWithIV(Key, Key.size(), Iv, sizeof(Iv));

    std::string Plain;
    const StringSource StringSource(Cipher, true, new AuthenticatedDecryptionFilter(Decryptor, new StringSink(Plain), false, this->TagSize));
    return Plain;
}

std::string Crypto::Aes256Gcm::GetKey()
{
    return this->KeyStr;
}

std::string Crypto::Aes256Gcm::GetIv()
{
    return this->IvStr;
}