#include "Common.hpp"

int main()
{
	SetConsoleTitleA("Server");
	Network::Create(NETWORK_PORT_INT);
	return std::getchar();
}