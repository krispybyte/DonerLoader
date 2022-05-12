#pragma once
#include <iostream>

// Asio
#define _WIN32_WINNT 0xA00
#define ASIO_STANDALONE	1
#include <asio/asio/include/asio.hpp>

// Json
#include <json/single_include/nlohmann/json.hpp>
using namespace nlohmann;

// CryptoPP
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
using namespace CryptoPP;