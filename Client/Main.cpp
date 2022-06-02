#include "Common.hpp"
#include "Network/SocketHandler.hpp"
#include "Gui/Gui.hpp"

void main()
{
	SetConsoleTitleA("Client");

	std::thread([&]
	{
		co_spawn(asio::system_executor(), Network::Connect(), asio::detached);
	}).detach();

	Gui::Run();

	// Clean module data
	ClientInformation::Data.erase(ClientInformation::Data.begin(), ClientInformation::Data.end());
	ClientInformation::Data.shrink_to_fit();
	ExitProcess(EXIT_SUCCESS);
}