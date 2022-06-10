#include "SocketHandler.hpp"
#include "Network.hpp"
#include "../Cryptography/Rsa.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"
#include "../Gui/Gui.hpp"
#include "../Utilities/Utilities.hpp"
#include <fstream>

namespace Network::Handle
{
	std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
	asio::mutable_buffer ReadBuffer(ReadBufferData.data(), ReadBufferData.size());
}

namespace Client
{
	std::vector<std::uint8_t> ModuleData;
	const json HardwareId = Utilities::GenerateHardwareId();
	bool HasConnected = false;
}

asio::awaitable<void> Network::Handle::Idle(tcp::socket& Socket)
{
	// Write
	{
		const json Json =
		{
			{ "Id", SocketIds::Idle }
		};

		const std::string WriteData = Json.dump() + '\0';

		if (WriteData.size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[!] Prepared too large of a socket buffer! Terminating client connection." << '\n';
			Socket.close();
			co_return;
		}

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

		if (WriteData.size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[!] Prepared too large of a socket buffer! Terminating client connection." << '\n';
			Socket.close();
			co_return;
		}

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
	}

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
			{ "Password", EncryptedPassword },
			{ "Hwid", Client::HardwareId }
		};

		const std::string WriteData = Json.dump() + '\0';

		if (WriteData.size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[!] Prepared too large of a socket buffer! Terminating client connection." << '\n';
			Socket.close();
			co_return;
		}

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

		LoginStatus = Json["Status"];

		// Handle login status
		switch (LoginStatus)
		{
			case LoginStatusIds::Success:
			{
				std::cout << "[+] Successfully logged in!" << '\n';
				break;
			}
			case LoginStatusIds::WrongCredentials:
			{
				MessageBoxA(nullptr, "Wrong username or password.", "Error (71)", MB_ICONERROR | MB_OK);
				break;
			}
			case LoginStatusIds::WrongHwid:
			{
				MessageBoxA(nullptr, "Hardware ID Mismatch.", "Error (102)", MB_ICONERROR | MB_OK);
				Socket.close();
				ExitProcess(102);
			}
			default:
			{
				Socket.close();
				co_return;
			}
		}
	}

	// Set state to the next one
	Network::ClientState = ClientStates::IdleState;
}

int Times = 0;
constexpr int ModuleId = Network::ModuleIds::Test8MB;

asio::awaitable<void> Network::Handle::Module(tcp::socket& Socket)
{
	// Write
	{
		const json Json =
		{
			{ "Id", SocketIds::Module },
			{ "ModuleId", Network::ModuleIds::Test8MB }
		};

		const std::string WriteData = Json.dump() + '\0';

		if (WriteData.size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[!] Prepared too large of a socket buffer! Terminating client connection." << '\n';
			Socket.close();
			co_return;
		}

		co_await Socket.async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	// Read
	{
		co_await Socket.async_read_some(ReadBuffer, asio::use_awaitable);

		Times++;
		std::cout << "Read #" << Times << std::endl;

		const json Json = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));

		const std::string DecryptedMessage = Utilities::DecryptMessage(Json["Data"], Json["AesIv"]);

		Client::ModuleData.insert(Client::ModuleData.end(), DecryptedMessage.begin(), DecryptedMessage.begin() + DecryptedMessage.size());

		const int ExpectedModuleSize = Json["Size"];
		if (Client::ModuleData.size() >= ExpectedModuleSize)
		{
			Client::ModuleData.resize(ExpectedModuleSize);

			std::cout << "[!] Module #" << ModuleId << " has successfully streamed!" << '\n';

			// Set state to the next one
			Network::ClientState = ClientStates::IdleState;
			co_return;
		}
	}
}