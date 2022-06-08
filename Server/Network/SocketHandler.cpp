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
			{ "Null", NULL }
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
		std::cout << "[!] User has already initialized!" << '\n';
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
			std::cout << "[!] Prepared too large of a socket buffer! Terminating client connection." << '\n';
			Socket.Get().close();
			co_return;
		}

		co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
	}

	// Verify keys have exchanged for this connection
	Socket.HasInitialized = true;
	std::cout << "[+] Exchanged keys." << '\n';
}

asio::awaitable<void> Network::Handle::Login(Network::Socket& Socket, const json& ReadJson)
{
	if (!Socket.HasInitialized)
	{
		std::cout << "[!] User hasn't initialized before trying to log in!" << '\n';
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
		const LoginStatusIds LoginStatusId = Database::VerifyLogin(DecryptedUsername, DecryptedPassword, Hwid);

		// Handle login status
		switch (LoginStatusId)
		{
			case LoginStatusIds::Success:
			{
				// Verify user has logged in
				Socket.HasLoggedIn = true;
				break;
			}
			case LoginStatusIds::WrongCredentials:
			{
				Socket.LoginAttempts++;

				if (Socket.LoginAttempts == 4)
				{
					Socket.Get().close();
					co_return;
				}
				break;
			}
			case LoginStatusIds::WrongHwid:
			{
				Socket.WrongHwid = true;
				break;
			}
			default:
			{
				Socket.Get().close();
				co_return;
			}
		}

		const json Json =
		{
			{ "Status", LoginStatusId }
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

asio::awaitable<void> Network::Handle::Module(Network::Socket& Socket, const json& ReadJson)
{
	if (!Socket.HasLoggedIn)
	{
		std::cout << "[!] User hasn't logged in before trying to stream the module!" << '\n';
		Socket.Get().close();
		co_return;
	}

	// Read
	const int ModuleId = ReadJson["ModuleId"];

	// Validate module id
	if (ModuleId < ModuleIds::Test8MB || ModuleId > ModuleIds::Test1KB)
	{
		std::cout << "[!] Invalid module id received!" << '\n';
		Socket.Get().close();
		co_return;
	}

	// Verify the user isn't trying to load
	// the same module for the second time
	if (std::find(Socket.ModuleIdLoadList.begin(), Socket.ModuleIdLoadList.end(), ModuleId) != Socket.ModuleIdLoadList.end())
	{
		std::cout << "[!] User already loaded this module!" << '\n';
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
			std::cout << "[!] Prepared too large of a socket buffer! Terminating client connection." << '\n';
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

				std::cout << '\n' << "[!] Module has successfully streamed!" << '\n';
				std::cout		  << "[!] This user has streamed " << Socket.ModuleIdLoadList.size() << " module(s) so far!" << '\n';
				std::cout		  << "[!] Modules streamed by this user so far (by ID):" << '\n';

				for (const int Id : Socket.ModuleIdLoadList)
				{
					std::cout << "[+] Module #" << Id << '\n';
				}

				// Verify user has streamed the module
				Socket.HasStreamedModule = true;

				co_return;
			}
		}
	}
}