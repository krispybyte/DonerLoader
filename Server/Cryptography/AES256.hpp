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
        void Generate();
        std::string Encrypt(const std::string Plain);
        std::string Decrypt(const std::string Cipher);
        std::string GetKey();
        std::string GetIv();
    };
}