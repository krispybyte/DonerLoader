#pragma once
#include <Shared.hpp>

namespace Network::Handle
{
	inline std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
	inline asio::mutable_buffer ReadBuffer(ReadBufferData.data(), ReadBufferData.size());

	asio::awaitable<void> Idle(tcp::socket& Socket);
	asio::awaitable<void> Initialization(tcp::socket& Socket, CryptoPP::RSA::PrivateKey& PrivateKey);
	asio::awaitable<void> Login(tcp::socket& Socket);
	asio::awaitable<void> Module(tcp::socket& Socket);
}

namespace Module
{
	inline int ChunkIndex = 0;
	inline std::vector<std::uint8_t> Data;
}