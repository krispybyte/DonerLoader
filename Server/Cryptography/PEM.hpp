#pragma once
#include <Shared.hpp>

namespace Crypto
{
	namespace PEM
	{
		CryptoPP::RSA::PublicKey ImportKey(const std::string PublicKeyString);
		std::string ExportKey(const RSA::PrivateKey PrivateKey);
		std::string ExportKey(const RSA::PublicKey PublicKey);
	}
}