#pragma once
#include <Shared.hpp>

namespace Network
{
	class Socket
	{
	private:
		tcp::socket& _Socket;
	public:
		Socket(tcp::socket& Socket) : _Socket(Socket) { }
		~Socket() { _Socket.shutdown(asio::socket_base::shutdown_both); _Socket.close(); }

		tcp::socket& Get();
		asio::ip::address GetIpAddress();
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