#include "Network.hpp"

asio::awaitable<void> Network::SocketHandler(tcp::socket Socket)
{
	std::printf("[+] Connected.");

	while (true)
	{
		if (!Socket.is_open())
		{
			std::printf("[-] Disconnected.");
			break;
		}

		try
		{
			{
				const std::string WriteMessage = "Hello server!";
				co_await Socket.async_write_some(asio::const_buffer(WriteMessage.data() + '\0', WriteMessage.size()), asio::use_awaitable);
			}

			{
				const std::size_t ByteCount = co_await Socket.async_read_some(Data::ReadBuffer, asio::use_awaitable);
				std::cout << "[Read " << ByteCount << " bytes] " << reinterpret_cast<const char*>(Data::ReadBuffer.data()) << std::endl;
			}
		}
		catch (std::exception& Ex)
		{
			std::printf("[!] Exception: %s\n", Ex.what());
			break;
		}
	}

	Socket.shutdown(asio::socket_base::shutdown_both);
	Socket.close();
}

asio::awaitable<void> Network::Connect()
{
	std::printf("[!] Spawning launch coroutine\n");

	const auto Executor = asio::get_associated_executor(asio::use_awaitable);
	auto Socket = tcp::socket(Executor);
	auto Resolver = tcp::resolver(Executor);
	const auto Query = tcp::resolver::query(NETWORK_IP_AND_PORT_STR);

	const auto Endpoint = tcp::resolver::iterator{ co_await Resolver.async_resolve(Query, asio::use_awaitable) };

	co_await Socket.async_connect(*Endpoint, asio::use_awaitable);
	co_await co_spawn(Executor, Network::SocketHandler(std::move(Socket)), asio::use_awaitable);
}