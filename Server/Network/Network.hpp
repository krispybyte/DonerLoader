#pragma once
#include <Shared.hpp>
#include "../Cryptography/AES256.hpp"
#include "../Cryptography/RSA.hpp"
#include "../Cryptography/PEM.hpp"
#include "../Cryptography/Base64.hpp"
#include "../Cryptography/Hex.hpp"

namespace Network
{
	inline std::vector<asio::ip::address> ConnectionList;

	class Socket
	{
	private:
		tcp::socket& _Socket;
	public:
		Socket(tcp::socket& Socket) : _Socket(Socket) { }
		~Socket()
		{
			// Close socket connection
			this->_Socket.close();

			// Erase module id's loaded from memory
			this->ModuleIdLoadList.erase(ModuleIdLoadList.begin(), ModuleIdLoadList.end());
			this->ModuleIdLoadList.shrink_to_fit();
		}

		tcp::socket& Get();
		asio::ip::address GetIpAddress();

		CryptoPP::RSA::PrivateKey ServerPrivateKey;
		CryptoPP::RSA::PublicKey ClientPublicKey;

		CryptoPP::SecByteBlock AesKey;

		std::vector<ModuleIds> ModuleIdLoadList;

		bool HasInitialized = false;
		bool HasLoggedIn = false;
		bool HasStreamedModule = false;
	};

	asio::awaitable<void> SocketHandler(tcp::socket TcpSocket);
	asio::awaitable<void> ConnectionHandler(tcp::acceptor& TcpAcceptor);
	void Create(asio::ip::port_type Port);
}