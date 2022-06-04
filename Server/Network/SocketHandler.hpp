#pragma once
#include <Shared.hpp>
#include "Network.hpp"

namespace Network::Handle
{
	asio::awaitable<void> Idle(Network::Socket& Socket);
	asio::awaitable<void> Initialize(Network::Socket& Socket, const json& ReadJson);
	asio::awaitable<void> Login(Network::Socket& Socket, const json& ReadJson);
	asio::awaitable<void> Module(Network::Socket& Socket, const json& ReadJson);
}