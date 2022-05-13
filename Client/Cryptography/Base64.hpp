#pragma once
#include <Shared.hpp>

namespace Crypto::Base64
{
	std::string Encode(std::string Decoded);
	std::string Decode(std::string Encoded);
}