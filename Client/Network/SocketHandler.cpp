#include "SocketHandler.hpp"
#include "Network.hpp"
#include "../Cryptography/Rsa.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"
#include "../Utilities/Utilities.hpp"

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
	// Write
	{
		// Setup client public key
		const std::string ClientPublicKey = Utilities::GetPublicKeyStr(PrivateKey);

		const json Json =
		{
			{ "Id", SocketIds::Initialize },
			{ "ClientKey", ClientPublicKey }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	// Read
	{
		co_await Socket.async_read_some(ReadBuffer, asio::use_awaitable);

		const json Json = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));

		// Decode and set the server's public key
		const std::string ServerPublicKey = Crypto::Base64::Decode(Json["ServerKey"]);
		Network::ServerPublicKey = Crypto::PEM::ImportKey(ServerPublicKey);

		// Decode and decrypt the aes key received from the server
		std::string AesKey = Crypto::Base64::Decode(Json["AesKey"]);
		AesKey = Crypto::Rsa::Decrypt(AesKey, PrivateKey);

		Network::AesKey = SecByteBlock(reinterpret_cast<const byte*>(AesKey.data()), AesKey.size());

		std::cout << "[+] Exchanged keys!" << '\n';
	}

	// Set state to next one
	Network::ClientState = ClientStates::LoginState;
}

asio::awaitable<void> Network::Handle::Login(tcp::socket& Socket)
{
	// Write
	{
		const auto EncryptionData = Utilities::EncryptMessage("Hello server!");

		const std::string EncryptedMessage = std::get<0>(EncryptionData);
		const std::string AesIvStr = std::get<1>(EncryptionData);
		
		const json Json =
		{
			{ "Id", SocketIds::Login },
			{ "AesIv", AesIvStr },
			{ "Data", EncryptedMessage }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	// Read
	{
		co_await Socket.async_read_some(ReadBuffer, asio::use_awaitable);

		const json Json = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));

		const std::string DecryptedMessage = Utilities::DecryptMessage(Json["Data"], Json["AesIv"]);
		std::cout << "[+] Decrypted login message: " << DecryptedMessage.c_str() << '\n';
	}

	// Set state to next one
	Network::ClientState = ClientStates::IdleState;
}