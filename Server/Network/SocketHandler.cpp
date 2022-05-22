#include "SocketHandler.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Modules/Test.hpp"

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
		const std::string ClientPublicKey = Crypto::Hex::Decode(ReadJson["ClientKey"]);
		Socket.ClientPublicKey = Crypto::PEM::ImportKey(ClientPublicKey);
	}

	// Write
	{
		// Setup server public key
		const std::string ServerPublicKey = Utilities::GetPublicKeyStr(Socket.ServerPrivateKey);

		// Convert aes key to a string then encrypt it using the client's
		// public rsa key which was read, and then encode it using base64
		std::string AesKey = std::string(reinterpret_cast<const char*>(Socket.AesKey.data()), Socket.AesKey.size());
		AesKey = Crypto::Hex::Encode(Crypto::Rsa::Encrypt(AesKey, Socket.ClientPublicKey));

		const json Json =
		{
			{ "ServerKey", ServerPublicKey },
			{ "AesKey", AesKey }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	std::cout << "[+] Exchanged keys." << '\n';
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
		const std::string AesIv = std::get<1>(EncryptionData);

		const json Json =
		{
			{ "AesIv", AesIv },
			{ "Data", EncryptedMessage }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}

asio::awaitable<void> Network::Handle::Module(Network::Socket& Socket, json& ReadJson)
{
	const int ModuleChunkIndex = ReadJson["Index"];
	constexpr int ModuleChunkSize = 3072; // 3 Kilobytes

	// Write
	{
		const std::string ModuleChunkData = std::string(Modules::Test.begin() + (ModuleChunkSize * ModuleChunkIndex),
														Modules::Test.begin() + (ModuleChunkSize * ModuleChunkIndex) + ModuleChunkSize);

		const auto EncryptionData = Utilities::EncryptMessage(ModuleChunkData, Socket.AesKey);

		const std::string EncryptedMessage = std::get<0>(EncryptionData);
		const std::string AesIv = std::get<1>(EncryptionData);

		const json Json =
		{
			{ "AesIv", AesIv },
			{ "Data", EncryptedMessage },
			{ "Size", Modules::Test.size() }
		};

		if (Json.dump().size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[!] Prepared too large of a socket! Terminating client connection." << '\n';
			Socket.Get().close();
		}

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}