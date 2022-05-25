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

	if (Gui::Run())
	{
		// Clean data on ImGui exit
		Module::Data.erase(Module::Data.begin(), Module::Data.end());
		Module::Data.shrink_to_fit();
		ExitProcess(EXIT_SUCCESS);
	}

	return std::getchar();
}