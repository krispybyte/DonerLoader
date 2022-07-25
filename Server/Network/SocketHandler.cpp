#include "SocketHandler.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Modules/Modules.hpp"
#include "../Database/Database.hpp"

asio::awaitable<void> Network::Handle::Idle(Network::Socket& Socket)
{
	// Write
	{
		const json Json =
		{
			{ "Null", 0 }
		};

		const std::string WriteData = Json.dump() + '\0';

		if (WriteData.size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[!] Prepared too large of a socket buffer! Terminating client connection." << '\n';
			Socket.Get().close();
			co_return;
		}

		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}

asio::awaitable<void> Network::Handle::Initialize(Network::Socket& Socket, const json& ReadJson)
{
	if (Socket.HasInitialized)
	{
		Socket.Get().close();
		co_return;
	}

	// Server cryptography data
	CryptoPP::RSA::PrivateKey ServerPrivateKey = Crypto::Rsa::GeneratePrivate();
	const std::string ServerPublicKey = Utilities::GetPublicKeyStr(ServerPrivateKey);
	Socket.AesKey = Crypto::Aes256::GenerateKey();

	// Client cryptography data
	CryptoPP::RSA::PublicKey ClientPublicKey;

	// Read
	{
		// Get the client's public key
		const std::string ClientPublicKeyData = Crypto::Hex::Decode(ReadJson["ClientKey"]);

		// Convert the client's public key into a PublicKey object
		ClientPublicKey = Crypto::PEM::ImportKey(ClientPublicKeyData);
	}

	// Write
	{
		// Convert aes key to a string then encrypt it using the client's
		// public rsa key which was read, and then encode it using hex
		std::string AesKey = std::string(reinterpret_cast<const char*>(Socket.AesKey.data()), Socket.AesKey.size());
		AesKey = Crypto::Hex::Encode(Crypto::Rsa::Encrypt(AesKey, ClientPublicKey));

		const json Json =
		{
			{ "ServerKey", ServerPublicKey },
			{ "AesKey", AesKey }
		};

		const std::string WriteData = Json.dump() + '\0';

		if (WriteData.size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[" << Socket.GetIpAddress().to_string().c_str() << "] Prepared too large of a socket buffer! Terminating client connection." << '\n';
			Socket.Get().close();
			co_return;
		}

		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	// Verify keys have exchanged for this connection
	Socket.HasInitialized = true;
	std::cout << "[" << Socket.GetIpAddress().to_string().c_str() << "] Has exchanged keys." << '\n';
}

asio::awaitable<void> Network::Handle::Login(Network::Socket& Socket, const json& ReadJson)
{
	if (!Socket.HasInitialized)
	{
		std::cout << "[" << Socket.GetIpAddress().to_string().c_str() << "] Hasn't initialized before trying to log in." << '\n';
		Socket.Get().close();
		co_return;
	}

	if (Socket.WrongHwid)
	{
		Socket.Get().close();
		co_return;
	}

	// Read
	const std::string DecryptedUsername = Utilities::DecryptMessage(ReadJson["Username"], ReadJson["AesIv"], Socket.AesKey);
	const std::string DecryptedPassword = Utilities::DecryptMessage(ReadJson["Password"], ReadJson["AesIv"], Socket.AesKey);
	const json Hwid = ReadJson["Hwid"];

	// Write
	{
		const LoginStatusIds LoginStatusId = Database::Login(DecryptedUsername, DecryptedPassword, Hwid);

		json Json =
		{
			{ "Status", LoginStatusId }
		};

		// Handle login status
		switch (LoginStatusId)
		{
			case LoginStatusIds::Success:
			{
				// Verify user has logged in
				Socket.HasLoggedIn = true;

				// Set the users name
				Socket.Username = DecryptedUsername;

				std::cout << "[" << Socket.Username << "] Has logged in." << '\n';
				break;
			}
			case LoginStatusIds::WrongCredentials:
			{
				std::cout << "[" << Socket.GetIpAddress().to_string().c_str() << "] Wrong login credentials." << '\n';

				Socket.LoginAttempts++;

				if (Socket.LoginAttempts == 4)
				{
					Socket.Get().close();
					const auto IpAddressPosition = std::find(ConnectionList.begin(), ConnectionList.end(), Socket.GetIpAddress());
					if (IpAddressPosition != ConnectionList.end())
					{
						ConnectionList.erase(IpAddressPosition);
					}
					co_return;
				}
				break;
			}
			case LoginStatusIds::WrongHwid:
			{
				std::cout << "[" << Socket.GetIpAddress().to_string().c_str() << "] HWID Mismatch." << '\n';

				Socket.WrongHwid = true;
				break;
			}
			default:
			{
				std::cout << "[" << Socket.GetIpAddress().to_string().c_str() << "] CRITICAL: Unknown login status id." << '\n';
				Socket.Get().close();
				co_return;
			}
		}

		const std::string WriteData = Json.dump() + '\0';

		if (WriteData.size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[" << Socket.Username << "] Prepared too large of a socket buffer! Terminating client connection." << '\n';
			Socket.Get().close();
			co_return;
		}

		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}
}

asio::awaitable<void> Network::Handle::Module(Network::Socket& Socket, const json& ReadJson)
{
	if (!Socket.HasLoggedIn)
	{
		std::cout << "[" << Socket.Username << "] Hasn't logged in before trying to stream the module." << '\n';
		Socket.Get().close();
		co_return;
	}

	// Read
	const int ModuleId = ReadJson["ModuleId"];

	// Validate module id
	if (ModuleId < ModuleIds::Test8MB || ModuleId > ModuleIds::Test1KB)
	{
		std::cout << "[" << Socket.Username << "] Sent an invalid module id." << '\n';
		Socket.Get().close();
		co_return;
	}

	// Verify the user isn't trying to load
	// the same module for the second time
	if (std::find(Socket.ModuleIdLoadList.begin(), Socket.ModuleIdLoadList.end(), ModuleId) != Socket.ModuleIdLoadList.end())
	{
		std::cout << "[" << Socket.Username << "] Has already loaded this module." << '\n';
		Socket.Get().close();
		co_return;
	}

	// Get module by index
	const Modules::Module& Module = Modules::List[ModuleId];

	constexpr std::size_t ModuleChunkSize = 80000; // 80 Kilobytes

	// Write
	{
		const std::string ModuleChunkData = std::string(Module.begin() + ModuleChunkSize * static_cast<std::size_t>(Socket.StreamChunkIndex),
														Module.begin() + ModuleChunkSize * static_cast<std::size_t>(Socket.StreamChunkIndex) + ModuleChunkSize);

		const auto EncryptionData = Utilities::EncryptMessage(ModuleChunkData, Socket.AesKey);

		const std::string EncryptedMessage = std::get<0>(EncryptionData);
		const std::string AesIv = std::get<1>(EncryptionData);

		const json Json =
		{
			{ "AesIv", AesIv },
			{ "Data", EncryptedMessage },
			{ "Size", Module.size() }
		};

		const std::string WriteData = Json.dump() + '\0';

		if (WriteData.size() > NETWORK_CHUNK_SIZE)
		{
			std::cout << "[" << Socket.Username << "] Prepared too large of a socket buffer!Terminating client connection." << '\n';
			Socket.Get().close();
			co_return;
		}

		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);

		// Increment after writing
		Socket.StreamChunkIndex++;

		// Check if we're at the last iteration, if we are, then we
		// push the module id onto the socket's module load list and return
		{
			const int ClientModuleSize = static_cast<int>(ModuleChunkData.size()) * Socket.StreamChunkIndex;

			if (ClientModuleSize >= Module.size())
			{
				Socket.ModuleIdLoadList.push_back(static_cast<Network::ModuleIds>(ModuleId));

				std::cout << "[" << Socket.Username << "] Modules streamed (by id): ";
				for (const int Id : Socket.ModuleIdLoadList)
				{
					std::cout << "#" << Id << ' ';
				}
				std::cout << '\n';

				// Verify user has streamed the module
				Socket.HasStreamedModule = true;

				co_return;
			}
		}
	}
}