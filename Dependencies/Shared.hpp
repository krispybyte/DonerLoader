#pragma once
#include <iostream>

// Asio
#define _WIN32_WINNT 0xA00
#define ASIO_STANDALONE	1
#include <asio/asio/include/asio.hpp>
using asio::ip::tcp;

// Json
#include <json/single_include/nlohmann/json.hpp>
using namespace nlohmann;

// CryptoPP
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
using namespace CryptoPP;

namespace Network
{
#define NETWORK_CHUNK_SIZE			4096
#define NETWORK_PORT_INT			42069
#define NETWORK_IP_AND_PORT_STR		"127.0.0.1", "42069"

	typedef enum SocketIds : std::uint8_t
	{
		Module = 4892,
	};
}