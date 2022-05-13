#pragma once
#include <Shared.hpp>

namespace Crypto
{
    class Aes256Gcm
    {
    private:
        AutoSeededRandomPool Rng;
        std::string KeyStr;
        std::string IvStr;
        const int TagSize = 12;
    public:
        Aes256Gcm()
        {
            SecByteBlock Key(AES::MAX_KEYLENGTH);
            Rng.GenerateBlock(Key, Key.size());

            SecByteBlock Iv(AES::BLOCKSIZE);
            Rng.GenerateBlock(Iv, Iv.size());

            this->KeyStr = std::string(reinterpret_cast<const char*>(Key.data()), Key.size());
            this->IvStr = std::string(reinterpret_cast<const char*>(Iv.data()), Iv.size());
        }

        std::string Encrypt(const std::string Plain);
        std::string Decrypt(const std::string Cipher);
        std::string GetKey();
        std::string GetIv();
    };
}