#include "Network.hpp"
#include "../Cryptography/Base64.hpp"
#include "../Cryptography/PEM.hpp"

tcp::socket& Network::Socket::Get()
{
	return this->_Socket;
}

asio::ip::address Network::Socket::GetIpAddress()
{
	return this->_Socket.remote_endpoint().address();
}

asio::awaitable<void> Network::SocketHandler(tcp::socket TcpSocket)
{
	Network::Socket Socket(TcpSocket);

	const auto IpAddress = Socket.GetIpAddress();

	if (std::find(ConnectionList.begin(), ConnectionList.end(), IpAddress) != ConnectionList.end())
	{
		std::cout << "[-] " << IpAddress.to_string().c_str() << " is already connected!" << std::endl;
		co_return;
	}

	ConnectionList.push_back(IpAddress);

	std::cout << "[+] " << IpAddress.to_string().c_str() << " has connected." << std::endl;

	// Rsa setup
	Socket.ServerPrivate = Crypto::Rsa::GeneratePrivate();
	bool ExchangedPublicKeys = false;

	// Aes setup
	auto ServerAesKey = Crypto::Aes256::GenerateKey();
	auto ServerAesIv = Crypto::Aes256::GenerateKey();

	std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
	asio::mutable_buffer ReadBuffer(ReadBufferData.data(), ReadBufferData.size());

	while (true)
	{
		if (!Socket.Get().is_open())
		{
			std::cout << "[-] " << IpAddress.to_string().c_str() << " has disconnected." << std::endl;
			break;
		}

		try
		{
			const auto ByteCount = co_await Socket.Get().async_read_some(ReadBuffer, asio::use_awaitable);
			const auto JsonRead = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));
			const auto SocketId = static_cast<Network::SocketIds>(JsonRead["Id"]);

			json JsonWrite;

			switch (SocketId)
			{
				case SocketIds::Idle:
				{
					JsonWrite =
					{
						{ "Id", SocketIds::Idle }
					};

					break;
				}
				case SocketIds::Initialize:
				{
					// Receive the client's public
					if (!ExchangedPublicKeys)
					{
						Socket.ClientPublic = Crypto::PEM::ImportKey(Crypto::Base64::Decode(JsonRead["PublicKey"]));
						std::cout << "[+] Got the client's public key!" << std::endl;

						const auto ServerPublic = Crypto::Rsa::GeneratePublic(Socket.ServerPrivate);

						JsonWrite =
						{
							{ "Id", SocketIds::Initialize },
							{ "PublicKey", Crypto::Base64::Encode(Crypto::PEM::ExportKey(ServerPublic)) }
						};

						ExchangedPublicKeys = true;

						// Break to not continue onto the
						// aes key and iv exchange stage
						break;
					}

					// Receive the client's aes key and iv

					// Decode
					auto DecodedClientAesKey = Crypto::Base64::Decode(JsonRead["AesKey"]);
					auto DecodedClientAesIv = Crypto::Base64::Decode(JsonRead["AesIv"]);

					// Decrypt
					const auto DecryptedClientAesKey = Crypto::Rsa::Decrypt(DecodedClientAesKey, Socket.ServerPrivate);
					const auto DecryptedClientAesIv = Crypto::Rsa::Decrypt(DecodedClientAesIv, Socket.ServerPrivate);

					Socket.ClientAesKey = SecByteBlock((const byte*)DecryptedClientAesKey.data(), DecryptedClientAesKey.size());
					Socket.ClientAesIv = SecByteBlock((const byte*)DecryptedClientAesIv.data(), DecryptedClientAesIv.size());

					auto ServerAesKeyStr = std::string(reinterpret_cast<const char*>(ServerAesKey.data()), ServerAesKey.size());
					auto ServerAesIvStr = std::string(reinterpret_cast<const char*>(ServerAesIv.data()), ServerAesIv.size());

					printf("[+] Client AES key: %s", Crypto::Base64::Encode(DecryptedClientAesKey).c_str());
					printf("[+] Server AES key: %s\n", Crypto::Base64::Encode(ServerAesKeyStr).c_str());

					JsonWrite =
					{
						{ "Id", SocketIds::Initialize },
						{ "AesKey", Crypto::Base64::Encode(Crypto::Rsa::Encrypt(ServerAesKeyStr, Socket.ClientPublic)) },
						{ "AesIv", Crypto::Base64::Encode(Crypto::Rsa::Encrypt(ServerAesIvStr, Socket.ClientPublic)) }
					};

					break;
				}
				case SocketIds::Login:
				{
					JsonWrite =
					{
						{ "Id", SocketIds::Login },
						{ "Data", Crypto::Base64::Encode("I logged u in bruh :skull:") }
					};

					std::cout << "[+] Logging " << IpAddress.to_string().c_str() << " in." << std::endl;

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

			const auto WriteData = JsonWrite.dump() + '\0';
			co_await Socket.Get().async_write_some(asio::buffer(WriteData, WriteData.size()), asio::use_awaitable);
		}
		catch (std::exception& Ex)
		{
			std::cerr << "[" << IpAddress.to_string().c_str() << "] Exception: " << Ex.what() << std::endl;
			break;
		}
	}

	// Client will close socket at this point
	ConnectionList.erase(std::remove(ConnectionList.begin(), ConnectionList.end(), Socket.GetIpAddress()), ConnectionList.end());
}

asio::awaitable<void> Network::ConnectionHandler(tcp::acceptor& TcpAcceptor)
{
	std::cout << "[!] Spawning launch coroutine." << std::endl;

	while (true)
	{
		// Asynchronously wait for a socket and then accept
		// it's connection using our tcp acceptor.
		auto Socket = co_await TcpAcceptor.async_accept(asio::use_awaitable);
		co_spawn(Socket.get_executor(), Network::SocketHandler(std::move(Socket)), asio::detached);
	}
}

void Network::Create(asio::ip::port_type Port)
{
	// Create asio io context
	asio::io_context IoCtx(1);

	const auto Executor = IoCtx.get_executor();
	tcp::acceptor Acceptor(Executor, { tcp::v4(), Port });

	// Spawn launch coroutine
	co_spawn(IoCtx, Network::ConnectionHandler(Acceptor), asio::detached);

	// Run event processing loop
	IoCtx.run();
}