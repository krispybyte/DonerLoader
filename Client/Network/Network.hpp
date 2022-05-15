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
		IdleState = 0,
		InitializeState,
		LoginState,
		HwidState,
		ModuleState,
	};

	inline Network::ClientStates ClientState = Network::ClientStates::InitializeState;

	asio::awaitable<void> SocketHandler(tcp::socket Socket);
	asio::awaitable<void> Connect();
}