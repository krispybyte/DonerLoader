#include "Network.hpp"
#include "SocketHandler.hpp"

asio::awaitable<void> Network::SocketHandler(tcp::socket Socket)
{
	std::cout << "[+] Connected." << '\n';

	// Cryptography
	RSA::PrivateKey ClientPrivateKey = Crypto::Rsa::GeneratePrivate();

	while (true)
	{
		if (!Socket.is_open())
		{
			std::cout << "[-] Disconnected." << '\n';
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
				case ClientStates::LoginState:
				{
					co_await Handle::Login(Socket);
					break;
				}
				case ClientStates::ModuleState:
				{
					co_await Handle::Module(Socket);
					break;
				}
				default:
				{
					std::cout << "[!] Invalid client state." << '\n';
					break;
				}
			}
		}
		catch (std::exception& Ex)
		{
			std::cout << "[!] Exception: " << Ex.what() << '\n';
			MessageBoxA(nullptr, "The server closed the connection.", "Error (94)", MB_ICONERROR | MB_OK);
			ExitProcess(94);
			break;
		}
	}
}

asio::awaitable<void> Network::Connect()
{
	std::cout << "[!] Spawning launch coroutine." << '\n';

	const asio::system_executor Executor = asio::get_associated_executor(asio::use_awaitable);
	tcp::socket Socket = tcp::socket(Executor);
	tcp::resolver Resolver = tcp::resolver(Executor);
	const tcp::resolver::query Query = tcp::resolver::query(NETWORK_IP_AND_PORT_STR);

	const tcp::resolver::iterator Endpoint = tcp::resolver::iterator{ co_await Resolver.async_resolve(Query, asio::use_awaitable) };

	co_await Socket.async_connect(*Endpoint, asio::use_awaitable);
	co_await co_spawn(Executor, Network::SocketHandler(std::move(Socket)), asio::use_awaitable);
}