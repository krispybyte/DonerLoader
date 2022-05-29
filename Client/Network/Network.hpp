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

	inline Network::ClientStates ClientState = Network::ClientStates::InitializeState;

	inline CryptoPP::RSA::PublicKey ServerPublicKey;
	inline CryptoPP::SecByteBlock AesKey;

	inline bool SuccessfulLogin = false;
	inline std::uint8_t LoginAttempts = 0;

	asio::awaitable<void> SocketHandler(tcp::socket Socket);
	asio::awaitable<void> Connect();
}