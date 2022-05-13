#include "RSA.hpp"
#include <cryptopp/gcm.h>

std::string Crypto::Rsa::Encrypt(std::string Plain, RSA::PublicKey& PublicKey)
{
	const RSAES_PKCS1v15_Encryptor Encryptor(PublicKey);
	
	std::string Cipher;
	StringSource StringSource(Plain, true, new PK_EncryptorFilter(Rng, Encryptor, new StringSink(Cipher)));
	return Cipher;
}

std::string Crypto::Rsa::Decrypt(std::string Cipher, RSA::PrivateKey& PrivateKey)
{
	const RSAES_PKCS1v15_Decryptor Decryptor(PrivateKey);

	std::string Plain;
	StringSource StringSource(Cipher, true, new PK_DecryptorFilter(Rng, Decryptor, new StringSink(Plain)));
	return Plain;
}

RSA::PublicKey Crypto::Rsa::GetPublicKey()
{
	return this->PublicKey;
}

RSA::PrivateKey Crypto::Rsa::GetPrivateKey()
{
	return this->PrivateKey;
}