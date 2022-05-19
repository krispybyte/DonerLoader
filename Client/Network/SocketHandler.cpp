#include "SocketHandler.hpp"
#include "Network.hpp"
#include "../Cryptography/Rsa.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"

asio::awaitable<void> Network::Handle::Idle(tcp::socket& Socket)
{
	// Write
	{
		const json Json =
		{
			{ "Id", SocketIds::Idle }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	// Read
	{
		co_await Socket.async_read_some(ReadBuffer, asio::use_awaitable);
	}
}

asio::awaitable<void> Network::Handle::Initialization(tcp::socket& Socket, RSA::PrivateKey& PrivateKey)
{
	// Write client public key
	{
		// Setup client public key
		RSA::PublicKey ClientPublicKey = RSA::PublicKey(PrivateKey);
		const std::string ClientPublicKeyStr = Crypto::PEM::ExportKey(ClientPublicKey);

		const json Json =
		{
			{ "Id", SocketIds::Initialize },
			{ "ClientKey", Crypto::Base64::Encode(ClientPublicKeyStr) }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	// Read server public key and aes key (encrypted with the client's public key)
	{
		co_await Socket.async_read_some(ReadBuffer, asio::use_awaitable);
		const json Json = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));

		// Get server public key
		const std::string DecodedServerPublicKeyStr = Crypto::Base64::Decode(Json["ServerKey"]);
		Network::ServerPublicKey = Crypto::PEM::ImportKey(DecodedServerPublicKeyStr);

		// Get aes key
		std::string DecodedAesKeyStr = Crypto::Base64::Decode(Json["AesKey"]);
		const std::string DecryptedAesKeyStr = Crypto::Rsa::Decrypt(DecodedAesKeyStr, PrivateKey);

		Network::AesKey = SecByteBlock(reinterpret_cast<const byte*>(DecryptedAesKeyStr.data()), DecryptedAesKeyStr.size());

		std::cout << "[+] Got the server's public key and aes key!" << std::endl;
	}

	// Set state to next one
	Network::ClientState = ClientStates::IdleState;
}