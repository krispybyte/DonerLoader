#include "Common.hpp"
#include "Network/SocketHandler.hpp"
#include "Gui/Gui.hpp"

int main()
{
	SetConsoleTitleA("Client");

	std::thread([&]
	{
		co_spawn(asio::system_executor(), Network::Connect(), asio::detached);
	}).detach();

	Gui::Run();

	// Clean module data
	Client::ModuleData.erase(Client::ModuleData.begin(), Client::ModuleData.end());
	Client::ModuleData.shrink_to_fit();
	ExitProcess(EXIT_SUCCESS);
}