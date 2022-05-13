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
		Socket.~Socket();
		co_return;
	}

	ConnectionList.push_back(IpAddress);

	std::cout << "[+] " << IpAddress.to_string().c_str() << " has connected." << std::endl;

	Socket.Rsa.Generate();

	while (true)
	{
		if (!Socket.Get().is_open())
		{
			std::cout << "[-] " << IpAddress.to_string().c_str() << " has disconnected." << std::endl;
			break;
		}

		Socket.Aes.Generate();

		try
		{
			{
				const auto ByteCount = co_await Socket.Get().async_read_some(Data::ReadBuffer, asio::use_awaitable);
				std::cout << "[Read " << ByteCount << " bytes] " << reinterpret_cast<const char*>(Data::ReadBuffer.data()) << std::endl;
			}

			{
				json Json;

				{
					Json["Stub"] = 1337;
				}

				const std::string RawJson = Json.dump();

				co_await Socket.Get().async_write_some(asio::const_buffer(RawJson.data() + '\0', RawJson.size()), asio::use_awaitable);
			}
		}
		catch (std::exception& Ex)
		{
			std::cerr << "[" << IpAddress.to_string().c_str() << "] Exception: " << Ex.what() << std::endl;
			break;
		}
	}

	ConnectionList.erase(std::remove(ConnectionList.begin(), ConnectionList.end(), Socket.GetIpAddress()), ConnectionList.end());
	Socket.~Socket();
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