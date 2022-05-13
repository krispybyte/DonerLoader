#include "Network.hpp"
#include "../Cryptography/AES256.hpp"
#include "../Cryptography/RSA.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"

asio::awaitable<void> Network::SocketHandler(tcp::socket Socket)
{
	std::cout << "[+] Connected." << std::endl;

	Crypto::Rsa Rsa(4096);
	Crypto::Aes256Gcm Aes;

	while (true)
	{
		if (!Socket.is_open())
		{
			std::cout << "[-] Disconnected." << std::endl;
			break;
		}

		Aes.Generate();

		try
		{
			{
				json Json;

				{
					Json["Id"] = Network::SocketIds::KeyExchange;
					Json["Data"] = "Hello";
				}

				const std::string RawJson = Json.dump();

				co_await Socket.async_write_some(asio::const_buffer(RawJson.data() + '\0', RawJson.size()), asio::use_awaitable);
			}

			{
				const std::size_t ByteCount = co_await Socket.async_read_some(Data::ReadBuffer, asio::use_awaitable);
				std::cout << "[Read " << ByteCount << " bytes] " << reinterpret_cast<const char*>(Data::ReadBuffer.data()) << std::endl;
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

	Socket.shutdown(asio::socket_base::shutdown_both);
	Socket.close();
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