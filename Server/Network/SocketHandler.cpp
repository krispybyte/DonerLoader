#include "SocketHandler.hpp"
#include "../Utilities/Utilities.hpp"

asio::awaitable<void> Network::Handle::Idle(Network::Socket& Socket)
{
	// Write
	{
		const json Json =
		{
			{ "Response", NULL }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}

asio::awaitable<void> Network::Handle::Initialize(Network::Socket& Socket, json& ReadJson)
{
	// Read
	{
		// Get the client's public key
		const std::string ClientPublicKey = Crypto::Base64::Decode(ReadJson["ClientKey"]);
		Socket.ClientPublicKey = Crypto::PEM::ImportKey(ClientPublicKey);

		std::cout << "[+] Exchanged keys!" << '\n';
	}

	// Write
	{
		// Setup server public key
		const std::string ServerPublicKey = Utilities::GetPublicKeyStr(Socket.ServerPrivateKey);

		// Convert aes key to a string then encrypt it using the client's
		// public rsa key which was read, and then encode it using base64
		std::string AesKey = std::string(reinterpret_cast<const char*>(Socket.AesKey.data()), Socket.AesKey.size());
		AesKey = Crypto::Base64::Encode(Crypto::Rsa::Encrypt(AesKey, Socket.ClientPublicKey));

		const json Json =
		{
			{ "ServerKey", ServerPublicKey },
			{ "AesKey", AesKey }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}

asio::awaitable<void> Network::Handle::Login(Network::Socket& Socket, json& ReadJson)
{
	// Read
	{
		const std::string DecryptedMessage = Utilities::DecryptMessage(ReadJson["Data"], ReadJson["AesIv"], Socket.AesKey);
		std::cout << "[+] Decrypted login message: " << DecryptedMessage.c_str() << '\n';
	}

	// Write
	{
		const auto EncryptionData = Utilities::EncryptMessage("Hello client!", Socket.AesKey);

		const std::string EncryptedMessage = std::get<0>(EncryptionData);
		const std::string AesIvStr = std::get<1>(EncryptionData);

		const json Json =
		{
			{ "Id", SocketIds::Login },
			{ "AesIv", AesIvStr },
			{ "Data", EncryptedMessage }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}