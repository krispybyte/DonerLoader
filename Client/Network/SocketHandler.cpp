#include "SocketHandler.hpp"
#include "Network.hpp"
#include "../Cryptography/Rsa.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Gui/Gui.hpp"
#include <fstream>

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

asio::awaitable<void> Network::Handle::Initialize(tcp::socket& Socket)
{
	CryptoPP::RSA::PrivateKey ClientPrivateKey = Crypto::Rsa::GeneratePrivate();

	// Write
	{
		// Setup client public key
		const std::string ClientPublicKey = Utilities::GetPublicKeyStr(ClientPrivateKey);

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
		const std::string ServerPublicKey = Crypto::Hex::Decode(Json["ServerKey"]);
		Network::ServerPublicKey = Crypto::PEM::ImportKey(ServerPublicKey);

		// Decode and decrypt the aes key received from the server
		std::string AesKey = Crypto::Hex::Decode(Json["AesKey"]);
		AesKey = Crypto::Rsa::Decrypt(AesKey, ClientPrivateKey);

		Network::AesKey = CryptoPP::SecByteBlock(reinterpret_cast<const CryptoPP::byte*>(AesKey.data()), AesKey.size());

		std::cout << "[+] Exchanged keys." << '\n';
	}

	// Set state to idle so we can
	// wait until the user logs in
	Network::ClientState = ClientStates::IdleState;
}

asio::awaitable<void> Network::Handle::Login(tcp::socket& Socket)
{
	// Write
	{
		const std::string AesIv = Utilities::GenerateIv();

		const std::string EncryptedUsername = Utilities::EncryptMessage(Gui::Username, AesIv);
		const std::string EncryptedPassword = Utilities::EncryptMessage(Gui::Password, AesIv);
		
		const json Json =
		{
			{ "Id", SocketIds::Login },
			{ "AesIv", AesIv },
			{ "Username", EncryptedUsername },
			{ "Password", EncryptedPassword }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);

		LoginAttempts++;

		if (LoginAttempts >= 4)
		{
			MessageBoxA(nullptr, "Too many login attempts have been made.", "Error (335)", MB_ICONERROR | MB_OK);
			Gui::ShouldRun = false;
			Socket.close();
			co_return;
		}
	}

	// Read
	{
		co_await Socket.async_read_some(ReadBuffer, asio::use_awaitable);

		const json Json = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));

		SuccessfulLogin = Json["Success"];

		if (SuccessfulLogin)
		{
			std::cout << "[+] Successfully logged in!" << '\n';

			if (Gui::RememberMe)
			{
				// Remember me here
			}
		}
		else
		{
			MessageBoxA(nullptr, "Wrong username or password.", "Error (71)", MB_ICONERROR | MB_OK);
		}
	}

	// Set state to the next one
	Network::ClientState = ClientStates::IdleState;
}

asio::awaitable<void> Network::Handle::Module(tcp::socket& Socket)
{
	constexpr int ModuleId = Network::ModuleIds::Test8MB;

	// Write
	{
		const json Json =
		{
			{ "Id", SocketIds::Module },
			{ "ModuleId", ModuleId },
			{ "Index", Module::ChunkIndex }
		};

		const std::string WriteData = Json.dump() + '\0';
		co_await Socket.async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	// Read
	{
		co_await Socket.async_read_some(ReadBuffer, asio::use_awaitable);

		const json Json = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));

		const std::string DecryptedMessage = Utilities::DecryptMessage(Json["Data"], Json["AesIv"]);

		Module::Data.insert(Module::Data.end(), DecryptedMessage.begin(), DecryptedMessage.begin() + DecryptedMessage.size());

		const int ExpectedModuleSize = Json["Size"];
		if (Module::Data.size() >= ExpectedModuleSize)
		{
			Module::Data.resize(ExpectedModuleSize);

			std::cout << '\n' << "[!] Module has successfully streamed! Debug information:" << '\n';
			std::cout <<		 "[+] Module id: " << ModuleId << '\n';
			std::cout <<		 "[+] Took " << Module::ChunkIndex + 1 << " streams (3KB each)";

			// Set state to the next one
			Network::ClientState = ClientStates::IdleState;
			co_return;
		}
	}

	// Increment index to receive the next chunk in the next read
	Module::ChunkIndex++;
}