#include "Network.hpp"
#include "SocketHandler.hpp"

asio::awaitable<void> Network::SocketHandler(tcp::socket Socket)
{
	std::cout << "[+] Connected." << std::endl;

	// Cryptography
	RSA::PrivateKey ClientPrivateKey = Crypto::Rsa::GeneratePrivate();

	while (true)
	{
		if (!Socket.is_open())
		{
			std::cout << "[-] Disconnected." << std::endl;
			break;
		}

		try
		{
			switch (ClientState)
			{
				case ClientStates::IdleState:
				{
					co_await Handle::Idle(Socket);
					break;
				}
				case ClientStates::InitializeState:
				{
					co_await Handle::Initialization(Socket, ClientPrivateKey);
					break;
				}
				default:
				{
					std::cout << "[!] Invalid client state.";
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

	const asio::system_executor Executor = asio::get_associated_executor(asio::use_awaitable);
	tcp::socket Socket = tcp::socket(Executor);
	tcp::resolver Resolver = tcp::resolver(Executor);
	const tcp::resolver::query Query = tcp::resolver::query(NETWORK_IP_AND_PORT_STR);

	const tcp::resolver::iterator Endpoint = tcp::resolver::iterator{ co_await Resolver.async_resolve(Query, asio::use_awaitable) };

	co_await Socket.async_connect(*Endpoint, asio::use_awaitable);
	co_await co_spawn(Executor, Network::SocketHandler(std::move(Socket)), asio::use_awaitable);
}