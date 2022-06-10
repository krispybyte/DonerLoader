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
	
	std::thread([&]
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));
	
		// Failed connecting within 3 seconds
		if (!Client::HasConnected)
		{
			MessageBoxA(nullptr, "Failed connecting to the server.", "Error (14)", MB_ICONERROR | MB_OK);
			ExitProcess(14);
		}
	}).detach();
	
	Gui::Run();
	
	// Clean module data
	Client::ModuleData.erase(Client::ModuleData.begin(), Client::ModuleData.end());
	Client::ModuleData.shrink_to_fit();
	ExitProcess(EXIT_SUCCESS);
}