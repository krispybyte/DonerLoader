#pragma once
#include <Shared.hpp>

namespace Crypto::Rsa
{
	using namespace CryptoPP;

	inline AutoSeededRandomPool Rng;
	RSA::PrivateKey GeneratePrivate(unsigned int Size = 4096);
	RSA::PublicKey GeneratePublic(RSA::PrivateKey& PrivateKey);
	std::string Encrypt(std::string& Plain, RSA::PublicKey& PublicKey);
	std::string Decrypt(std::string& Cipher, RSA::PrivateKey& PrivateKey);
}