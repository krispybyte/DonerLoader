#include "Common.hpp"
#include "Utilities/Utilities.hpp"

__forceinline void Connect()
{
	co_spawn(asio::system_executor(), Network::Connect(), asio::detached);
}

int main()
{
	SetConsoleTitleA("Client");

	HANDLE NetworkThreadHandle;
	Utilities::NtCreateThreadEx(&NetworkThreadHandle, MAXIMUM_ALLOWED, nullptr, GetCurrentProcess(), &Connect, nullptr, 0x40, 0, 0, 0, nullptr);
	Utilities::NtSuspendProcess(GetCurrentProcess());

	// GUI...

	return std::getchar();
}