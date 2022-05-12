#pragma once
#include <Shared.hpp>

namespace Network
{
	namespace Data
	{
		inline std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
		inline asio::mutable_buffer ReadBuffer(ReadBufferData.data(), ReadBufferData.size());
	}

	asio::awaitable<void> SocketHandler(tcp::socket Socket);
	asio::awaitable<void> Connect();
}