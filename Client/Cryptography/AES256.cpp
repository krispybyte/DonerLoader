#include "AES256.hpp"

std::string Crypto::Aes256Gcm::Encrypt(const std::string Plain)
{
    const SecByteBlock Key(reinterpret_cast<const unsigned char*>(this->KeyStr.data()), this->KeyStr.size());
    const SecByteBlock Iv(reinterpret_cast<const unsigned char*>(this->KeyStr.data()), this->KeyStr.size());

    GCM<AES>::Encryption Encryptor;
    Encryptor.SetKeyWithIV(Key, Key.size(), Iv, sizeof(Iv));

    std::string Cipher;
    const StringSource StringSource(Plain, true, new AuthenticatedEncryptionFilter(Encryptor, new StringSink(Cipher), false, this->TagSize));
    return Cipher;
}

std::string Crypto::Aes256Gcm::Decrypt(const std::string Cipher)
{
    const SecByteBlock Key(reinterpret_cast<const unsigned char*>(this->KeyStr.data()), this->KeyStr.size());
    const SecByteBlock Iv(reinterpret_cast<const unsigned char*>(this->KeyStr.data()), this->KeyStr.size());

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