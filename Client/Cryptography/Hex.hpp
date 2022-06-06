#pragma once
#include <Shared.hpp>

namespace Crypto::Hex
{
	using namespace CryptoPP;

	std::string Encode(std::string Decoded);
	std::string Decode(std::string Encoded);
}