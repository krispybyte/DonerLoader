#include "Base64.hpp"

std::string Crypto::Base64::Encode(std::string Decoded)
{
	std::string Encoded;

	Base64Encoder Encoder(nullptr, false, 72);
	Encoder.Put(reinterpret_cast<byte*>(Decoded.data()), Decoded.size());
	Encoder.MessageEnd();

	const word64 Size = Encoder.MaxRetrievable();

	if (Size && Size <= SIZE_MAX)
	{
		Encoded.resize(Size);
		Encoder.Get(reinterpret_cast<byte*>(&Encoded[0]), Encoded.size());
	}

	return Encoded;
}

std::string Crypto::Base64::Decode(std::string Encoded)
{
	std::string Decoded;

	Base64Decoder Decoder;
	Decoder.Put(reinterpret_cast<byte*>(Encoded.data()), Encoded.size());
	Decoder.MessageEnd();

	const word64 Size = Decoder.MaxRetrievable();

	if (Size && Size <= SIZE_MAX)
	{
		Decoded.resize(Size);
		Decoder.Get(reinterpret_cast<byte*>(&Decoded[0]), Decoded.size());
	}

	return Decoded;
}