#include "AES256.hpp"

CryptoPP::SecByteBlock Crypto::Aes256::GenerateKey(std::size_t Size)
{
    SecByteBlock Key(Size);
    Rng.GenerateBlock(Key, Key.size());
    return Key;
}

CryptoPP::SecByteBlock Crypto::Aes256::GenerateIv(std::size_t Size)
{
    SecByteBlock Iv(Size);
    Rng.GenerateBlock(Iv, Iv.size());
    return Iv;
}

std::string Crypto::Aes256::Encrypt(const std::string& Plain, const SecByteBlock& Key, const SecByteBlock& Iv)
{
    GCM<AES>::Encryption Encryptor;
    Encryptor.SetKeyWithIV(Key, Key.size(), Iv, sizeof(Iv));

    std::string Cipher;
    const StringSource StringSource(Plain, true, new AuthenticatedEncryptionFilter(Encryptor, new StringSink(Cipher), false, Aes256::TagSize));
    return Cipher;
}

std::string Crypto::Aes256::Decrypt(const std::string& Cipher, const SecByteBlock& Key, const SecByteBlock& Iv)
{
    GCM<AES>::Decryption Decryptor;
    Decryptor.SetKeyWithIV(Key, Key.size(), Iv, sizeof(Iv));

    std::string Plain;
    const StringSource StringSource(Cipher, true, new AuthenticatedDecryptionFilter(Decryptor, new StringSink(Plain), false, Aes256::TagSize));
    return Plain;
}