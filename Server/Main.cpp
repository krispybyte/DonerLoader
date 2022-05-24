#include "Common.hpp"
#include "Utilities/Utilities.hpp"

int main()
{
	SetConsoleTitleA("Server");

	// Load all modules
	if (!Modules::Load(Network::ModuleIds::Test8MB, "Modules/Test8MB.dll") ||
		!Modules::Load(Network::ModuleIds::Test1KB, "Modules/Test1KB.dll"))
	{
		std::cout << "[!] Failed loading a module." << '\n';
		return std::getchar();
	}

	// Create server
	Network::Create(NETWORK_PORT_INT);

	return std::getchar();
}