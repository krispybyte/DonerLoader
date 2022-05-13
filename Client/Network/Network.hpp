#pragma once
#include <Shared.hpp>
#include "../Cryptography/AES256.hpp"
#include "../Cryptography/RSA.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"

namespace Network
{
	inline Network::ClientStates CurrentState = ClientStates::ExchangingKeys;

	namespace Data
	{
		inline std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
		inline asio::mutable_buffer ReadBuffer(ReadBufferData.data(), ReadBufferData.size());
	}

	asio::awaitable<void> SocketHandler(tcp::socket Socket);
	asio::awaitable<void> Connect();
}