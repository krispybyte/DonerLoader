#include "RSA.hpp"
#include <ThemidaSDK.h>

CryptoPP::RSA::PrivateKey Crypto::Rsa::GeneratePrivate(unsigned int Size)
{
	VM_START
	RSA::PrivateKey PrivateKey;
	PrivateKey.GenerateRandomWithKeySize(Rng, Size);
	VM_END
	return PrivateKey;
}

CryptoPP::RSA::PublicKey Crypto::Rsa::GeneratePublic(RSA::PrivateKey& PrivateKey)
{
	return RSA::PublicKey(PrivateKey);
}

std::string Crypto::Rsa::Encrypt(std::string& Plain, RSA::PublicKey& PublicKey)
{
	VM_START
	const RSAES_PKCS1v15_Encryptor Encryptor(PublicKey);

	std::string Cipher;
	const StringSource StringSource(Plain, true, new PK_EncryptorFilter(Rng, Encryptor, new StringSink(Cipher)));
	VM_END
	return Cipher;
}

std::string Crypto::Rsa::Decrypt(std::string& Cipher, RSA::PrivateKey& PrivateKey)
{
	VM_START
	const RSAES_PKCS1v15_Decryptor Decryptor(PrivateKey);

	std::string Plain;
	const StringSource StringSource(Cipher, true, new PK_DecryptorFilter(Rng, Decryptor, new StringSink(Plain)));
	VM_END
	return Plain;
}