#pragma once
#include <Shared.hpp>
#include "../Cryptography/AES256.hpp"
#include "../Cryptography/RSA.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"
#include "../Cryptography/Hex.hpp"

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

	extern Network::ClientStates ClientState;

	extern CryptoPP::RSA::PublicKey ServerPublicKey;
	extern CryptoPP::SecByteBlock AesKey;

	extern Network::LoginStatusIds LoginStatus;
	extern std::uint8_t LoginAttempts;

	asio::awaitable<void> SocketHandler(tcp::socket Socket);
	asio::awaitable<void> Connect();
}