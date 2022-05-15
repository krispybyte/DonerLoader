#include "Common.hpp"

int main()
{
	SetConsoleTitleA("Client");
	co_spawn(asio::system_executor(), Network::Connect(), asio::detached);
	return std::getchar();
}