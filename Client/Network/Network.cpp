#include "Network.hpp"

asio::awaitable<void> Network::SocketHandler(tcp::socket Socket)
{
	std::cout << "[+] Connected." << std::endl;

	Crypto::Rsa Rsa;
	Crypto::Aes256Gcm Aes;

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
				case ClientStates::InitializeState:
				{
					JsonWrite =
					{
						{ "Id", SocketIds::Initialize },
						{ "Data", Crypto::Base64::Encode(Crypto::PEM::ExportKey(Rsa.GetPublicKey())) }
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
				case SocketIds::Initialize:
				{
					auto ServerPublicKey = Crypto::PEM::ImportKey(Crypto::Base64::Decode(JsonRead["Data"]));

					std::cout << "[+] Got the server's public key!" << std::endl;

					Rsa.SetPublicKey(ServerPublicKey);
					ClientState = ClientStates::LoginState;
					break;
				}
				case SocketIds::Login:
				{
					std::cout << "[+] Server logged me in." << std::endl;
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