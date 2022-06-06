#pragma once
#include <Shared.hpp>

namespace Crypto::Base64
{
	using namespace CryptoPP;

	std::string Encode(std::string Decoded);
	std::string Decode(std::string Encoded);
}