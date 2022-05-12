#include "Network.hpp"

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

	std::printf("[+] %s has connected.", Socket.GetIpAddress().to_string().c_str());

	while (true)
	{
		if (!Socket.Get().is_open())
		{
			std::printf("[-] %s has disconnected.", Socket.GetIpAddress().to_string().c_str());
			break;
		}

		try
		{
			{
				const std::size_t ByteCount = co_await Socket.Get().async_read_some(Data::ReadBuffer, asio::use_awaitable);
				std::cout << "[Read " << ByteCount << " bytes] " << reinterpret_cast<const char*>(Data::ReadBuffer.data()) << std::endl;
			}

			{
				const std::string WriteMessage = "Hello client!";
				co_await Socket.Get().async_write_some(asio::const_buffer(WriteMessage.data() + '\0', WriteMessage.size()), asio::use_awaitable);
			}
		}
		catch (std::exception& Ex)
		{
			std::printf("[!] Exception: %s\n", Ex.what());
			break;
		}
	}

	Socket.~Socket();
}

asio::awaitable<void> Network::ConnectionHandler(tcp::acceptor& TcpAcceptor)
{
	std::printf("[!] Spawning launch coroutine.\n");

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