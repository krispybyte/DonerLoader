#pragma once
#include <Shared.hpp>

namespace Network::Handle
{
	asio::awaitable<void> Idle(tcp::socket& Socket);
	asio::awaitable<void> Initialize(tcp::socket& Socket);
	asio::awaitable<void> Login(tcp::socket& Socket);
	asio::awaitable<void> Module(tcp::socket& Socket);
}

namespace ClientInformation
{
	extern int ChunkIndex;
	extern std::vector<std::uint8_t> Data;
	extern json HardwareId;
}