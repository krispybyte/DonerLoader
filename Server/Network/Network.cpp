#include "Network.hpp"
#include "SocketHandler.hpp"

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

	const asio::ip::address IpAddress = Socket.GetIpAddress();

	if (std::find(ConnectionList.begin(), ConnectionList.end(), IpAddress) != ConnectionList.end())
	{
		std::cout << "[-] " << IpAddress.to_string().c_str() << " is already connected!" << std::endl;
		co_return;
	}

	ConnectionList.push_back(IpAddress);

	std::cout << "[+] " << IpAddress.to_string().c_str() << " has connected." << std::endl;

	std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
	asio::mutable_buffer ReadBuffer(ReadBufferData.data(), ReadBufferData.size());

	// Cryptography
	Socket.ServerPrivateKey = Crypto::Rsa::GeneratePrivate();
	Socket.AesKey = Crypto::Aes256::GenerateKey();

	while (true)
	{
		if (!Socket.Get().is_open())
		{
			std::cout << "[-] " << IpAddress.to_string().c_str() << " has disconnected." << std::endl;
			break;
		}

		try
		{
			co_await Socket.Get().async_read_some(ReadBuffer, asio::use_awaitable);

			json ReadJson = json::parse(reinterpret_cast<const char*>(ReadBuffer.data()));

			const SocketIds SocketId = static_cast<SocketIds>(ReadJson["Id"]);
			switch (SocketId)
			{
				case SocketIds::Idle:
				{
					co_await Handle::Idle(Socket);
					break;
				}
				case SocketIds::Initialize:
				{
					co_await Handle::Initialize(Socket, ReadJson);
					break;
				}
				case SocketIds::Login:
				case SocketIds::Hwid:
				case SocketIds::Module:
				{
					std::cout << "[!] Unimplemented." << std::endl;
					break;
				}
				default:
				{
					std::cout << "[!] Invalid socket id." << std::endl;
					goto Disconnect;
				}
			}
		}
		catch (std::exception& Ex)
		{
			std::cerr << "[" << IpAddress.to_string().c_str() << "] Exception: " << Ex.what() << std::endl;
			break;
		}
	}

Disconnect:
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
		tcp::socket Socket = co_await TcpAcceptor.async_accept(asio::use_awaitable);
		co_spawn(Socket.get_executor(), Network::SocketHandler(std::move(Socket)), asio::detached);
	}
}

void Network::Create(asio::ip::port_type Port)
{
	// Create asio io context
	asio::io_context IoCtx(1);

	const asio::io_context::executor_type Executor = IoCtx.get_executor();
	tcp::acceptor Acceptor(Executor, { tcp::v4(), Port });

	// Spawn launch coroutine
	co_spawn(IoCtx, Network::ConnectionHandler(Acceptor), asio::detached);

	// Run event processing loop
	IoCtx.run();
}