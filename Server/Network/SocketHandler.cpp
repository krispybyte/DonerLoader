#include "SocketHandler.hpp"

asio::awaitable<void> Network::Handle::Idle(Network::Socket& Socket)
{
	// Write
	{
		const json Json =
		{
			{ "Response", 1337 }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}

asio::awaitable<void> Network::Handle::Initialize(Network::Socket& Socket, json& ReadJson)
{
	// Read client public key
	{
		// Get client public key
		const std::string ClientPublicKeyStr = Crypto::Base64::Decode(ReadJson["ClientKey"]);
		Socket.ClientPublicKey = Crypto::PEM::ImportKey(ClientPublicKeyStr);

		std::cout << "[+] Got the client's public key!" << std::endl;
	}

	// Write server public key and aes key (encrypted with client's public key)
	{
		// Setup server public key
		RSA::PublicKey ServerPublicKey = RSA::PublicKey(Socket.ServerPrivateKey);
		const std::string ServerPublicKeyStr = Crypto::PEM::ExportKey(ServerPublicKey);

		// Setup aes key
		std::string AesKeyStr = std::string(reinterpret_cast<const char*>(Socket.AesKey.data()), Socket.AesKey.size());
		const std::string EncryptedAesKeyStr = Crypto::Rsa::Encrypt(AesKeyStr, Socket.ClientPublicKey);

		const json Json =
		{
			{ "ServerKey", Crypto::Base64::Encode(ServerPublicKeyStr) },
			{ "AesKey", Crypto::Base64::Encode(EncryptedAesKeyStr) }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}