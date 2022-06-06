#pragma once
#include <Shared.hpp>

namespace Crypto::Aes256
{
	using namespace CryptoPP;
	
	inline AutoSeededRandomPool Rng;
	constexpr auto TagSize = 12;
	SecByteBlock GenerateKey(std::size_t Size = 32);
	SecByteBlock GenerateIv(std::size_t Size = 32);
	std::string Encrypt(const std::string& Plain, const SecByteBlock& Key, const SecByteBlock& Iv);
	std::string Decrypt(const std::string& Cipher, const SecByteBlock& Key, const SecByteBlock& Iv);
}