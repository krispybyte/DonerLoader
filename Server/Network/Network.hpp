#pragma once
#include <Shared.hpp>
#include "../Cryptography/AES256.hpp"
#include "../Cryptography/RSA.hpp"

namespace Network
{
	inline std::vector<asio::ip::address> ConnectionList;

	class Socket
	{
	private:
		tcp::socket& _Socket;
	public:
		Socket(tcp::socket& Socket) : _Socket(Socket) { }
		~Socket() { this->_Socket.shutdown(asio::socket_base::shutdown_both); }

		tcp::socket& Get();
		asio::ip::address GetIpAddress();

		Crypto::Aes256Gcm Aes;
		Crypto::Rsa Rsa;

		RSA::PublicKey ClientPublicKey;
	};

	namespace Data
	{
		inline std::array<char, NETWORK_CHUNK_SIZE> ReadBufferData;
		inline asio::mutable_buffer ReadBuffer(ReadBufferData.data(), ReadBufferData.size());
	}

	asio::awaitable<void> SocketHandler(tcp::socket TcpSocket);
	asio::awaitable<void> ConnectionHandler(tcp::acceptor& TcpAcceptor);
	void Create(asio::ip::port_type Port);
}