#pragma once
#include <Shared.hpp>

namespace Crypto
{
	class Rsa
	{
	private:
		AutoSeededRandomPool Rng;
		RSA::PublicKey PublicKey;
		RSA::PrivateKey PrivateKey;
	public:
		void Generate(unsigned int Size = 4096);
		std::string Encrypt(std::string Plain, RSA::PublicKey& PublicKey);
		std::string Decrypt(std::string Cipher, RSA::PrivateKey& PrivateKey);
		RSA::PublicKey GetPublicKey();
		RSA::PrivateKey GetPrivateKey();
	};
}