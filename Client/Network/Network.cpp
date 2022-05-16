#include "Network.hpp"

asio::awaitable<void> Network::SocketHandler(tcp::socket Socket)
{
	std::cout << "[+] Connected." << std::endl;

	// Rsa setup
	auto ClientPrivate = Crypto::Rsa::GeneratePrivate();
	RSA::PublicKey ServerPublic;
	bool SentPublicKey = false;
	bool ReceivedPublicKey = false;

	// Aes setup
	auto ClientAesKey = Crypto::Aes256::GenerateKey();
	auto ClientAesIv = Crypto::Aes256::GenerateKey();
	SecByteBlock ServerAesKey;
	SecByteBlock ServerAesIv;

	std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
	asio::mutable_buffer ReadBuffer(ReadBufferData.data(), ReadBufferData.size());

	while (true)
	{
		if (!Socket.is_open())
		{
			std::cout << "[-] Disconnected." << std::endl;
			break;
		}

		try
		{
			json JsonWrite;

			switch (ClientState)
			{
				case ClientStates::IdleState:
				{
					JsonWrite =
					{
						{ "Id", SocketIds::Idle },
					};

					break;
				}
				case ClientStates::InitializeState:
				{
					// Send the client's public key
					if (!SentPublicKey)
					{
						const auto ClientPublic = Crypto::Rsa::GeneratePublic(ClientPrivate);

						JsonWrite =
						{
							{ "Id", SocketIds::Initialize },
							{ "PublicKey", Crypto::Base64::Encode(Crypto::PEM::ExportKey(ClientPublic)) }
						};

						SentPublicKey = true;

						// Break so we don't continue to the
						// Aes key exchange stage, that will
						// happen after the client has received
						// the server's public key.
						break;
					}

					// Send the client's aes key and iv,
					// encrypted using the server's
					// public rsa key.

					auto ClientAesKeyStr = std::string(reinterpret_cast<const char*>(ClientAesKey.data()), ClientAesKey.size());
					auto ClientAesIvStr = std::string(reinterpret_cast<const char*>(ClientAesIv.data()), ClientAesIv.size());

					printf("[+] Client AES key: %s", Crypto::Base64::Encode(ClientAesKeyStr).c_str());

					JsonWrite =
					{
						{ "Id", SocketIds::Initialize },
						{ "AesKey", Crypto::Base64::Encode(Crypto::Rsa::Encrypt(ClientAesKeyStr, ServerPublic))},
						{ "AesIv", Crypto::Base64::Encode(Crypto::Rsa::Encrypt(ClientAesIvStr, ServerPublic)) }
					};

					break;
				}
				case ClientStates::LoginState:
				{
					JsonWrite =
					{
						{ "Id", SocketIds::Login },
						{ "Data", Crypto::Base64::Encode("Log me in bruh :fire:") }
					};

					break;
				}
				case ClientStates::HwidState:
				case ClientStates::ModuleState:
				{
					std::cout << "[!] Unimplemented." << std::endl;
					break;
				}
				default:
				{
					std::cout << "[!] Invalid client state." << std::endl;
					break;
				}
			}

			const auto WriteData = JsonWrite.dump() + '\0';
			co_await Socket.async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);

			const auto ByteCount = co_await Socket.async_read_some(ReadBuffer, asio::use_awaitable);
			const auto JsonRead = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));
			const auto SocketId = static_cast<Network::SocketIds>(JsonRead["Id"]);

			switch (SocketId)
			{
				case SocketIds::Idle:
				{
					// Await things such as a login event,
					// injection event etc to change this state
					break;
				}
				case SocketIds::Initialize:
				{
					// Receive the server's public key
					if (!ReceivedPublicKey)
					{
						ServerPublic = Crypto::PEM::ImportKey(Crypto::Base64::Decode(JsonRead["PublicKey"]));
						std::cout << "[+] Got the server's public key!" << std::endl;
						ReceivedPublicKey = true;

						// Break to not continue onto the
						// aes key and iv exchange stage
						break;
					}

					// Receive the server's aes key and iv

					// Decode
					auto DecodedServerAesKey = Crypto::Base64::Decode(JsonRead["AesKey"]);
					auto DecodedServerAesIv = Crypto::Base64::Decode(JsonRead["AesIv"]);

					// Decrypt
					const auto DecryptedServerAesKey = Crypto::Rsa::Decrypt(DecodedServerAesKey, ClientPrivate);
					const auto DecryptedServerAesIv = Crypto::Rsa::Decrypt(DecodedServerAesIv, ClientPrivate);

					ServerAesKey = SecByteBlock((const byte*)DecryptedServerAesKey.data(), DecryptedServerAesKey.size());
					ServerAesIv = SecByteBlock((const byte*)DecryptedServerAesIv.data(), DecryptedServerAesIv.size());

					printf("[+] Server AES key: %s\n", Crypto::Base64::Encode(DecryptedServerAesKey).c_str());

					ClientState = ClientStates::LoginState;
					break;
				}
				case SocketIds::Login:
				{
					std::cout << "[+] Server logged me in." << std::endl;

					ClientState = ClientStates::IdleState;
					break;
				}
				case SocketIds::Hwid:
				case SocketIds::Module:
				{
					std::cout << "[!] Unimplemented." << std::endl;
					break;
				}
				default:
				{
					std::cout << "[!] Invalid socket id." << std::endl;
					break;
				}
			}
		}

		catch (std::exception& Ex)
		{
			std::cout << "[!] Exception: " << Ex.what() << std::endl;
			MessageBoxA(nullptr, "The server closed the connection.", "Error (94)", MB_ICONERROR | MB_OK);
			ExitProcess(94);
			break;
		}
	}
}

asio::awaitable<void> Network::Connect()
{
	std::cout << "[!] Spawning launch coroutine." << std::endl;

	const auto Executor = asio::get_associated_executor(asio::use_awaitable);
	auto Socket = tcp::socket(Executor);
	auto Resolver = tcp::resolver(Executor);
	const auto Query = tcp::resolver::query(NETWORK_IP_AND_PORT_STR);

	const auto Endpoint = tcp::resolver::iterator{ co_await Resolver.async_resolve(Query, asio::use_awaitable) };

	co_await Socket.async_connect(*Endpoint, asio::use_awaitable);
	co_await co_spawn(Executor, Network::SocketHandler(std::move(Socket)), asio::use_awaitable);
}