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

		std::string Encrypt(std::string Plain);
		std::string Decrypt(std::string Cipher);
		RSA::PublicKey GetPublicKey();
		RSA::PrivateKey GetPrivateKey();
		void SetPublicKey(RSA::PublicKey& NewPublicKey);
		void SetPrivateKey(RSA::PrivateKey& NewPrivateKey);
	};
}