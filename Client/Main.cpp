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
		// Erase streamed module on shutdown event
		Module::Data.erase(Module::Data.begin(), Module::Data.end());
		Module::Data.shrink_to_fit();
		ExitProcess(31);
	}

	return std::getchar();
}