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
		Rsa(unsigned int Size = 4096)
		{
			this->PrivateKey.GenerateRandomWithKeySize(Rng, Size);
			this->PublicKey = RSA::PublicKey(PrivateKey);
		}

		std::string Encrypt(std::string Plain, RSA::PublicKey& PublicKey);
		std::string Decrypt(std::string Cipher, RSA::PrivateKey& PrivateKey);
		RSA::PublicKey GetPublicKey();
		RSA::PrivateKey GetPrivateKey();
	};
}