#include "PEM.hpp"
#include <ThemidaSDK.h>

CryptoPP::RSA::PublicKey Crypto::PEM::ImportKey(const std::string PublicKeyString)
{
	VM_START
	RSA::PublicKey PublicKey;

	StringSource Source(PublicKeyString, true);
	PEM_Load(Source, PublicKey);
	VM_END
	return PublicKey;
}

std::string Crypto::PEM::ExportKey(const RSA::PrivateKey PrivateKey)
{
	VM_START
	ByteQueue Queue;
	PEM_Save(Queue, PrivateKey);

	std::string Result;
	StringSink Sink(Result);

	Queue.CopyTo(Sink);
	Sink.MessageEnd();
	VM_END
	return Result;
}

std::string Crypto::PEM::ExportKey(const RSA::PublicKey PublicKey)
{
	VM_START
	ByteQueue Queue;
	PEM_Save(Queue, PublicKey);

	std::string Result;
	StringSink Sink(Result);

	Queue.CopyTo(Sink);
	Sink.MessageEnd();
	VM_END
	return Result;
}