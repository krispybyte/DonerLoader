#pragma once
#include <Shared.hpp>

namespace Network::Handle
{
	extern std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
	extern asio::mutable_buffer ReadBuffer;

	asio::awaitable<void> Idle(tcp::socket& Socket);
	asio::awaitable<void> Initialize(tcp::socket& Socket);
	asio::awaitable<void> Login(tcp::socket& Socket);
	asio::awaitable<void> Module(tcp::socket& Socket);
}

namespace Client
{
	extern std::vector<std::uint8_t> ModuleData;
	extern const json HardwareId;
}