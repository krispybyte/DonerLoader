#include "Common.hpp"

int main()
{
	co_spawn(asio::system_executor(), Network::Connect(), asio::detached);
	return std::getchar();
}