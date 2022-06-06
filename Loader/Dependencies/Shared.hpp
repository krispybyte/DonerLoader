/*
	APOLLO
*/

#pragma once
#pragma warning(disable : 4309)
#pragma warning(disable : 4369)
#pragma warning(disable : 4091)

#include <iostream>

// Asio
#define _WIN32_WINNT 0x0A00
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
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>

namespace Network
{
#define NETWORK_CHUNK_SIZE			200000					// 200 Kilobytes 
#define NETWORK_PORT_INT			42069
#define NETWORK_IP_AND_PORT_STR		"127.0.0.1", "42069"

	enum class SocketIds : int
	{
		Idle = 1104,
		Initialize = 2485,
		Login = 8922,
		Hwid = 9014,
		Module = 1482
	};

	typedef enum ModuleIds : int
	{
		Test8MB = 0,
		Test1KB = 1
	};
}