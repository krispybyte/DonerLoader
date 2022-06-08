#pragma once
#include <Shared.hpp>

namespace Network::Handle
{
	extern std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
	extern asio::mutable_buffer ReadBuffer;

	extern inline asio::awaitable<void> Idle(tcp::socket& Socket);
	extern inline asio::awaitable<void> Initialize(tcp::socket& Socket);
	extern inline asio::awaitable<void> Login(tcp::socket& Socket);
	extern inline asio::awaitable<void> Module(tcp::socket& Socket);
}

namespace Client
{
	extern std::vector<std::uint8_t> ModuleData;
	extern const json HardwareId;
	extern bool HasConnected;
}