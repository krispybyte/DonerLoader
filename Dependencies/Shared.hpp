#pragma once
#pragma warning(disable : 4309)
#pragma warning(disable : 4369)
#pragma warning(disable : 4091)

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
#include <cryptopp/gcm.h>
#include <cryptopp/rsa.h>
#include <cryptopp/pem.h>
#include <cryptopp/base64.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
using namespace CryptoPP;

namespace Network
{
#define NETWORK_CHUNK_SIZE			4096
#define NETWORK_PORT_INT			42069
#define NETWORK_IP_AND_PORT_STR		"127.0.0.1", "42069"

	typedef enum SocketIds : int
	{
		Idle = 0,
		KeyExchange = 9021,
		Module = 4892,
	};

	typedef enum ClientStates : int
	{
		None = 0,
		ExchangingKeys = 1,
	};
}