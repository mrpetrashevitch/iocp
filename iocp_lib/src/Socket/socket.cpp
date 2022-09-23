#include "socket.h"
#pragma warning(disable: 4996)
#pragma warning(disable: 4200)

namespace web
{
	namespace io_base
	{
		socket::socket() :_socket(), _inited(false)
		{
			ZeroMemory(&_socket_address, sizeof(_socket_address));
		}

		void socket::init(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port)
		{
			byte addres[4]{ s_b1,s_b2, s_b3, s_b4 };
			init(*((unsigned long*)&addres), port);
		}

		void socket::init(uint addres, ushort port)
		{
			sockaddr_in serv_adr;
			serv_adr.sin_family = AF_INET;
			serv_adr.sin_addr.s_addr = addres;
			serv_adr.sin_port = htons(port);
			init(serv_adr);
		}

		void socket::init(const char* addres, ushort port)
		{
			init(inet_addr(addres), port);
		}

		void socket::init(const sockaddr_in& addr)
		{
			if (_inited) return;
			_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			_socket_address = addr;
			_inited = true;
		}

		void socket::bind()
		{
			const unsigned so_reuseaddr = 1;
			//::setsockopt(_socket_accept, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&so_reuseaddr), sizeof(so_reuseaddr));
			int res = ::bind(_socket, reinterpret_cast<SOCKADDR*>(&_socket_address), sizeof(_socket_address));
			return;
		}
		void socket::bind_before_connect()
		{
			// ConnectEx requires the socket to be initially bound
			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = 0;

			int res = ::bind(_socket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr));
		}
		void socket::listen()
		{
			listen(1);
		}
		void socket::listen(int backlog)
		{
			int res = ::listen(_socket, backlog);
			return;
		}
		const SOCKET& socket::get_socket() const
		{
			return _socket;
		}
		const sockaddr_in& socket::get_socket_address() const
		{
			return _socket_address;
		}
	}
}