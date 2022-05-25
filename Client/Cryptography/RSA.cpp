#include "RSA.hpp"

CryptoPP::RSA::PrivateKey Crypto::Rsa::GeneratePrivate(unsigned int Size)
{
	RSA::PrivateKey PrivateKey;
	PrivateKey.GenerateRandomWithKeySize(Rng, Size);
	return PrivateKey;
}

CryptoPP::RSA::PublicKey Crypto::Rsa::GeneratePublic(RSA::PrivateKey& PrivateKey)
{
	return RSA::PublicKey(PrivateKey);
}

std::string Crypto::Rsa::Encrypt(std::string& Plain, RSA::PublicKey& PublicKey)
{
	const RSAES_PKCS1v15_Encryptor Encryptor(PublicKey);

	std::string Cipher;
	const StringSource StringSource(Plain, true, new PK_EncryptorFilter(Rng, Encryptor, new StringSink(Cipher)));
	return Cipher;
}

std::string Crypto::Rsa::Decrypt(std::string& Cipher, RSA::PrivateKey& PrivateKey)
{
	const RSAES_PKCS1v15_Decryptor Decryptor(PrivateKey);

	std::string Plain;
	const StringSource StringSource(Cipher, true, new PK_DecryptorFilter(Rng, Decryptor, new StringSink(Plain)));
	return Plain;
}