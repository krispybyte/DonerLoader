#include "RSA.hpp"

std::string Crypto::Rsa::Encrypt(std::string Plain)
{
	const RSAES_PKCS1v15_Encryptor Encryptor(this->PublicKey);
	
	std::string Cipher;
	StringSource StringSource(Plain, true, new PK_EncryptorFilter(Rng, Encryptor, new StringSink(Cipher)));
	return Cipher;
}

std::string Crypto::Rsa::Decrypt(std::string Cipher)
{
	const RSAES_PKCS1v15_Decryptor Decryptor(this->PrivateKey);

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

void Crypto::Rsa::SetPublicKey(RSA::PublicKey& NewPublicKey)
{
	this->PublicKey = NewPublicKey;
}

void Crypto::Rsa::SetPrivateKey(RSA::PrivateKey& NewPrivateKey)
{
	this->PrivateKey = NewPrivateKey;
}