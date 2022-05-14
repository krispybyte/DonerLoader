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

	Socket.Rsa.Generate();
	Socket.Aes.Generate();

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
			// Clear read data buffer
			memset(&ReadBufferData, NULL, NETWORK_CHUNK_SIZE);
			
			const auto ByteCount = co_await Socket.Get().async_read_some(ReadBuffer, asio::use_awaitable);
			const auto JsonRead = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));
			const auto SocketId = static_cast<Network::SocketIds>(JsonRead["Id"]);

			json JsonWrite;

			switch (SocketId)
			{
				case SocketIds::Initialize:
				{
					const auto ClientPublicKeyStr = Crypto::Base64::Decode(JsonRead["Data"]);
					auto ClientPublicKey = Crypto::PEM::ImportKey(ClientPublicKeyStr);
					Socket.Rsa.SetPublicKey(ClientPublicKey);

					JsonWrite =
					{
						{ "Id", SocketIds::Initialize },
						{ "Data", Crypto::Base64::Encode(Crypto::PEM::ExportKey(Socket.Rsa.GetPublicKey())) }
					};

					std::cout << "[+] Got the client's public key!" << std::endl;

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

			const auto JsonWriteDump = JsonWrite.dump();
			co_await Socket.Get().async_write_some(asio::const_buffer(JsonWriteDump.data() + '\0', JsonWriteDump.size()), asio::use_awaitable);
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