﻿#include "PEM.hpp"

CryptoPP::RSA::PublicKey Crypto::PEM::ImportKey(const std::string PublicKeyString)
{
	RSA::PublicKey PublicKey;

	try
	{
		StringSource Source(PublicKeyString, true);
		PEM_Load(Source, PublicKey);
	}
	catch (const Exception& Ex)
	{
		std::cerr << Ex.what() << std::endl;
	}

	return PublicKey;
}

std::string Crypto::PEM::ExportKey(const RSA::PrivateKey PrivateKey)
{
	ByteQueue Queue;
	PEM_Save(Queue, PrivateKey);

	std::string Result;
	StringSink Sink(Result);

	Queue.CopyTo(Sink);
	Sink.MessageEnd();

	return Result;
}

std::string Crypto::PEM::ExportKey(const RSA::PublicKey PublicKey)
{
	ByteQueue Queue;
	PEM_Save(Queue, PublicKey);

	std::string Result;
	StringSink Sink(Result);

	Queue.CopyTo(Sink);
	Sink.MessageEnd();

	return Result;
}