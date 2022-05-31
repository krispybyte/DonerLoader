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

	// Clean data on finish
	ClientInformation::Data.erase(ClientInformation::Data.begin(), ClientInformation::Data.end());
	ClientInformation::Data.shrink_to_fit();
	ExitProcess(EXIT_SUCCESS);

	return std::getchar();
}