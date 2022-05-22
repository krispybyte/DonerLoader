#pragma once
#include <Shared.hpp>

namespace Crypto::Hex
{
	std::string Encode(std::string Decoded);
	std::string Decode(std::string Encoded);
}