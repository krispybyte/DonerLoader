#pragma once
#include <Shared.hpp>
#include "../Cryptography/AES256.hpp"
#include "../Cryptography/RSA.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"

namespace Network
{
	enum class ClientStates : int
	{
		InitializeState = 0,
		LoginState = 1,
		HwidState = 2,
		ModuleState = 3,
	};

	inline Network::ClientStates ClientState = Network::ClientStates::InitializeState;

	asio::awaitable<void> SocketHandler(tcp::socket Socket);
	asio::awaitable<void> Connect();
}